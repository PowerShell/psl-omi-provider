#include <MI.h>
#include "Shell.h"
#include "ShellAPI.h"
#include "BufferManipulation.h"
#include <pal/strings.h>
#include <pal/format.h>
#include <pal/lock.h>
#include <base/batch.h>
#include <base/instance.h>

/* TODO:
    * No provider (de-)initialization through WSManPluginStartup and WSManPluginShutdown
    * No re-connect
*/


/* Number of characters reserved for command ID and shell ID -- max number of digits for hex 64-bit number with null terminator */
#define ID_LENGTH 17

#define GOTO_ERROR(result) { miResult = result; goto error; }
#define GOTO_ERROR_EX(result, label) { miResult = result; goto label; }

/* Indexes into the WSMAN_PLUGIN_REQUEST arrays in CommandData and ShellData */

/* Commands and shells can have multiple streams associated with them. */
typedef struct _StreamData
{
	MI_Char *streamName;
	MI_Boolean done;
} StreamData;
typedef struct _StreamDataSet
{
    MI_Uint32 streamDataCount;
    StreamData *streamData;
} StreamDataSet;

typedef struct _StreamSet
{
    MI_Uint32 streamNamesCount;
    MI_Char **streamNames;
} StreamSet;
/* Index for CommonData arrays as well as the type of the CommonData */
typedef enum
{
    WSMAN_PLUGIN_REQUEST_SHELL = 0,
    WSMAN_PLUGIN_REQUEST_COMMAND = 1,
    WSMAN_PLUGIN_REQUEST_SEND = 2,
    WSMAN_PLUGIN_REQUEST_RECEIVE = 3,
    WSMAN_PLUGIN_REQUEST_SIGNAL = 4
} CommonData_Type;

typedef struct _CommonData CommonData;
typedef struct _ShellData ShellData;
typedef struct _CommandData CommandData;
typedef struct _SendData SendData;
typedef struct _ReceiveData ReceiveData;
typedef struct _SignalData SignalData;


struct _CommonData
{
    /* MUST BE FIRST ITEM IN STRUCTURE  as pluginRequest gets cast to CommonData*/
    WSMAN_PLUGIN_REQUEST pluginRequest;

    /* Pointer to the owning operation data, either commandData, shellData or NULL if this is the shell */
    CommonData *parentData;

    /* Pointer to sibling operation data, beit other shells, direct children list for shell or direct children list for command */
    CommonData *siblingData;

    /* Allows us to identify if we are a request for a Shell, Command, Send, Receive or Signal request */
    CommonData_Type requestType;

    /* Associated MI_Context for the WSMAN plugin request */
    MI_Context *miRequestContext;

    /* MI_Instance that was passed in for creating instance, or the parameter object passed in to the operation method */
    MI_Instance *miOperationInstance;

    /* Batch allocator for this request data object and all memory we allocate inside this object */
    Batch *batch;
} ;

struct _ShellData
{
    CommonData common;

    /* This shells ID */
    MI_Char *shellId;

    /* pointer to list of all active child requests, including command, send, receive and signals. We only support 1 active command */
    CommonData *childNext;

    StreamSet inputStreams;
    StreamSet outputStreams;

    WSMAN_SHELL_STARTUP_INFO wsmanStartupInfo;

    /* Is the inbound/outbound streams compressed? */
    MI_Boolean isCompressed;

    /* MI provider self pointer that is returned from MI provider load which holds all shell state. When our shell
     * goes away we will need to remove outself from the list 
    */
    Shell_Self *shell;

    void * pluginShellContext;
};

struct _CommandData
{
    /* MUST BE FIRST ITEM IN STRUCTURE as pointer to CommonData gets cast to CommandData */
    CommonData common;

    /* This commands ID. There is only one command per shell,
    * but there may be more than one shell in a process.
    */
    MI_Char *commandId;

    /* pointer to list of all active child requests, including send, receive and signals. We only support 1 active command */
    CommonData *childNext;

    WSMAN_COMMAND_ARG_SET wsmanArgSet;

    /* WSMAN shell Plug-in context is the context reported from either the shell or command depending on which type it is */
    void * pluginCommandContext;
};

struct _SendData
{
    /* MUST BE FIRST ITEM IN STRUCTURE as pointer to CommonData gets cast to SendData */
    CommonData common;

};

struct _ReceiveData
{
    /* MUST BE FIRST ITEM IN STRUCTURE as pointer to CommonData gets cast to ReceiveData */
    CommonData common;

};

struct _SignalData
{
    /* MUST BE FIRST ITEM IN STRUCTURE as pointer to CommonData gets cast to SignalData */
    CommonData common;

};

/* The master shell object that the provider passes back as context for all provider
 * operations. Currently it only needs to point to the list of shells.
 */
struct _Shell_Self
{
    ShellData *shellList;

    void *pluginContext;
} ;

/* Based on the shell ID, find the existing ShellData object */
ShellData * FindShell(struct _Shell_Self *shell, const MI_Char *shellId)
{
    ShellData *shellData = shell->shellList;

    if (shellId == NULL)
        return NULL;

    while (shellData && (Tcscmp(shellId, shellData->shellId) != 0))
    {
        shellData = (ShellData*)shellData->common.siblingData;
    }

    return shellData;
}

/* Shell_Load is called after the provider has been loaded to return
 * the provider schema to the engine. It also allocates and returns our own
 * context object that is passed to all operations that holds the current
 * state of all our shells.
 */
void MI_CALL Shell_Load(Shell_Self** self, MI_Module_Self* selfModule,
        MI_Context* context)
{
    *self = calloc(1, sizeof(Shell_Self));
    if (*self == NULL)
    {
    	MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    else
    {
        MI_Uint32 result = WSManPluginStartup(0, NULL, NULL, &(*self)->pluginContext);

        MI_Context_PostResult(context, result);
    }
}

/* Shell_Unload is called after all operations have completed, or they should
 * be completed. This function is called right before the provider is unloaded.
 * We clean up our shell context object at this point as no more operations will
 * be using it.
 */
void MI_CALL Shell_Unload(Shell_Self* self, MI_Context* context)
{
    /* TODO: Do we have any shells still active? */
    
    WSManPluginShutdown(self->pluginContext, 0, 0);
    /* NOTE: Expectation is that WSManPluginReportCompletion should be called, but it is not looking like that is always happening */

    free(self);
    MI_Context_PostResult(context, MI_RESULT_OK);
}

/* Shell_EnumerateInstances should return all active shell instances to the client.
 * It is not currently supported.
 */
void MI_CALL Shell_EnumerateInstances(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_PropertySet* propertySet, MI_Boolean keysOnly,
        const MI_Filter* filter)
{
	/* Enumerate through the list of shells and post the results back */
	ShellData *shellData = self->shellList;
	MI_Result miResult = MI_RESULT_OK;

	while (shellData)
	{
		miResult = MI_Context_PostInstance(context, shellData->common.miOperationInstance);
		if (miResult != MI_RESULT_OK)
		{
			break;
		}

        shellData = (ShellData*) shellData->common.siblingData;
	}
    MI_Context_PostResult(context, miResult);
}

/* Shell_GetInstance should return information about a shell.
 * It is not currently supported.
 */
void MI_CALL Shell_GetInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* instanceName, const MI_PropertySet* propertySet)
{
    MI_Result miResult = MI_RESULT_NOT_FOUND;
    ShellData *shellData = FindShell(self, instanceName->Name.value);

    if (shellData)
    {
        miResult = MI_Context_PostInstance(context, shellData->common.miOperationInstance);
    }
    MI_Context_PostResult(context, miResult);
}

/* ExtractStreamSet takes a list of streams that are space delimited and
 * copies the data into the ShellData object for the stream. These stream
 * names are allocated and so will need to be deleted when the shell is deleted.
 * We allocate a single buffer for the array and the string, then insert null terminators
 * into the string where spaces were and fix up the array pointers to these strings.
 */
static MI_Boolean ExtractStreamSet(const MI_Char *streams, StreamSet *streamSet, Batch *batch)
{
	MI_Char *cursor = (MI_Char*) streams;
	MI_Uint32 i;
	int stringLength = Tcslen(streams)+1; /* string length including null terminator */

	/* Count how many stream names we have so we can allocate the array */
	while (cursor && *cursor)
	{
        streamSet->streamNamesCount++;
		cursor = Tcschr(cursor, MI_T(' '));
		if (cursor && *cursor)
		{
			cursor++;
		}
	}

	/* Allocate the buffer for the array and all strings inside the array */
    streamSet->streamNames = Batch_Get(batch,
			  (sizeof(MI_Char*) * streamSet->streamNamesCount) /* array length */
			+ (sizeof(MI_Char) * stringLength)); /* Length for the copied stream names */
	if (streamSet->streamNames == NULL)
		return MI_FALSE;

	/* Set the cursor to the start of where the strings are placed in the buffer */
	cursor = (MI_Char*)streamSet->streamNames + (sizeof(MI_Char*) * streamSet->streamNamesCount);

	/* Copy the stream names across to the correct place */
	Tcslcpy(cursor, streams, stringLength);

	/* Loop through the streams fixing up the array pointers and replace spaces with null terminator */
	for (i = 0; i != streamSet->streamNamesCount; i++)
	{
		/* Fix up the pointer to the stream name */
        streamSet->streamNames[i] = cursor;

		/* Find the next space or end of string */
		cursor = Tcschr(cursor, MI_T(' '));
		if (cursor)
		{
			/* We found a space to replace with null terminator */
			*cursor = MI_T('\0');

			/* Skip to start of next stream name */
			cursor++;
		}
	}
	return MI_TRUE;
}

MI_Boolean ExtractStartupInfo(ShellData *shellData, const Shell *shellInstance, Batch *batch)
{
    memset(&shellData->wsmanStartupInfo, 0, sizeof(shellData->wsmanStartupInfo));
    
    shellData->wsmanStartupInfo.name = shellInstance->Name.value;

    if (shellInstance->WorkingDirectory.exists)
        shellData->wsmanStartupInfo.workingDirectory = shellInstance->WorkingDirectory.value;

    if (shellInstance->InputStreams.exists)
    {
        shellData->wsmanStartupInfo.inputStreamSet = Batch_Get(batch, sizeof(*shellData->wsmanStartupInfo.inputStreamSet));
        if (shellData->wsmanStartupInfo.inputStreamSet == NULL)
            return MI_FALSE;

        shellData->wsmanStartupInfo.inputStreamSet->streamIDs = (const MI_Char**) shellData->inputStreams.streamNames;
        shellData->wsmanStartupInfo.inputStreamSet->streamIDsCount = shellData->inputStreams.streamNamesCount;
    }

    if (shellInstance->OutputStreams.exists)
    {
        shellData->wsmanStartupInfo.outputStreamSet = Batch_Get(batch, sizeof(*shellData->wsmanStartupInfo.outputStreamSet));
        if (shellData->wsmanStartupInfo.outputStreamSet == NULL)
            return MI_FALSE;

        shellData->wsmanStartupInfo.outputStreamSet->streamIDs = (const MI_Char**) shellData->outputStreams.streamNames;
        shellData->wsmanStartupInfo.outputStreamSet->streamIDsCount = shellData->outputStreams.streamNamesCount;
    }

    if (shellInstance->Environment.exists)
    {
        MI_Uint32 i;
        shellData->wsmanStartupInfo.variableSet = Batch_Get(batch, 
            sizeof(WSMAN_ENVIRONMENT_VARIABLE_SET) +
            (sizeof(WSMAN_ENVIRONMENT_VARIABLE) * shellInstance->Environment.value.size));
        if (shellData->wsmanStartupInfo.variableSet == NULL)
            return MI_FALSE;
        shellData->wsmanStartupInfo.variableSet->vars = (WSMAN_ENVIRONMENT_VARIABLE*) (shellData->wsmanStartupInfo.variableSet + 1);
        shellData->wsmanStartupInfo.variableSet->varsCount = shellInstance->Environment.value.size;
        for (i = 0; i != shellInstance->Environment.value.size; i++)
        {
            shellData->wsmanStartupInfo.variableSet->vars[i].name = shellInstance->Environment.value.data[i]->Name.value;
            shellData->wsmanStartupInfo.variableSet->vars[i].value = shellInstance->Environment.value.data[i]->Value.value;
        }
    }

    return MI_TRUE;
}

/* Shell_CreateInstance
 * Called by the client to create a shell. The shell is given an ID by us and sent back.
 * The list of streams that a command could have is listed out in the shell instance passed
 * to us. Other stuff can be included but we are ignoring that for the moment.
 */
void MI_CALL Shell_CreateInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* newInstance)
{
    ShellData *shellData = NULL;
    MI_Result miResult;
    MI_Instance *miOperationInstance = NULL;
    Batch *batch;

    /* Allocate our shell data out of a batch so we can allocate most of it from a single page and free it easily */
    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Create an internal representation of the shell object that can can use to
     * hold state. It is based on this recored data that a client should be able
     * to do an EnumerateInstance call to the provider.
     * We also allocate enough space for the shell ID string.
     */
    shellData = Batch_GetClear(batch, sizeof(*shellData));
    if (shellData == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Set the shell ID to be the pointer within the bigger buffer */
    shellData->shellId = Batch_Get(batch, sizeof(MI_Char)*ID_LENGTH);
    if (shellData->shellId == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (Stprintf(shellData->shellId, ID_LENGTH, MI_T("%llx"), (MI_Uint64) shellData->shellId) < 0)
    {
        GOTO_ERROR(MI_RESULT_FAILED);
    }

    /* Create an instance of the shell that we can send for the result of this Create as well as a get/enum operation*/
    /* Note: Instance is allocated from the batch so will be deleted when the shell batch is destroyed */
    if (Instance_Clone(&newInstance->__instance, &miOperationInstance, batch) != MI_RESULT_OK)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if ((miResult = Shell_SetPtr_Name((Shell*)miOperationInstance, shellData->shellId)) != MI_RESULT_OK)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Extract the outbound stream names that are space delimited into an
     * actual array of strings
     */
    if (!ExtractStreamSet(newInstance->InputStreams.value, &shellData->inputStreams, batch))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    if (!ExtractStreamSet(newInstance->OutputStreams.value, &shellData->outputStreams, batch))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (!ExtractStartupInfo(shellData, newInstance, batch))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    {
    	MI_Value value;
    	MI_Type type;
    	MI_Uint32 flags;
    	MI_Uint32 index;

		if ((MI_Instance_GetElement(&newInstance->__instance, MI_T("IsCompressed"), &value, &type, &flags, &index) == 0) &&
				(type == MI_BOOLEAN) &&
				value.boolean)
		{
			shellData->isCompressed = MI_TRUE;
		}
    }

    /* TODO: Fill in shellData->common.pluginRequest */
    shellData->common.parentData = NULL;    /* We are the top-level shell object */
    shellData->common.requestType = WSMAN_PLUGIN_REQUEST_SHELL;
    shellData->common.miRequestContext = context;
    shellData->common.miOperationInstance = miOperationInstance;
    shellData->common.batch = batch;

    /* Plumb this shell into our list. Failure paths after this need to unplumb it!
    */
    shellData->common.siblingData = (CommonData *)self->shellList;
    self->shellList = shellData;
    shellData->shell = self;

    /* Call out to external plug-in API to continue shell creation.
     * Acceptance of shell is reported through WSManPluginReportContext.
     * If the shell succeeds or fails we will get a call through WSManPluginOperationComplete
     */
    WSManPluginShell(self->pluginContext, &shellData->common.pluginRequest, 0, &shellData->wsmanStartupInfo, NULL);
    return;

error:
    if (batch)
        Batch_Delete(batch);

    MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
}


/* Don't support modifying a shell instance */
void MI_CALL Shell_ModifyInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* modifiedInstance, const MI_PropertySet* propertySet)
{
    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

/* Delete a shell instance. This should not be done by the client until
 * the command is finished and shut down.
 */
void MI_CALL Shell_DeleteInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* instanceName)
{
    ShellData *shellData;
    MI_Result miResult = MI_RESULT_NOT_FOUND;

    shellData = FindShell(self, instanceName->Name.value);

    if (shellData)
    {
        /* TODO: Is there an active child requests? */

        /* Decouple shellData from list of active shells */
        ShellData **pointerToPatch = &self->shellList;

        while (*pointerToPatch && (*pointerToPatch != shellData))
        {
            pointerToPatch = (ShellData **) &(*pointerToPatch)->common.siblingData;
        }
        if (*pointerToPatch)
            *pointerToPatch = (ShellData *) shellData->common.siblingData;

        /* Delete the shell object and all the memory owned by it (owned by the batch) */
        Batch_Delete(shellData->common.batch);

        miResult = MI_RESULT_OK;
    }

    MI_Context_PostResult(context, miResult);
}

MI_Boolean ExtractCommandArgs(Shell_Command* shellInstance, WSMAN_COMMAND_ARG_SET *wsmanArgSet, Batch *batch)
{
    if (shellInstance->arguments.exists)
    {
        MI_Uint32 i;

        wsmanArgSet->args = Batch_Get(batch, shellInstance->arguments.value.size * sizeof(MI_Char*));
        if (wsmanArgSet->args == NULL)
        {
            return MI_FALSE;
        }

        for (i = 0; i != shellInstance->arguments.value.size; i++)
        {
            wsmanArgSet->args[i] = shellInstance->arguments.value.data[i];
        }
        wsmanArgSet->argsCount = shellInstance->arguments.value.size;
    }
    return MI_TRUE;
}

MI_Boolean AddChild(CommonData **childHeadPointer, CommonData *childData)
{
    CommonData *currentChild = *childHeadPointer;

    while (currentChild)
    {
        if (currentChild->requestType == childData->requestType)
        {
            /* Already have one of those */
            return MI_FALSE;
        }

        currentChild = currentChild->siblingData;
    }

    /* Not found this type so we can add it */
    childData->siblingData = *childHeadPointer;
    *childHeadPointer = childData;

    return MI_TRUE;
}

/* Initiate a command on a given shell. The command has parameters and the likes
 * and when created inbound/outbound streams are sent/received via the Send/Receive
 * methods while specifying this command ID. The command is alive until a signal is
 * sent telling us is it officially finished.
 */
void MI_CALL Shell_Invoke_Command(Shell_Self* self, MI_Context* context,
    const MI_Char* nameSpace, const MI_Char* className,
    const MI_Char* methodName, const Shell* instanceName,
    const Shell_Command* in)
{
    MI_Result miResult = MI_RESULT_OK;
    ShellData *shellData;
    CommandData *commandData;
    MI_Instance *miOperationInstance = NULL;
    Batch *batch = NULL;

    shellData = FindShell(self, instanceName->Name.value);

    if (!shellData)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    /* Allocate our shell data out of a batch so we can allocate most of it from a single page and free it easily */
    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Create internal command structure.  */
    commandData = Batch_GetClear(batch, sizeof(CommandData));
    if (!commandData)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    commandData->common.batch = batch;

    commandData->commandId = Batch_Get(batch, sizeof(MI_Char)*ID_LENGTH);
    if (!commandData->commandId)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    Stprintf(commandData->commandId, ID_LENGTH, MI_T("%llx"), (MI_Uint64)commandData->commandId);

#if 0
    /* Set up the stream objects for the command. We need to hold extra state so we need to copy it */
    commandData->inputStreams.streamData = Batch_Get(batch, shellData->inputStreams.streamNamesCount * sizeof(StreamData));
    if (!commandData->inputStreams.streamData)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    {
        MI_Uint32 i;
        for (i = 0; i != shellData->inputStreams.streamNamesCount; i++)
        {
            commandData->inputStreams.streamData[i].streamName = shellData->inputStreams.streamNames[i];
        }
    }

    commandData->outputStreams.streamData = Batch_Get(batch, shellData->outputStreams.streamNamesCount * sizeof(StreamData));
    if (!commandData->outputStreams.streamData)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Set up stream name pointers */
    {
        MI_Uint32 i;
        for (i = 0; i != shellData->outputStreams.streamNamesCount; i++)
        {
            commandData->outputStreams.streamData[i].streamName = shellData->outputStreams.streamNames[i];
        }
    }
#endif

    /* Create command instance to send back to client */
    miResult = Instance_Clone(&in->__instance, &miOperationInstance, batch);
    if (miResult != MI_RESULT_OK)
    {
    	GOTO_ERROR(miResult);
    }

    Shell_Command_SetPtr_CommandId((Shell_Command*) miOperationInstance, commandData->commandId);
    
    if (!ExtractCommandArgs((Shell_Command*)miOperationInstance, &commandData->wsmanArgSet, batch))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* TODO: Fill in shellData->command->common.pluginRequest */
    commandData->common.parentData = (CommonData*)shellData;
    commandData->common.requestType = WSMAN_PLUGIN_REQUEST_COMMAND;
    commandData->common.miRequestContext = context;
    commandData->common.miOperationInstance = miOperationInstance;

    if (!AddChild(&shellData->childNext, (CommonData*) commandData))
    {
        GOTO_ERROR(MI_RESULT_ALREADY_EXISTS); /* Only one command allowed at a time */
    }

    WSManPluginCommand(
    		&commandData->common.pluginRequest,
			0,
			shellData->pluginShellContext,
            ((Shell_Command*)miOperationInstance)->command.value, 
            &commandData->wsmanArgSet);

    /* Success path will send the response back from the callback from this API*/
    return;

error:
    /* Cleanup all the memory and post the error back*/

    if (batch)
        Batch_Delete(batch);

    MI_Context_PostResult(context, miResult);

}

CommandData *GetCommand(const ShellData *shell, const MI_Char *commandId)
{
    CommonData *child = shell->childNext;

    if (commandId == NULL)
        return NULL;

    while (child)
    {
        if (child->requestType == WSMAN_PLUGIN_REQUEST_COMMAND)
        {
            CommandData *command = (CommandData*)child;
            if (Tcscmp(commandId, command->commandId) == 0)
            {
                return command;
            }
        }

        child = child->siblingData;
    }

    return NULL;
}
/* Shell_Invoke_Send
 *
 * This CIM method is called when the client is delivering a chunk of data to the shell.
 * It can be delivering it to the shell itself or to a command, depending on if the
 * commandId parameter is present.
 * This test provider will deliver anything that is sent back to the client through
 * a pending Receive methods result.
 */
void MI_CALL Shell_Invoke_Send(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Send* in)
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Uint32 pluginFlags = 0;
    ShellData *shellData = FindShell(self, instanceName->Name.value);
    CommandData *commandData = NULL;
    SendData *sendData = NULL;
    Batch *batch = NULL;
    DecodeBuffer decodeBuffer, decodedBuffer;
    MI_Instance *clonedIn = NULL;
    
    memset(&decodeBuffer, 0, sizeof(decodeBuffer));
	memset(&decodedBuffer, 0, sizeof(decodedBuffer));

	/* Was the shell ID the one we already know about? */
    if (!shellData)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    /* Check to make sure the command ID is correct if this send is aimed at the command */
    if (in->streamData.value->commandId.exists)
    {
        commandData = GetCommand(shellData, in->streamData.value->commandId.value);
        if (commandData == NULL)
        {
            GOTO_ERROR(MI_RESULT_NOT_FOUND);
        }
    }

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    miResult = Instance_Clone(&in->__instance, &clonedIn, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR(miResult);
    }

    /* We may not actually have any data but we may be completing the data. Make sure
     * we are only processing the inbound stream if we have some data to process.
     */
    if (in->streamData.value->data.exists)
    {
        decodeBuffer.buffer = (MI_Char*)in->streamData.value->data.value;
        decodeBuffer.bufferLength = in->streamData.value->dataLength.value * sizeof(MI_Char);
        decodeBuffer.bufferUsed = decodeBuffer.bufferLength;

        /* Base-64 decode the data from decodeBuffer to decodedBuffer. The result buffer
         * gets allocated in this function and we need to free it.*/
         /* TODO: This does not support unicode MI_Char strings */
        miResult = Base64DecodeBuffer(&decodeBuffer, &decodedBuffer);
        if (miResult != MI_RESULT_OK)
        {
            /* decodeBuffer.buffer does not need deleting */
            GOTO_ERROR(miResult);
        }

        /* decodeBuffer.buffer does not need freeing as it was from method in parameters. */
        /* We switch the decodedBuffer to decodeBuffer for further processing. */
        decodeBuffer = decodedBuffer;
        memset(&decodedBuffer, 0, sizeof(decodedBuffer));

        if (shellData->isCompressed)
        {
            /* Decompress it from decodeBuffer to decodedBuffer. The result buffer
             * gets allocated in this function and we need to free it.
             */
            miResult = DecompressBuffer(&decodeBuffer, &decodedBuffer);
            if (miResult != MI_RESULT_OK)
            {
                free(decodeBuffer.buffer);
                decodeBuffer.buffer = NULL;
                GOTO_ERROR(miResult);
            }

            /* Free the previously allocated buffer and switch the
             * decodedBuffer back to decodeBuffer for further processing.
             */
            free(decodeBuffer.buffer);
            decodeBuffer = decodedBuffer;
        }
    }
    else
    {
        pluginFlags = WSMAN_FLAG_SEND_NO_MORE_DATA;
    }
    {
        WSMAN_DATA inboundData;
        memset(&inboundData, 0, sizeof(inboundData));
        inboundData.type = WSMAN_DATA_TYPE_BINARY;
        inboundData.binaryData.data = (MI_Uint8*) decodeBuffer.buffer;
        inboundData.binaryData.dataLength = decodeBuffer.bufferUsed;

        sendData = Batch_GetClear(batch, sizeof(SendData));
        if (sendData == NULL)
        {
            GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        sendData->common.batch = batch;
        sendData->common.miRequestContext = context;
        sendData->common.miOperationInstance = clonedIn;
        sendData->common.requestType = WSMAN_PLUGIN_REQUEST_SEND;
        /* TODO: Fill in sendData->common.pluginRequest */

        if (commandData)
        {
            sendData->common.parentData = (CommonData*)commandData;

            if (!AddChild(&commandData->childNext, (CommonData*)sendData))
            {
                GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
            }

            WSManPluginSend(
                &sendData->common.pluginRequest,
                pluginFlags,
                shellData->pluginShellContext,
                commandData->pluginCommandContext,
                in->streamData.value->streamName.value,
                &inboundData);
        }
        else
        {
            sendData->common.parentData = (CommonData*) shellData;

            if (!AddChild(&shellData->childNext, (CommonData*)sendData))
            {
                GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
            }

            WSManPluginSend(
                &sendData->common.pluginRequest,
                pluginFlags,
                shellData->pluginShellContext,
                NULL,
                in->streamData.value->streamName.value,
                &inboundData);
        }
    }
    /* Now the plugin has been called the result is sent from the WSManPluginOperationComplete callback */
    free(decodedBuffer.buffer);
    return;

error:
	free(decodedBuffer.buffer);

	if (miResult == MI_RESULT_OK)
    {
        Shell_Send send;
        Shell_Send_Construct(&send, context);
        Shell_Send_Set_MIReturn(&send, MI_RESULT_OK);
        Shell_Send_Post(&send, context);
        Shell_Send_Destruct(&send);
    }

	MI_Context_PostResult(context, miResult);

    if (batch)
        Batch_Delete(batch);
}

/* Shell_Invoke_Receive
 * This gets called to queue up a receive of output from the provider when there is enough
 * data to send.
 * In our test provider all we are doing is sending the data back that we received in Send
 * so we are caching the Receive context and wake up any pending Send that is waiting
 * for us.
 */
void MI_CALL Shell_Invoke_Receive(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Receive* in)
{
    MI_Result miResult = MI_RESULT_OK;
    ShellData *shellData = FindShell(self, instanceName->Name.value);
    CommandData *commandData = NULL;
    ReceiveData *receiveData = NULL;
    Batch *batch = NULL;
    MI_Instance *clonedIn = NULL;

    if (!shellData)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }
    /* If we have a command ID make sure it is the correct one */
    if (in->commandId.exists)
    {
        commandData = GetCommand(shellData, in->commandId.value);
        if (commandData == NULL)
        {
            GOTO_ERROR(MI_RESULT_NOT_FOUND);
        }

        /* Find an existing receiveData is one exists */
        {
            CommonData *tmp = commandData->childNext;
            while (tmp && (tmp->requestType != WSMAN_PLUGIN_REQUEST_RECEIVE))
            {
                tmp = tmp->siblingData;
            }
            receiveData = (ReceiveData*) tmp;
        }
    }
    else
    {
        /* Find an existing receiveData is one exists */
        CommonData *tmp = shellData->childNext;
        while (tmp && (tmp->requestType != WSMAN_PLUGIN_REQUEST_RECEIVE))
        {
            tmp = tmp->siblingData;
        }
        receiveData = (ReceiveData*)tmp;
    }


    if (receiveData)
    {
        /* We already have a Receive queued up with the plug-in so cache the context and wake it up in case it is waiting for it */
        receiveData->common.miRequestContext = context;
        CondLock_Broadcast((ptrdiff_t)&receiveData->common.miRequestContext);
        return;
    }

    /* new one */
    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    miResult = Instance_Clone(&in->__instance, &clonedIn, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR(miResult);
    }

    receiveData = Batch_GetClear(batch, sizeof(ReceiveData));
    if (receiveData == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    receiveData->common.batch = batch;
    receiveData->common.miRequestContext = context;
    receiveData->common.miOperationInstance = clonedIn;
    receiveData->common.requestType = WSMAN_PLUGIN_REQUEST_RECEIVE;
    /* TODO: Fill in receiveData->common.pluginRequest */


    if (commandData)
    {
        receiveData->common.parentData = (CommonData*)commandData;

        if (!AddChild(&commandData->childNext, (CommonData*)receiveData))
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }

        WSManPluginReceive(
            &receiveData->common.pluginRequest,
            0,
            shellData->pluginShellContext,
            commandData->pluginCommandContext,
            NULL);
    }
    else
    {
        receiveData->common.parentData = (CommonData*)shellData;

        if (!AddChild(&shellData->childNext, (CommonData*)receiveData))
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }

        WSManPluginReceive(
            &receiveData->common.pluginRequest,
            0,
            shellData->pluginShellContext,
            NULL,
            NULL);
    }

    /* Posting on receive context happens when we get WSManPluginOperationComplete callback to terminate the request or WSManPluginReceiveResult with some data */
    return;

error:
    MI_Context_PostResult(context, miResult);
    
    if (batch)
    {
        Batch_Delete(batch);
    }
}

/* Shell_Invoke_Signal
 * This function is called for a few reasons like hitting Ctrl+C. It is
 * also called to signal the completion of a command from the clients perspective
 * and the command is not considered complete until this fact.
 * In reality we may time out a command if we don't receive any data
 * but for the sake of this provider we don't care.
 */
void MI_CALL Shell_Invoke_Signal(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Signal* in)
{
    MI_Result miResult = MI_RESULT_OK;
    ShellData *shellData = FindShell(self, instanceName->Name.value);
    CommandData *commandData = NULL;
    SignalData *signalData = NULL;
    Batch *batch = NULL;
    MI_Instance *clonedIn = NULL;

    if (!shellData)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }
    if (!in->code.exists)
    {
        GOTO_ERROR(MI_RESULT_NOT_SUPPORTED);
    }
    /* If we have a command ID make sure it is the correct one */
    if (in->commandId.exists)
    {
        commandData = GetCommand(shellData, in->commandId.value);
        if (commandData == NULL)
        {
            GOTO_ERROR(MI_RESULT_NOT_FOUND);
        }
    }

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    miResult = Instance_Clone(&in->__instance, &clonedIn, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR(miResult);
    }

    signalData = Batch_GetClear(batch, sizeof(SignalData));
    if (signalData == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    signalData->common.batch = batch;
    signalData->common.miRequestContext = context;
    signalData->common.miOperationInstance = clonedIn;
    signalData->common.requestType = WSMAN_PLUGIN_REQUEST_RECEIVE;
    /* TODO: Fill in signalData->common.pluginRequest */

    if (commandData)
    {
        signalData->common.parentData = (CommonData*)commandData;

        if (!AddChild(&commandData->childNext, (CommonData*)signalData))
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }

        WSManPluginSignal(
            &signalData->common.pluginRequest,
            0,
            shellData->pluginShellContext,
            commandData->pluginCommandContext,
            ((Shell_Signal*)clonedIn)->code.value);
    }
    else
    {
        signalData->common.parentData = (CommonData*)shellData;

        if (!AddChild(&shellData->childNext, (CommonData*)signalData))
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }

        WSManPluginSignal(
            &signalData->common.pluginRequest,
            0,
            shellData->pluginShellContext,
            NULL,
            ((Shell_Signal*)clonedIn)->code.value);
    }

    /* Posting on signal context happens when we get a WSManPluginOperationComplete callback */
    return;

error:
    MI_Context_PostResult(context, miResult);

    if (batch)
    {
        Batch_Delete(batch);
    }
}

/* Connect allows the client to re-connect to a shell that was started from a different remote machine and continue getting output delivered to a new client */
void MI_CALL Shell_Invoke_Connect(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Connect* in)
{
    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}


/* report a shell or command context from teh winrm plugin. We use this for future calls into the plugin.
 This also means the shell or command has been started successfully so we can post the operation instance
 back to the client to indicate to them that they can now post data to the operation or receive data back from it.
 */
MI_Uint32 MI_CALL WSManPluginReportContext(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ void * context
    )
{
    CommonData *commonData = (CommonData*) requestDetails;
    MI_Result miResult;

    /* Grab the providers context, which may be shell or command, and store it in our object */
    if (commonData->requestType == WSMAN_PLUGIN_REQUEST_SHELL)
    {
        ((ShellData*)commonData)->pluginShellContext = context;
    }
    else if (commonData->requestType == WSMAN_PLUGIN_REQUEST_COMMAND)
    {
        ((CommandData*)commonData)->pluginCommandContext = context;
    }
    else
    {
        return MI_RESULT_INVALID_PARAMETER;
    }

    /* Post our shell or command object back to the client */
    if (((miResult = MI_Context_PostInstance(commonData->miRequestContext, commonData->miOperationInstance)) != MI_RESULT_OK) ||
        ((miResult = MI_Context_PostResult(commonData->miRequestContext, miResult)) != MI_RESULT_OK))
    {
        /* Something failed. They will call WSManPluginOperationComplete to report it is done/failed though so we will clean up in there */
    }
	return miResult;
}

/* Max recursion is for a child operation of a command which is 2 recursions to hit the shell */
MI_Boolean IsStreamCompressed(CommonData *commonData)
{
    if (commonData->parentData == NULL)
    {
        ShellData *shellData = (ShellData*)commonData;
        return shellData->isCompressed;
    }
    return IsStreamCompressed(commonData->parentData);
}

/*
 * The WSMAN plug-in gets called once and it keeps sending data back to us.
 * At our level, however, we need to potentially wait for the next request
 * before we can send more.
 */
MI_Uint32 MI_CALL WSManPluginReceiveResult(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_opt_ const MI_Char * stream,
    _In_opt_ WSMAN_DATA *streamResult,
    _In_opt_ const MI_Char * commandState,
    _In_ MI_Uint32 exitCode
    )
{
    MI_Result miResult;
    CommonData *commonData = (CommonData*)requestDetails;
    MI_Context *receiveContext = NULL;
    CommandState commandStateInst;
    Shell_Receive *receive = NULL;
    Stream receiveStream;
    DecodeBuffer decodeBuffer, decodedBuffer;
    memset(&decodeBuffer, 0, sizeof(decodeBuffer));
    memset(&decodedBuffer, 0, sizeof(decodedBuffer));

    if (commonData->requestType != WSMAN_PLUGIN_REQUEST_RECEIVE)
        return MI_RESULT_INVALID_PARAMETER;

    /* TODO: Which stream if stream is NULL? */

    /* Wait for a Receive request to come in before we post the result back */
    do
    {
    } while (CondLock_Wait((ptrdiff_t)&commonData->miRequestContext,
                           (ptrdiff_t*)&commonData->miRequestContext,
                           0, 
                           CONDLOCK_DEFAULT_SPINCOUNT) == 0);
    
    /* Copy off the receive context. Another Receive should not happen at the same time but this makes
    * sure we are the only one processing it. This will also help to protect us if a Signal comes in
    * to shut things down as we are now responsible for delivering its results.
    */
    receiveContext = commonData->miRequestContext;
    commonData->miRequestContext = NULL;

    /* Construct our instance result objects */
    receive = (Shell_Receive*)commonData->miOperationInstance;
    commonData->miOperationInstance = NULL;

    miResult =  CommandState_Construct(&commandStateInst, receiveContext);
    if (miResult != MI_RESULT_OK)
    {
        Shell_Receive_Delete(receive);
        GOTO_ERROR_EX(miResult, errorSkipInstanceDeletes);
    }
    miResult = Stream_Construct(&receiveStream, receiveContext);
    if (miResult != MI_RESULT_OK)
    {
        Stream_Destruct(&receiveStream);
        Shell_Receive_Delete(receive);
        GOTO_ERROR_EX(miResult, errorSkipInstanceDeletes);
    }

    /* Set the command ID for the instances that need it */
    if (commonData->parentData->requestType == WSMAN_PLUGIN_REQUEST_COMMAND)
    {
        CommandData *commandData = (CommandData*)commonData->parentData;
        CommandState_SetPtr_commandId(&commandStateInst, commandData->commandId);
        Stream_SetPtr_commandId(&receiveStream, commandData->commandId);
    }


    
    /* TODO: Does powershell use binary data rather than string? */
    decodeBuffer.buffer = (MI_Char*) streamResult->binaryData.data;
    decodeBuffer.bufferLength = streamResult->binaryData.dataLength;
    decodeBuffer.bufferUsed = decodeBuffer.bufferLength;

    if (IsStreamCompressed(commonData))
    {
		/* Re-compress it from decodeBuffer to decodedBuffer. The result buffer
		 * gets allocated in this function and we need to free it.
		 */
		miResult = CompressBuffer(&decodeBuffer, &decodedBuffer, sizeof(MI_Char));
		if (miResult != MI_RESULT_OK)
		{
			decodeBuffer.buffer = NULL;
			GOTO_ERROR(miResult);
		}

		/* switch the decodedBuffer back to decodeBuffer for further processing.
		 */
		decodeBuffer = decodedBuffer;
	}

    /* TODO: This does not support unicode MI_Char strings */
    /* NOTE: Base64EncodeBuffer allocates enough space for a NULL terminator */
    miResult = Base64EncodeBuffer(&decodeBuffer, &decodedBuffer);

    /* Free previously allocated buffer if it was compressed. */
    if (IsStreamCompressed(commonData))
    {
        free(decodeBuffer.buffer);
    }
    decodeBuffer.buffer = NULL;

    if (miResult != MI_RESULT_OK)
    {
    	GOTO_ERROR(miResult);
    }

    /* Set the null terminator on the end of the buffer as this is supposed to be a string*/
    memset(decodedBuffer.buffer+decodedBuffer.bufferUsed, 0, sizeof(MI_Char));

    /* CommandState tells the client if we are done or not with this stream.
    */

    if (commandState &&
        (Tcscasecmp(commandState, WSMAN_COMMAND_STATE_DONE) == 0))
    {
        /* TODO: Mark stream as complete for either shell or command */
        CommandState_SetPtr_state(&commandStateInst, WSMAN_COMMAND_STATE_DONE);

#if 0
        MI_Uint32 i;
        /* find and mark the stream as done in our records for when command is terminated and we need to terminate streams */
        for (i = 0; i != commandData->shellData->outboundStreamNamesCount; i++)
        {
            if (Tcscmp(stream, commandData->outputStreams[i].streamName))
            {
                commandData->outputStreams[i].done = MI_TRUE;
                break;
            }
        }
#endif
    }
    else if (commandState)
    {
        CommandState_SetPtr_state(&commandStateInst, commandState);
    }
    else
    {
        CommandState_SetPtr_state(&commandStateInst, WSMAN_COMMAND_STATE_RUNNING);
    }

    /* Stream holds the results of the inbound/outbound stream. A result can have more
    * than one stream, either for the same stream or different ones.
    */
    if (flags & WSMAN_FLAG_RECEIVE_RESULT_NO_MORE_DATA)
        Stream_Set_endOfStream(&receiveStream, MI_TRUE);
    else
        Stream_Set_endOfStream(&receiveStream, MI_FALSE);

    Stream_SetPtr_streamName(&receiveStream, stream);

    /* The result of the Receive contains the command results and a set of streams.
    * We only support one stream at a time for now.
    */
    Shell_Receive_Set_MIReturn(receive, MI_RESULT_OK);
    Shell_Receive_SetPtr_CommandState(receive, &commandStateInst);

    /* Add the final string to the stream. This is just a pointer to it and
     * is not getting copied so we need to delete the buffer after we have posted the instance
     * to the receive context.
     */
    Stream_SetPtr_data(&receiveStream, decodedBuffer.buffer);

    /* Add the stream embedded instance to the receive result.  */
    Shell_Receive_SetPtr_Stream(receive, &receiveStream);

    /* Post the result back to the client. We can delete the base-64 encoded buffer after
    * posting it.
    */
    Shell_Receive_Post(receive, receiveContext);

error:
    /* Clean up the various result objects */
    CommandState_Destruct(&commandStateInst);
    Stream_Destruct(&receiveStream);
    Shell_Receive_Delete(receive);

errorSkipInstanceDeletes:
    MI_Context_PostResult(receiveContext, miResult);
    
    return (MI_Uint32) miResult;

}

/* PowerShell only uses WSMAN_PLUGIN_PARAMS_GET_REQUESTED_LOCALE and WSMAN_PLUGIN_PARAMS_GET_REQUESTED_DATA_LOCALE 
 * which are optional wsman packet headers.
 * Other things that could be retrieved are things like operation timeout. 
 * This data should be retrieved from the operation context through the operation options method.
 */
MI_Uint32 MI_CALL WSManPluginGetOperationParameters (
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _Out_ WSMAN_DATA *data
    )
{
	return (MI_Uint32) MI_RESULT_FAILED;
}

/* Gets information about the host the provider is hosted in */
MI_Uint32 MI_CALL WSManPluginGetConfiguration (
  _In_   void * pluginOperationContext,
  _In_   MI_Uint32 flags,
  _Out_  WSMAN_DATA *data
)
{
    return (MI_Uint32)MI_RESULT_FAILED;
}

/*
* Reports the completion of an operation by all operation entry points except for the WSMAN_PLUGIN_STARTUP and WSMAN_PLUGIN_SHUTDOWN methods.
*/
MI_Uint32 MI_CALL WSManPluginOperationComplete(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ MI_Uint32 errorCode,
    _In_opt_ const MI_Char * extendedInformation
    )
{
    CommonData *commonData = (CommonData*) requestDetails;
    MI_Result miResult;

    /* Question is: which request is this? */
    switch (commonData->requestType)
    {
    case WSMAN_PLUGIN_REQUEST_SHELL:
        /* TODO: Shell has completed. We should get no more calls after this */
        /* Remove the shell data object from the shell list */
        /* Delete the shell batch */
        break;
    case WSMAN_PLUGIN_REQUEST_COMMAND:
        /* TODO: This command is complete. No more calls for this command should happen */
        /* Remove the command data pointer from the owning shell */
        /* Delete the command batch */
        break;
    case WSMAN_PLUGIN_REQUEST_RECEIVE:
        /* TODO: This is the termination of the receive. No more will happen for either the command or shell, depending on which is is aimed at */
        /* We may or may not have a pending Receive protocol packet for this depending on if we have already send a command completion message or not */
        if (commonData->miRequestContext)
        {
            /* We have a pending request that needs to be terminated */
            WSManPluginReceiveResult(requestDetails, WSMAN_FLAG_RECEIVE_RESULT_NO_MORE_DATA, NULL, NULL, WSMAN_COMMAND_STATE_DONE, errorCode);
        }
        /* TODO: Clean-up here */
        /* TODO: Remove this item from the parent object */
        /* TODO: Delete the batch */
        break;

    /* Send/Receive only need to post back the operation instance with the MIReturn code set */
    case WSMAN_PLUGIN_REQUEST_SIGNAL:
    case WSMAN_PLUGIN_REQUEST_SEND:
    {
        MI_Value miValue;
        MI_Context *miRequestContext = commonData->miRequestContext;
        MI_Instance *miOperationInstance = commonData->miOperationInstance;

        commonData->miRequestContext = NULL;
        commonData->miOperationInstance = NULL;

        /* Methods only have the return code set in the instance so set that and post back. */
        miValue.uint32 = errorCode;
        MI_Instance_SetElement(
            miOperationInstance,
            MI_T("MIReturn"),
            &miValue,
            MI_UINT32,
            0);
        
        /* If this is the end signal code for the command we need to delete the commandData */
        if (commonData->requestType == WSMAN_PLUGIN_REQUEST_SIGNAL)
        {
            Shell_Signal *signalInstance = (Shell_Signal*)miOperationInstance;

            if ((commonData->parentData->requestType == WSMAN_PLUGIN_REQUEST_COMMAND) &&
                signalInstance->code.value &&
                ((Tcscasecmp(signalInstance->code.value, WSMAN_SIGNAL_CODE_EXIT) == 0) ||
                 (Tcscasecmp(signalInstance->code.value, WSMAN_SIGNAL_CODE_TERMINATE) == 0)))
            {
                /* The command has now completed and everything needs to be deleted */
                CommandData *commandData = (CommandData *)commonData->parentData;
                commandData->shellData->command = NULL;
                Batch_Delete(commandData->common.batch);
            }
        }

        miResult = MI_Context_PostInstance(miRequestContext, miOperationInstance);
        MI_Context_PostResult(miRequestContext, miResult);

        MI_Instance_Delete(miOperationInstance);

        /* Remove self from parent */
        /* Delete out batch */
        
        break;
    }
    }

    return MI_RESULT_OK;
}

/* Seems to be called to say the plug-in is actually done */
MI_Uint32 MI_CALL WSManPluginReportCompletion(
  _In_      void * pluginOperationContext,
  _In_      MI_Uint32 flags
  )
{
    return (MI_Uint32)MI_RESULT_OK;
}

/* Free up information inside requestDetails if possible */
MI_Uint32 MI_CALL WSManPluginFreeRequestDetails(_In_ WSMAN_PLUGIN_REQUEST *requestDetails)
{
	return MI_RESULT_OK;
}
