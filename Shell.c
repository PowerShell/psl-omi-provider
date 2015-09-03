#include <MI.h>
#include "Shell.h"
#include "ShellAPI.h"
#include "BufferManipulation.h"
#include <pal/strings.h>
#include <pal/format.h>
#include <pal/lock.h>


/* Number of characters reserved for command ID and shell ID -- max number of digits for hex 64-bit number with null terminator */
#define ID_LENGTH 17

#define GOTO_ERROR(result) { miResult = result; goto error; }

/* Indexes into the WSMAN_PLUGIN_REQUEST arrays in CommandData and ShellData */
#define WSMAN_PLUGIN_REQUEST_SHELL 0
#define WSMAN_PLUGIN_REQUEST_COMMAND 0
#define WSMAN_PLUGIN_REQUEST_SEND 1
#define WSMAN_PLUGIN_REQUEST_RECEIVE 2
#define WSMAN_PLUGIN_REQUEST_SIGNAL 3

/* Commands and shells can have multiple streams associated with them. */
typedef struct _StreamData
{
	MI_Char *streamName;
	MI_Boolean done;
} StreamData;

typedef struct _CommonData
{
	/* pluginRequest is at start so we can cast to the CommandData structure */
    /* 0 = command/shell, 1 = send, 2 = receive, 3 = signal */
    WSMAN_PLUGIN_REQUEST pluginRequest[4];

    /* MI_TRUE is shell, MI_FALSE is command */
    enum { Type_Shell, Type_Command } dataType;

} CommonData;


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

    Shell_Command *commandInstance;

    ShellData *shellData;

    MI_Context *commandContext;
    MI_Context *receiveContext;

    void * pluginCommandContext;
};

/* List of active shells. Only one command per shell is supported. */
struct _ShellData
{
	/* pluginRequest is at start so we can cast to the ShellData structure */
    /* 0 = shell, 1 = send, 2 = receive, 3 = signal */
    WSMAN_PLUGIN_REQUEST pluginRequest[4];

    /* This is part of a linked list of shells */
    ShellData *nextShell;

    /* This shells ID */
    MI_Char *shellId;

    /* Only support a single command, so pointer to the command */
    CommandData *command;

    MI_Uint32 outboundStreamNamesCount;
    MI_Char **outboundStreamNames;

    MI_Boolean isCompressed;

    /* Instance that we can use to send back when shells are enumerated */
    Shell *shellInstance;

    MI_Context *requestContext;

    Shell_Self *shell;

    void * pluginShellContext;

};

/* The master shell object that the provider passes back as context for all provider
 * operations. Currently it only needs to point to the list of shells.
 */
struct _Shell_Self
{
    ShellData *shellList;
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
        MI_Context_PostResult(context, MI_RESULT_OK);
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
		miResult = Shell_Post(shellData->shellInstance, context);
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
    ShellData *thisShell = FindShell(self, instanceName->Name.value);

    if (thisShell)
    {
    	Shell_Post(thisShell->shellInstance, context);
    	MI_Context_PostResult(context, MI_RESULT_OK);
    }
    else
    {
    	MI_Context_PostResult(context, MI_RESULT_NOT_FOUND);
    }
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
    Shell_Clone(newInstance, &thisShell->shellInstance);
    Shell_SetPtr_Name(thisShell->shellInstance, thisShell->shellId);

    /* Call out to external plug-in API to continue shell creation.
     * Acceptance of shell is reported through WSManPluginReportContext.
     * If something fails then we will get a failure through WSManPluginOperationComplete
     */
    WSMAN_PLUGIN_SHELL(NULL, &thisShell->pluginRequest[WSMAN_PLUGIN_REQUEST_SHELL], 0, NULL, NULL);
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

        /* Delete the stream list buffer -- they are all allocated out of a single buffer */
        free(thisShell->outboundStreamNames);

        /* Delete the instance representing the shell */
        Shell_Delete(thisShell->shellInstance);

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
    Shell_Command command;
    MI_Result miResult = MI_RESULT_OK;
    ShellData *thisShell;
    MI_Uint32 i;

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
    thisShell->command->receiveContext = context;

    /* Create command instance to send back to client */
    miResult = Shell_Command_Clone(&command, &thisShell->command->commandInstance);
    if (miResult != MI_RESULT_OK)
    {
    	GOTO_ERROR(miResult);
    }

    Shell_Command_SetPtr_CommandId(thisShell->command->commandInstance, thisShell->command->commandId);
    Shell_Command_Set_MIReturn(&command, MI_RESULT_OK);

    WSMAN_PLUGIN_COMMAND(
    		&thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_COMMAND],
			0,
			thisShell->pluginShellContext,
			NULL, NULL);


error:
    if (miResult != MI_RESULT_OK)
    {
        /* Cleanup all the memory */
        Shell_Command_Delete(thisShell->command->commandInstance);
        free(thisShell->command);
        thisShell->command = NULL;

        MI_Context_PostResult(context, miResult);
    }
    else
    {
    	/* We succeeded so we rely on the callback to post the result */
    }
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
	memset(&decodeBuffer, 0, sizeof(decodeBuffer));
	memset(&decodedBuffer, 0, sizeof(decodedBuffer));

	/* Was the shell ID the one we already know about? */
    if (!thisShell)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    /* Check to make sure the command ID is correct */
    if (in->streamData.value->commandId.exists)
    {
        if (!in->streamData.value->commandId.value || !thisShell->command ||
            (Tcscmp(in->streamData.value->commandId.value, thisShell->command->commandId) != 0))
        {
            GOTO_ERROR(MI_RESULT_NOT_FOUND);
        }
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
            WSMAN_PLUGIN_SEND(
                &thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_SEND],
                pluginFlags,
                thisShell->pluginShellContext,
                thisShell->command->pluginCommandContext,
                in->streamData.value->streamName.value,
                &inboundData);
        }
        else
        {
            WSMAN_PLUGIN_SEND(
                &thisShell->pluginRequest[WSMAN_PLUGIN_REQUEST_SEND],
                pluginFlags,
                thisShell->pluginShellContext,
                NULL,
                in->streamData.value->streamName.value,
                &inboundData);
        }
    }


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

    if (!thisShell)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }
    if (!in->commandId.exists)
    {
        GOTO_ERROR(MI_RESULT_NOT_SUPPORTED);
    }
    if (Tcscmp(in->commandId.value, thisShell->command->commandId) != 0)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    if (thisShell->command->receiveContext)
    {
        GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
    }

    thisShell->command->receiveContext = context;

    WSMAN_PLUGIN_RECEIVE(
    		&thisShell->command->common.pluginRequest[WSMAN_PLUGIN_REQUEST_RECEIVE],
			0,
			thisShell->pluginShellContext,
			thisShell->command->pluginCommandContext,
			NULL);

    /* Posting on receive context happens when we receive some data on a _Send() */
    return;

error:
    MI_Context_PostResult(context, miResult);

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


    if (!thisShell)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }
    if (in->commandId.exists)
    {
		if (Tcscmp(in->commandId.value, thisShell->command->commandId) != 0)
		{
			GOTO_ERROR(MI_RESULT_NOT_FOUND);
		}
    }
    else
    {
    	/* For now we will assume this is something like ctrl_c which is not command specific, but targets all commands for which we have one */
    }

    if (thisShell->command->receiveContext)
    {
        Shell_Receive receive;
        Stream stream;
        CommandState commandState;
        MI_Context *receiveContext = thisShell->command->receiveContext;

        thisShell->command->receiveContext = 0;	/* Null out receive. */

        Shell_Receive_Construct(&receive, receiveContext);
        Shell_Receive_Set_MIReturn(&receive, MI_RESULT_OK);

        /* TODO: only work with stream[0] for now */
        if (!thisShell->command->outboundStreams[0].done)
        {
			Stream_Construct(&stream, receiveContext);
			Stream_SetPtr_commandId(&stream, in->commandId.value);
			Stream_SetPtr_streamName(&stream, thisShell->command->outboundStreams[0].streamName);

			Shell_Receive_SetPtr_Stream(&receive, &stream);
        }

        CommandState_Construct(&commandState, receiveContext);
        CommandState_SetPtr_commandId(&commandState, in->commandId.value);

        CommandState_SetPtr_state(&commandState,
                MI_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell/CommandState/Done"));

        Shell_Receive_SetPtr_CommandState(&receive, &commandState);

        miResult = Shell_Receive_Post(&receive, receiveContext);

        Shell_Receive_Destruct(&receive);
        CommandState_Destruct(&commandState);

        /* TODO: only work with stream[0] for now */
        if (!thisShell->command->outboundStreams[0].done)
        {
        	Stream_Destruct(&stream);
        }


        MI_Context_PostResult(receiveContext, miResult);
    }
    {
        Shell_Signal signal;

        Shell_Signal_Construct(&signal, context);
        Shell_Signal_Set_MIReturn(&signal, MI_RESULT_OK);
        miResult = Shell_Signal_Post(&signal, context);
        Shell_Signal_Destruct(&signal);
    }

error:

	/* Delete the command object and all associated memory -- it was chunked into a single malloc */
	free(thisShell->command);
	thisShell->command = NULL;

	MI_Context_PostResult(context, miResult);
}

void MI_CALL Shell_Invoke_Connect(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Connect* in)
{
    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}


MI_Uint32 MI_CALL WSManPluginReportContext(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ void * context
    )
{
	/* For shell or command the plug-in request is first member and
	 * element 0 in array.
	 */
	CommonData *commonData = (CommonData *) requestDetails;
	MI_Uint32 returnCode = 0;

	if (commonData->dataType == Type_Shell)
	{
		ShellData *thisShell = (ShellData*) commonData;
		MI_Result miResult;
		MI_Context *requestContext = thisShell->requestContext;

		thisShell->pluginShellContext = context;

	    /* Post instance to client */
	    miResult = Shell_Post(thisShell->shellInstance, requestContext);

	    /* If we failed to post it then the entire shell needs to be cleaned up */
	    if (miResult != MI_RESULT_OK)
	    {
	    	thisShell->shell->shellList = thisShell->nextShell;
	    	Shell_Delete(thisShell->shellInstance);
	    	free(thisShell);

	        returnCode = (MI_Uint32) miResult;
	    }

	    /* Post result back to client */
	    MI_Context_PostResult(requestContext, miResult);
	}
	else /* command */
	{
		CommandData *thisCommand = (CommandData*) commonData;
		MI_Result miResult;
		MI_Context *requestContext = thisCommand->commandContext;

		thisCommand->pluginCommandContext = context;

	    miResult = Shell_Command_Post(thisCommand->commandInstance, requestContext);
	    if (miResult != MI_RESULT_OK)
	    {
	    	Shell_Command_Delete(thisCommand->commandInstance);
	        free(thisCommand);
	        thisCommand->shellData->command = NULL;
	        miResult = MI_RESULT_SERVER_LIMITS_EXCEEDED;

	        returnCode = (MI_Uint32) miResult;
	    }
	    MI_Context_PostResult(requestContext, miResult);

	}
	/* Store either the shell or command context for future calls into the plug-in */
	return returnCode;
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
    CommonData *commonData = (CommonData *)requestDetails;
    CommandData *commandData = (CommandData*)commonData;
    MI_Context *receiveContext = NULL;
    CommandState commandStateInst;
    Shell_Receive receive;
    Stream receiveStream;
    DecodeBuffer decodeBuffer, decodedBuffer;
    memset(&decodeBuffer, 0, sizeof(decodeBuffer));
    memset(&decodedBuffer, 0, sizeof(decodedBuffer));

    /* TODO: Which stream if stream is NULL? */

    if (commonData->dataType == Type_Shell)
    {
        /* Should not happen as we currently don't support Receive on Shell */
        return (MI_Uint32) MI_RESULT_FAILED;
    }
    /* Wait for a Receive request to come in before we post the result back */
    do
    {

    } while (CondLock_Wait((ptrdiff_t)&commandData->receiveContext, (ptrdiff_t*)&commandData->receiveContext, 0, CONDLOCK_DEFAULT_SPINCOUNT) == 0);

    /* Copy off the receive context. Another Receive should not happen at the same time but this makes
    * sure we are the only one processing it. This will also help to protect us if a Signal comes in
    * to shut things down as we are now responsible for delivering its results.
    */
    receiveContext = commandData->receiveContext;
    commandData->receiveContext = NULL;
    
    /* TODO: Does powershell use binary data rather than string? */
    decodeBuffer.buffer = (MI_Char*) streamResult->binaryData.data;
    decodeBuffer.bufferLength = streamResult->binaryData.dataLength;
    decodeBuffer.bufferUsed = decodeBuffer.bufferLength;

    if (commandData->shellData->isCompressed)
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
    if (commandData->shellData->isCompressed)
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
    CommandState_Construct(&commandStateInst, receiveContext);
    CommandState_SetPtr_commandId(&commandStateInst, commandData->commandId);

    if (commandState &&
        (Tcscmp(commandState, MI_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell/CommandState/Done")) == 0))
    {
        MI_Uint32 i;
        CommandState_SetPtr_state(&commandStateInst,
            MI_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell/CommandState/Done"));

        /* find and mark the stream as done in our records for when command is terminated and we need to terminate streams */
        for (i = 0; i != commandData->shellData->outboundStreamNamesCount; i++)
        {
            if (Tcscmp(stream, commandData->outboundStreams[i].streamName))
            {
                commandData->outboundStreams[i].done = MI_TRUE;
                break;
            }
        }
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
    Stream_Construct(&receiveStream, receiveContext);
    Stream_Set_endOfStream(&receiveStream, streamResult->.value->endOfStream.value);
    Stream_SetPtr_streamName(&receiveStream, in->streamData.value->streamName.value);
    if (in->streamData.value->commandId.exists)
    {
        Stream_SetPtr_commandId(&receiveStream, in->streamData.value->commandId.value);
    }

    /* The result of the Receive contains the command results and a set of streams.
    * We only support one stream at a time for now.
    */
    Shell_Receive_Construct(&receive, receiveContext);
    Shell_Receive_Set_MIReturn(&receive, MI_RESULT_OK);
    Shell_Receive_SetPtr_CommandState(&receive, &commandStateInst);


    /* Add the final string to the stream. This is just a pointer to it and
     * is not getting copied so we need to delete the buffer after we have posted the instance
     * to the receive context.
     */
    Stream_SetPtr_data(&receiveStream, decodedBuffer.buffer);

    /* Add the stream embedded instance to the receive result.  */
    Shell_Receive_SetPtr_Stream(&receive, &receiveStream);

    /* Post the result back to the client. We can delete the base-64 encoded buffer after
    * posting it.
    */
    Shell_Receive_Post(&receive, receiveContext);

    /* Clean up the various result objects */
    Shell_Receive_Destruct(&receive);
    CommandState_Destruct(&commandStateInst);
    Stream_Destruct(&receiveStream);

error:

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
  _In_   void * pluginContext,
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
    return (MI_Uint32)MI_RESULT_FAILED;
}

/* Seems to be called to say the plug-in is actually done */
MI_Uint32 MI_CALL WSManPluginReportCompletion(
  _In_      void * pluginContext,
  _In_      MI_Uint32 flags
  )
{
    return (MI_Uint32)MI_RESULT_FAILED;
}

/* Free up information inside requestDetails if possible */
MI_Uint32 MI_CALL WSManPluginFreeRequestDetails(_In_ WSMAN_PLUGIN_REQUEST *requestDetails)
{
	return MI_RESULT_OK;
}
