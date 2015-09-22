#include <MI.h>
#include "Shell.h"
#include "ShellAPI.h"
#include "BufferManipulation.h"
#include <pal/strings.h>
#include <pal/format.h>
#include <pal/lock.h>

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

/* Index for PluginRequest arrays as well as the type of the PluginRequest */
typedef enum
{
    WSMAN_PLUGIN_REQUEST_OPERATION = 0,
    WSMAN_PLUGIN_REQUEST_SEND = 1,
    WSMAN_PLUGIN_REQUEST_RECEIVE = 2,
    WSMAN_PLUGIN_REQUEST_SIGNAL = 3
} PluginRequest_Type;

typedef struct _CommonData CommonData;

typedef struct _PluginRequest
{
    /* Request data passed into all plugin operations */
    WSMAN_PLUGIN_REQUEST pluginRequest;

    /* Pointer to the owning operation data, either command or shell */
    CommonData *commonData;

    PluginRequest_Type requestType;

    /* Associated MI_Context for the WSMAN plugin request */
    MI_Context *miRequestContext;

    /* MI_Instance that was passed in for creating instance, or the parameter object passed in to the operation method */
    MI_Instance *miOperationInstance;
} PluginRequest;

struct _CommonData
{
	/* indexes into pluginRequest array and context array... operation is either for shell or operation depending on the owning data object type */
    PluginRequest pluginRequest[4];

    /* common data can be part of either a shell or command data object */
    enum { 
        CommonData_Type_Shell, 
        CommonData_Type_Command 
    } dataType;

    /* WSMAN shell Plug-in context is the context reported from either the shell or command depending on which type it is */
    void * pluginOperationContext;
} ;

typedef struct _ShellData ShellData;
typedef struct _CommandData CommandData;

/* Command remembers the receive context so it can deliver
 * results to it when they are available.
 */
struct _CommandData
{
	CommonData common;

    /* This commands ID. There is only one command per shell,
     * but there may be more than one shell in a process.
     */
    MI_Char *commandId;

    /* array of outbound streams holding the state as to if they are
     * completed or not. */
    MI_Uint32 numberOutboundStreams;
    StreamData *outboundStreams;

    /* The shell that owns this command */
    ShellData *shellData;

};

/* List of active shells. Only one command per shell is supported. */
struct _ShellData
{
    CommonData common;
    
    /* This is part of a linked list of shells */
    ShellData *nextShell;

    /* This shells ID */
    MI_Char *shellId;

    /* Only support a single command, so pointer to the command if one is currenty running */
    CommandData *command;

    MI_Uint32 outboundStreamNamesCount;
    MI_Char **outboundStreamNames;

    /* Is the inbound/outbound streams compressed? */
    MI_Boolean isCompressed;

    /* MI provider self pointer that is returned from MI provider load which holds all shell state. When our shell
     * goes away we will need to remove outself from the list 
    */
    Shell_Self *shell;

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
    ShellData *thisShell = shell->shellList;

    while (thisShell && (Tcscmp(shellId, thisShell->shellId) != 0))
    {
        thisShell = thisShell->nextShell;
    }

    return thisShell;
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
		miResult = MI_Context_PostInstance(context, shellData->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miOperationInstance);
		if (miResult != MI_RESULT_OK)
		{
			break;
		}

		shellData = shellData->nextShell;
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
    ShellData *thisShell = FindShell(self, instanceName->Name.value);

    if (thisShell)
    {
        miResult = MI_Context_PostInstance(context, thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miOperationInstance);
    }
    MI_Context_PostResult(context, miResult);
}

/* ExtractOutboundStreams takes a list of streams that are space delimited and
 * copies the data into the ShellData object for the stream. These stream
 * names are allocated and so will need to be deleted when the shell is deleted.
 * We allocate a single buffer for the array and the string, then insert null terminators
 * into the string where spaces were and fix up the array pointers to these strings.
 */
static MI_Boolean ExtractOutboundStreams(const MI_Char *streams, ShellData *thisShell)
{
	MI_Char *cursor = (MI_Char*) streams;
	MI_Uint32 i;
	int stringLength = Tcslen(streams)+1; /* string length including null terminator */

	/* Count how many stream names we have so we can allocate the array */
	while (cursor && *cursor)
	{
		thisShell->outboundStreamNamesCount++;
		cursor = Tcschr(cursor, MI_T(' '));
		if (cursor && *cursor)
		{
			cursor++;
		}
	}

	/* Allocate the buffer for the array and all strings inside the array */
	thisShell->outboundStreamNames = malloc(
			  (sizeof(MI_Char*) * thisShell->outboundStreamNamesCount) /* array length */
			+ (sizeof(MI_Char) * stringLength)); /* Length for the copied stream names */
	if (thisShell->outboundStreamNames == NULL)
		return MI_FALSE;

	/* Set the cursor to the start of where the strings are placed in the buffer */
	cursor = (MI_Char*) thisShell->outboundStreamNames + (sizeof(MI_Char*) * thisShell->outboundStreamNamesCount);

	/* Copy the stream names across to the correct place */
	Tcslcpy(cursor, streams, stringLength);

	/* Loop through the streams fixing up the array pointers and replace spaces with null terminator */
	for (i = 0; i != thisShell->outboundStreamNamesCount; i++)
	{
		/* Fix up the pointer to the stream name */
		thisShell->outboundStreamNames[i] = cursor;

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

/* Shell_CreateInstance
 * Called by the client to create a shell. The shell is given an ID by us and sent back.
 * The list of streams that a command could have is listed out in the shell instance passed
 * to us. Other stuff can be included but we are ignoring that for the moment.
 */
void MI_CALL Shell_CreateInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* newInstance)
{
    ShellData *thisShell = 0;
    MI_Result miResult;
    MI_Instance *miOperationInstance;

    /* Shell instance should have the list of output stream names. */
    if (!newInstance->OutputStreams.exists)
    {
        MI_Context_PostResult(context, MI_RESULT_INVALID_PARAMETER);
        return;
    }

    /* Create an internal representation of the shell object that can can use to
     * hold state. It is based on this recored data that a client should be able
     * to do an EnumerateInstance call to the provider.
     * We also allocate enough space for the shell ID string.
     */
    thisShell = calloc(1, sizeof(*thisShell) + (sizeof(MI_Char)*ID_LENGTH));
    if (thisShell == 0)
    {
        MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
        return;
    }

    /* Set the shell ID to be the pointer within the bigger buffer */
    thisShell->shellId = (MI_Char*) (((char*)thisShell)+sizeof(*thisShell));
    if (Stprintf(thisShell->shellId, ID_LENGTH, MI_T("%llx"), (MI_Uint64) thisShell->shellId) < 0)
    {
    	free(thisShell);
        MI_Context_PostResult(context, MI_RESULT_FAILED);
    	return;
    }

    /* Extract the outbound stream names that are space delimited into an
     * actual array of strings
     */
    if (!ExtractOutboundStreams(newInstance->OutputStreams.value, thisShell))
    {
    	free(thisShell);
        MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
    	return;
    }

    /* Plumb this shell into our list. Failure paths after this need to
     * unplumb it!
     */
    thisShell->nextShell = self->shellList;
    self->shellList = thisShell;
    thisShell->shell = self;

    {
    	MI_Value value;
    	MI_Type type;
    	MI_Uint32 flags;
    	MI_Uint32 index;

		if ((MI_Instance_GetElement(&newInstance->__instance, MI_T("IsCompressed"), &value, &type, &flags, &index) == 0) &&
				(type == MI_BOOLEAN) &&
				value.boolean)
		{
			thisShell->isCompressed = MI_TRUE;
		}
    }

    /* Create an instance of the shell and send it back */
    if ((miResult = MI_Instance_Clone(&newInstance->__instance, &miOperationInstance)) != MI_RESULT_OK)
    {
        free(thisShell);
        MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
        return;
    }
    if ((miResult = Shell_SetPtr_Name((Shell*)miOperationInstance, thisShell->shellId)) != MI_RESULT_OK)
    {
        MI_Instance_Delete(miOperationInstance);
        free(thisShell);
        MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
        return;
    }

    thisShell->common.dataType = CommonData_Type_Shell;
    /* TODO: Fill in thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].pluginRequest */
    thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].commonData = &thisShell->common;
    thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].requestType = WSMAN_PLUGIN_REQUEST_OPERATION;
    thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miRequestContext = context;
    thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miOperationInstance = miOperationInstance;

    /* Call out to external plug-in API to continue shell creation.
     * Acceptance of shell is reported through WSManPluginReportContext.
     * If the shell succeeds or fails we will get a call through WSManPluginOperationComplete
     */
    WSManPluginShell(self->pluginContext, &thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].pluginRequest, 0, NULL, NULL);
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
    ShellData *thisShell = self->shellList;
    ShellData **previous = &self->shellList;
    MI_Result miResult = MI_RESULT_NOT_FOUND;

    /* Find and remove this shell from the list */
    while (thisShell
            && (Tcscmp(instanceName->Name.value, thisShell->shellId) != 0))
    {
        previous = &thisShell->nextShell;
        thisShell = thisShell->nextShell;
    }
    if (thisShell)
    {
        *previous = thisShell->nextShell;
        /* TODO: Is there an active command? */
        /* TODO: Is shell active? */

        /* Delete the stream list buffer -- they are all allocated out of a single buffer */
        free(thisShell->outboundStreamNames);

        /* Delete the instance representing the shell */
        MI_Instance_Delete(thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miOperationInstance);

        /* TODO: Decouple shell from list of active shells */

        /* Delete the shell object which also contains the shell ID string */
        free(thisShell);

        miResult = MI_RESULT_OK;
    }

    MI_Context_PostResult(context, miResult);
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
    ShellData *thisShell;
    MI_Uint32 i;
    MI_Instance *miOperationInstance = NULL;

    thisShell = FindShell(self, instanceName->Name.value);

    if (!thisShell)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    if (thisShell->command)
    {
        /* Command already exists on this shell so fail operation */
        GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
    }

    /* Create internal command structure. Allocate enough space for our stream list as well as command ID */
    thisShell->command = calloc(1,
    		(sizeof(*thisShell->command) +  /* command object */
    				(sizeof(MI_Char)*ID_LENGTH) +  /* command ID */
    				thisShell->outboundStreamNamesCount * sizeof(*thisShell->command->outboundStreams)) /* stream name array */
					);
    if (!thisShell->command)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    thisShell->command->commandId = (MI_Char*)(((char*)thisShell->command) + sizeof(*thisShell->command));
    Stprintf(thisShell->command->commandId, ID_LENGTH, MI_T("%llx"), (MI_Uint64) thisShell->command->commandId);

    /* Set up the stream objects for the command. We need to hold extra state so we need to copy it */
    thisShell->command->outboundStreams = (StreamData*)(((char*)thisShell->command) + sizeof(*thisShell->command) +  (sizeof(MI_Char)*ID_LENGTH));

    /* Set up stream name pointers */
    for (i = 0; i != thisShell->outboundStreamNamesCount; i++)
    {
    	thisShell->command->outboundStreams[i].streamName = thisShell->outboundStreamNames[i];
    }

    /* Set up rest of reference data for command */
    thisShell->command->shellData = thisShell;
    thisShell->command->common.dataType = CommonData_Type_Command;

    /* Create command instance to send back to client */
    miResult = MI_Instance_Clone(&in->__instance, &miOperationInstance);
    if (miResult != MI_RESULT_OK)
    {
    	GOTO_ERROR(miResult);
    }

    Shell_Command_SetPtr_CommandId((Shell_Command*) miOperationInstance, thisShell->command->commandId);

    /* TODO: Fill in thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].pluginRequest */
    thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].commonData = &thisShell->command->common;
    thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miRequestContext = context;
    thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].requestType = WSMAN_PLUGIN_REQUEST_OPERATION;
    thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miOperationInstance = miOperationInstance;

    WSManPluginCommand(
    		&thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].pluginRequest,
			0,
			thisShell->common.pluginOperationContext,
			NULL, NULL);

    /* Success path will send the response back from the callback from this API*/
    return;

error:
    /* Cleanup all the memory and post the error back*/
    if (miOperationInstance)
    {
        MI_Instance_Delete(miOperationInstance);
    }
    free(thisShell->command);
    thisShell->command = NULL;
    MI_Context_PostResult(context, miResult);

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
    ShellData *thisShell = FindShell(self, instanceName->Name.value);
	DecodeBuffer decodeBuffer, decodedBuffer;
    MI_Instance *clonedIn = NULL;
    
    memset(&decodeBuffer, 0, sizeof(decodeBuffer));
	memset(&decodedBuffer, 0, sizeof(decodedBuffer));

	/* Was the shell ID the one we already know about? */
    if (!thisShell)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    /* Check to make sure the command ID is correct if this send is aimed at the command */
    if (in->streamData.value->commandId.exists)
    {
        if (!in->streamData.value->commandId.value || !thisShell->command ||
            (Tcscmp(in->streamData.value->commandId.value, thisShell->command->commandId) != 0))
        {
            GOTO_ERROR(MI_RESULT_NOT_FOUND);
        }
    }

    miResult = MI_Instance_Clone(&in->__instance, &clonedIn);
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

        if (thisShell->isCompressed)
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

        if (in->streamData.value->commandId.exists)
        {
            if (thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].miRequestContext)
            {
                GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
            }
            /* TODO: Fill in thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].pluginRequest */
            thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].commonData = &thisShell->command->common;
            thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].requestType = WSMAN_PLUGIN_REQUEST_SEND;
            thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].miRequestContext = context;
            thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].miOperationInstance = clonedIn;

            WSManPluginSend(
                &thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].pluginRequest,
                pluginFlags,
                thisShell->common.pluginOperationContext,
                thisShell->command->common.pluginOperationContext,
                in->streamData.value->streamName.value,
                &inboundData);
        }
        else
        {
            if (thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].miRequestContext)
            {
                GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
            }
            /* TODO: Fill in thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].pluginRequest */
            thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].commonData = &thisShell->common;
            thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].requestType = WSMAN_PLUGIN_REQUEST_SEND;
            thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].miRequestContext = context;
            thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].miOperationInstance = clonedIn;

            WSManPluginSend(
                &thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND].pluginRequest,
                pluginFlags,
                thisShell->common.pluginOperationContext,
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

    if (clonedIn)
    {
        MI_Instance_Delete(clonedIn);
    }
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
    ShellData *thisShell = FindShell(self, instanceName->Name.value);
    MI_Instance *clonedIn = NULL;

    if (!thisShell)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }
    /* If we have a command ID make sure it is the correct one */
    if (in->commandId.exists && (Tcscmp(in->commandId.value, thisShell->command->commandId) != 0))
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    miResult = MI_Instance_Clone(&in->__instance, &clonedIn);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR(miResult);
    }

    if (in->commandId.exists)
    {
        if (thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].miRequestContext)
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }
        /* TODO: Fill in thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].pluginRequest */
        thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].commonData = &thisShell->command->common;
        thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].requestType = WSMAN_PLUGIN_REQUEST_RECEIVE;
        thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].miRequestContext = context;
        thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].miOperationInstance = clonedIn;

        WSManPluginReceive(
            &thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].pluginRequest,
            0,
            thisShell->common.pluginOperationContext,
            thisShell->command->common.pluginOperationContext,
            NULL);
    }
    else
    {
        if (thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].miRequestContext)
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }

        /* TODO: Fill in thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].pluginRequest */
        thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].commonData = &thisShell->common;
        thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].requestType = WSMAN_PLUGIN_REQUEST_RECEIVE;
        thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].miRequestContext = context;
        thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].miOperationInstance = clonedIn;

        WSManPluginReceive(
            &thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE].pluginRequest,
            0,
            thisShell->common.pluginOperationContext,
            NULL,
            NULL);
    }

    /* Posting on receive context happens when we get WSManPluginOperationComplete callback to terminate the request or WSManPluginReceiveResult with some data */
    return;

error:
    MI_Context_PostResult(context, miResult);
    if (clonedIn)
    {
        MI_Instance_Delete(clonedIn);
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
    ShellData *thisShell = FindShell(self, instanceName->Name.value);
    MI_Instance *clonedIn = NULL;

    if (!thisShell)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }
    if (!in->code.exists)
    {
        GOTO_ERROR(MI_RESULT_NOT_SUPPORTED);
    }
    /* If we have a command ID make sure it is the correct one */
    if (in->commandId.exists && (Tcscmp(in->commandId.value, thisShell->command->commandId) != 0))
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    miResult = MI_Instance_Clone(&in->__instance, &clonedIn);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR(miResult);
    }

    if (in->commandId.exists)
    {
        if (thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].miRequestContext)
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }
        /* TODO: Fill in thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].pluginRequest */
        thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].commonData = &thisShell->command->common;
        thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].requestType = WSMAN_PLUGIN_REQUEST_SIGNAL;
        thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].miRequestContext = context;
        thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].miOperationInstance = clonedIn;

        WSManPluginSignal(
            &thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].pluginRequest,
            0,
            thisShell->common.pluginOperationContext,
            thisShell->command->common.pluginOperationContext,
            in->code.value);
    }
    else
    {
        if (thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].miRequestContext)
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }
        /* TODO: Fill in thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].pluginRequest */
        thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].commonData = &thisShell->common;
        thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].requestType = WSMAN_PLUGIN_REQUEST_SIGNAL;
        thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].miRequestContext = context;
        thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].miOperationInstance = clonedIn;

        WSManPluginSignal(
            &thisShell->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SIGNAL].pluginRequest,
            0,
            thisShell->common.pluginOperationContext,
            NULL,
            in->code.value);
    }

    /* Posting on signal context happens when we get a WSManPluginOperationComplete callback */
    return;

error:
    MI_Context_PostResult(context, miResult);
    if (clonedIn)
    {
        MI_Instance_Delete(clonedIn);
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
    PluginRequest *pluginRequest = (PluginRequest*) requestDetails;
	CommonData *commonData = pluginRequest->commonData;
    MI_Result miResult;

    /* Grab the providers context, which may be shell or command, and store it in our object */
    commonData->pluginOperationContext = context;

    /* Post our shell or command object back to the client */
    if (((miResult = MI_Context_PostInstance(commonData->pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miRequestContext, commonData->pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miOperationInstance)) != MI_RESULT_OK) ||
        ((miResult = MI_Context_PostResult(commonData->pluginRequest[WSMAN_PLUGIN_REQUEST_OPERATION].miRequestContext, miResult)) != MI_RESULT_OK))
    {
        /* Something failed. They will call WSManPluginOperationComplete to report it is done/failed though so we will clean up in there */
    }
	return miResult;
}

MI_Boolean IsStreamCompressed(CommonData *commonData)
{
    if (commonData->dataType == CommonData_Type_Shell)
    {
        ShellData *shellData = (ShellData*)commonData;
        return shellData->isCompressed;
    }
    else
    {
        CommandData *commandData = (CommandData*)commonData;
        return commandData->shellData->isCompressed;
    }
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
    PluginRequest *pluginRequest = (PluginRequest*)requestDetails;
    MI_Context *receiveContext = NULL;
    CommandState commandStateInst;
    Shell_Receive *receive = NULL;
    Stream receiveStream;
    DecodeBuffer decodeBuffer, decodedBuffer;
    memset(&decodeBuffer, 0, sizeof(decodeBuffer));
    memset(&decodedBuffer, 0, sizeof(decodedBuffer));

    /* TODO: Which stream if stream is NULL? */

    /* Wait for a Receive request to come in before we post the result back */
    do
    {

    } while (CondLock_Wait((ptrdiff_t)&pluginRequest->miRequestContext,
                           (ptrdiff_t*)&pluginRequest->miRequestContext,
                           0, 
                           CONDLOCK_DEFAULT_SPINCOUNT) == 0);
    
    /* Copy off the receive context. Another Receive should not happen at the same time but this makes
    * sure we are the only one processing it. This will also help to protect us if a Signal comes in
    * to shut things down as we are now responsible for delivering its results.
    */
    receiveContext = pluginRequest->miRequestContext;
    pluginRequest->miRequestContext = NULL;

    /* Construct our instance result objects */
    receive = (Shell_Receive*)pluginRequest->miOperationInstance;
    pluginRequest->miOperationInstance = NULL;

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
    if (pluginRequest->commonData->dataType == CommonData_Type_Command)
    {
        CommandData *commandData = (CommandData*)pluginRequest->commonData;
        CommandState_SetPtr_commandId(&commandStateInst, commandData->commandId);
        Stream_SetPtr_commandId(&receiveStream, commandData->commandId);
    }


    
    /* TODO: Does powershell use binary data rather than string? */
    decodeBuffer.buffer = (MI_Char*) streamResult->binaryData.data;
    decodeBuffer.bufferLength = streamResult->binaryData.dataLength;
    decodeBuffer.bufferUsed = decodeBuffer.bufferLength;

    if (IsStreamCompressed(pluginRequest->commonData))
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
    if (IsStreamCompressed(pluginRequest->commonData))
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
        (Tcscmp(commandState, MI_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell/CommandState/Done")) == 0))
    {
        /* TODO: Mark stream as complete for either shell or command */
        CommandState_SetPtr_state(&commandStateInst,
            MI_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell/CommandState/Done"));

#if 0
        MI_Uint32 i;
        /* find and mark the stream as done in our records for when command is terminated and we need to terminate streams */
        for (i = 0; i != commandData->shellData->outboundStreamNamesCount; i++)
        {
            if (Tcscmp(stream, commandData->outboundStreams[i].streamName))
            {
                commandData->outboundStreams[i].done = MI_TRUE;
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
        CommandState_SetPtr_state(&commandStateInst,
            MI_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell/CommandState/Running"));
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
    PluginRequest *pluginRequest = (PluginRequest*) requestDetails;
    MI_Result miResult;

    /* Question is: which request is this? */
    switch (pluginRequest->requestType)
    {
    case WSMAN_PLUGIN_REQUEST_OPERATION:
        /* This is more of a clean-up notification for either command or shell... unless they did not post the contexts to us in which case we will need to post the result */

        if (pluginRequest->commonData->dataType == CommonData_Type_Shell)
        {
            /* TODO: Shell has completed. We should get no more calls after this */
            /* Remove the shell data object from the shell list */
            /* Delete the shell instance embedded in the shell data */
            /* Delete the shell data */
        }
        else
        {
            /* TODO: This command is complete. No more calls for this command should happen */
            /* Remove the command data pointer from the owning shell */
            /* Delete the command instance embedded in the command data */
            /* Delete the command data */
        }
        break;
    case WSMAN_PLUGIN_REQUEST_RECEIVE:
        /* TODO: This is the termination of the receive. No more will happen for either the command or shell, depending on which is is aimed at */
        /* We may or may not have a pending Receive protocol packet for this depending on if we have already send a command completion message or not */
        if (pluginRequest->miRequestContext)
        {
            /* We have a pending request that needs to be terminated */
            WSManPluginReceiveResult(requestDetails, WSMAN_FLAG_RECEIVE_RESULT_NO_MORE_DATA, NULL, NULL, MI_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell/CommandState/Done"), errorCode);
        }
        break;

   
        /* Send/Receive only need to post back the operation instance with the MIReturn code set */
    case WSMAN_PLUGIN_REQUEST_SEND:
    case WSMAN_PLUGIN_REQUEST_SIGNAL:
    {
        MI_Value miValue;
        MI_Context *miRequestContext = pluginRequest->miRequestContext;
        MI_Instance *miOperationInstance = pluginRequest->miOperationInstance;

        pluginRequest->miRequestContext = NULL;
        pluginRequest->miOperationInstance = NULL;

        /* Methods only have the return code set in the instance so set that and post back. */
        miValue.uint32 = errorCode;
        MI_Instance_SetElement(
            miOperationInstance,
            MI_T("MIReturn"),
            &miValue,
            MI_UINT32,
            0);
        
        miResult = MI_Context_PostInstance(miRequestContext, miOperationInstance);
        MI_Context_PostResult(miRequestContext, miResult);

        MI_Instance_Delete(miOperationInstance);
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
