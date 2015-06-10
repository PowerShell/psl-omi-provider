/* @migen@ */

#include <stdio.h>
#include <errno.h>
#include "Shell.h"
#include "Stream.h"
#include "CommandState.h"
#include <base/base64.h>
#include <pal/strings.h>
#include <pal/thread.h>
#include <pal/lock.h>
#include <omi_error/omierror.h>
#include <signal.h>

#define GOTO_ERROR(result) { miResult = result; goto error; }

PAL_Uint32 THREAD_API Shell_Command_OutputReaderThread(void* param);


typedef struct _StreamData
{
	MI_Char *streamName;
	MI_Boolean done;
    char *streamBuffer;
    size_t bufferSizeTotal;
    size_t bufferUsed;
} StreamData;

typedef struct _CommandData
{
    /* This commands ID. There is only one command per shell,
     * but there may be more than one shell in a process.
     */
    MI_Char *commandId;

    /* Pending receive context to send output to */
    MI_Context *receiveContext;

    /* We only support a single outbound stream */
    MI_Uint32 numberOutboundStreams;
    StreamData *outboundStreams;
} CommandData;

typedef struct _ShellData
{
    /* This is part of a linked list of shells */
    struct _ShellData *nextShell;

    /* This shells ID */
    MI_Char *shellId;

    /* Only support a single command, so pointer to the command */
    CommandData *command;

    MI_Uint32 outboundStreamNamesCount;
    MI_Char **outboundStreamNames;
} ShellData;

struct _Shell_Self
{
    struct _ShellData *shellList;

};

Shell_Self g_shellSelf =
{ 0 };

ShellData * FindShell(struct _Shell_Self *shell, const MI_Char *shellId)
{
    ShellData *thisShell = shell->shellList;

    while (thisShell && (Tcscmp(shellId, thisShell->shellId) != 0))
    {
        thisShell = thisShell->nextShell;
    }

    return thisShell;
}

void MI_CALL Shell_Load(Shell_Self** self, MI_Module_Self* selfModule,
        MI_Context* context)
{
    *self = &g_shellSelf;

    (*self)->shellList = 0;

    MI_Context_PostResult(context, MI_RESULT_OK);
}

void MI_CALL Shell_Unload(Shell_Self* self, MI_Context* context)
{
    /* TODO: Do we have any shells still active? */

    memset(self, 0, sizeof(*self));
    MI_Context_PostResult(context, MI_RESULT_OK);
}

void MI_CALL Shell_EnumerateInstances(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_PropertySet* propertySet, MI_Boolean keysOnly,
        const MI_Filter* filter)
{
    MI_Context_PostResult(context, MI_RESULT_OK);
}

void MI_CALL Shell_GetInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* instanceName, const MI_PropertySet* propertySet)
{
    MI_Context_PostResult(context, MI_RESULT_NOT_FOUND);
}

static MI_Boolean ExtractOutboundStreams(const MI_Char *streams, ShellData *thisShell)
{
	const MI_Char *cursor = streams;
	MI_Uint32 i;

	while (cursor && *cursor)
	{
		thisShell->outboundStreamNamesCount++;
		cursor = Tcschr(cursor, MI_T(' '));
		if (cursor && *cursor)
			cursor++;
	}
	thisShell->outboundStreamNames = malloc(sizeof(MI_Char*) * thisShell->outboundStreamNamesCount);
	if (thisShell->outboundStreamNames == NULL)
		return MI_FALSE;

	cursor = streams;
	for (i = 0; i != thisShell->outboundStreamNamesCount; i++)
	{
		const MI_Char *name = cursor;
		cursor = Tcschr(cursor, MI_T(' '));
		thisShell->outboundStreamNames[i] = malloc(cursor - name + sizeof(MI_Char));
		if (thisShell->outboundStreamNames[i] == NULL)
		{
			/* TODO: Cleanup code */
			thisShell->outboundStreamNamesCount = i;
			goto cleanup;
		}
		memcpy(thisShell->outboundStreamNames[i], name, cursor - name);
		thisShell->outboundStreamNames[i][cursor - name] = MI_T('\0');
	}
	return MI_TRUE;

cleanup:
	for (i = 0; i != thisShell->outboundStreamNamesCount; i++)
	{
		free(thisShell->outboundStreamNames[i]);
	}
	free(thisShell->outboundStreamNames);
	thisShell->outboundStreamNames = NULL;
	thisShell->outboundStreamNamesCount = 0;
	return MI_FALSE;
}

void MI_CALL Shell_CreateInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* newInstance)
{
    ShellData *thisShell = 0;
    Shell shellInstance;
    MI_Result miResult;

    if (!newInstance->OutputStreams.exists)
    {
        MI_Context_PostResult(context, MI_RESULT_INVALID_PARAMETER);
        return;
    }

    /* Create an internal representation of the object */
    thisShell = calloc(1, sizeof(*thisShell));
    if (thisShell == 0)
    {
        MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
        return;
    }
    thisShell->shellId = MI_T("Shell_1");
    thisShell->nextShell = self->shellList;
    self->shellList = thisShell;
    if (!ExtractOutboundStreams(newInstance->OutputStreams.value, thisShell))
    {
    	self->shellList = thisShell->nextShell;
    	free(thisShell);
        MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
    	return;
    }

    /* Create an instance of the shell and send it back */
    Shell_Construct(&shellInstance, context);
    Shell_SetPtr_Name(&shellInstance, thisShell->shellId);

    /* Post instance to client */
    miResult = Shell_Post(&shellInstance, context);

    /* Destruct the instance */
    Shell_Destruct(&shellInstance);

    /* Post result back to client */
    MI_Context_PostResult(context, miResult);
}

void MI_CALL Shell_ModifyInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* modifiedInstance, const MI_PropertySet* propertySet)
{
    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

void MI_CALL Shell_DeleteInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* instanceName)
{
    ShellData *thisShell = self->shellList;
    ShellData **previous = &self->shellList;
    MI_Result miResult = MI_RESULT_NOT_FOUND;

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
        free(thisShell);
        miResult = MI_RESULT_OK;
    }

    MI_Context_PostResult(context, miResult);
}

void MI_CALL Shell_Invoke_Command(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Command* in)
{
    Shell_Command command;
    MI_Result miResult = MI_RESULT_OK;
    OMI_Error *errorObject = NULL;
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

    /* Create internal command structure */
    thisShell->command = calloc(1, sizeof(*thisShell->command));
    if (!thisShell->command)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    thisShell->command->commandId = MI_T("Command_1");

    /* Set up the stream objects for the command */
    thisShell->command->outboundStreams = calloc(thisShell->outboundStreamNamesCount, sizeof(*thisShell->command->outboundStreams));
    if (thisShell->command->outboundStreams == NULL)
    {
        free(thisShell->command);
        thisShell->command = 0;
    	GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    for (i = 0; i != thisShell->outboundStreamNamesCount; i++)
    {
    	thisShell->command->outboundStreams[i].streamName = thisShell->outboundStreamNames[i];
    }

    /* Create command instance to send back to client */
    Shell_Command_Construct(&command, context);
    Shell_Command_SetPtr_CommandId(&command, thisShell->command->commandId);
    Shell_Command_Set_MIReturn(&command, MI_RESULT_OK);
    miResult = Shell_Command_Post(&command, context);
    if (miResult != MI_RESULT_OK)
    {
        /* TODO: Do we need to somehow kill the process? */
        free(thisShell->command->outboundStreams);
        free(thisShell->command);
        thisShell->command = NULL;
        miResult = MI_RESULT_SERVER_LIMITS_EXCEEDED;

        /* Need to fall through to destruct the instance */
    }
    Shell_Command_Destruct(&command);

error:
    if (errorObject)
    {
        MI_Context_PostCimError(context, &errorObject->__instance);
        MI_Instance_Delete(&errorObject->__instance);
    }
    else
    {
        MI_Context_PostResult(context, miResult);
    }
}

struct DecodeBuffer
{
	MI_Char *originalBuffer;
	MI_Uint32 originalBufferLength;
	MI_Char *newBuffer;
	MI_Uint32 newBufferLength;
	char *newBufferCursor;
	MI_Uint32 newBufferCursorLength;
};
static int Shell_Base64Dec_Callback(const void* data, size_t size,
        void* callbackData)
{
	struct DecodeBuffer *thisBuffer = callbackData;


	if ((thisBuffer->newBufferCursorLength+size) > thisBuffer->newBufferLength)
		return -1;

  	memcpy(thisBuffer->newBufferCursor, data, size);
	thisBuffer->newBufferCursor += size;
	thisBuffer->newBufferCursorLength += size;
    return 0;
}
static int Shell_Base64Enc_Callback(const char* data, size_t size,
        void* callbackData)
{
	struct DecodeBuffer *thisBuffer = callbackData;


	if (thisBuffer->newBufferCursorLength+size > thisBuffer->newBufferLength)
		return -1;

  	memcpy(thisBuffer->newBufferCursor, data, size);
	thisBuffer->newBufferCursor += size;
	thisBuffer->newBufferCursorLength += size;
    return 0;
}

void MI_CALL Shell_Invoke_Send(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Send* in)
{
    MI_Result miResult = MI_RESULT_OK;
    ShellData *thisShell = FindShell(self, instanceName->Name.value);
    MI_Context *receiveContext;
	Shell_Receive receive;
	CommandState commandState;
	struct DecodeBuffer decodeBuffer, encodeBuffer;
	memset(&decodeBuffer, 0, sizeof(decodeBuffer));
	memset(&encodeBuffer, 0, sizeof(encodeBuffer));

    if (!thisShell)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }
    if (!in->streamData.value->commandId.exists)
    {
        GOTO_ERROR(MI_RESULT_NOT_SUPPORTED);
    }
    if (Tcscmp(in->streamData.value->commandId.value,
            thisShell->command->commandId) != 0)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    /* Wait for a Receive request to come in before we post the result back */
    do
    {

    } while (CondLock_Wait((ptrdiff_t)&thisShell->command->receiveContext, (ptrdiff_t*)&thisShell->command->receiveContext, 0, CONDLOCK_DEFAULT_SPINCOUNT) == 0);

    receiveContext = thisShell->command->receiveContext;
    thisShell->command->receiveContext = NULL;

	CommandState_Construct(&commandState, receiveContext);
    CommandState_SetPtr_commandId(&commandState, in->streamData.value->commandId.value);

    if (in->streamData.value->endOfStream.value)
    {
    	MI_Uint32 i;
		CommandState_SetPtr_state(&commandState,
				MI_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell/CommandState/Done"));

		/* find and mark the stream as done in our records for when command is terminated and we need to terminate streams */
		for (i = 0; i != thisShell->outboundStreamNamesCount; i++)
		{
			if (Tcscmp(in->streamData.value->streamName.value, thisShell->command->outboundStreams[i].streamName))
			{
				thisShell->command->outboundStreams[i].done = MI_TRUE;
				break;
			}
		}
    }
    else
    {
        CommandState_SetPtr_state(&commandState,
                MI_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell/CommandState/Running"));
    }

	Shell_Receive_Construct(&receive, receiveContext);
	Shell_Receive_Set_MIReturn(&receive, MI_RESULT_OK);
    Shell_Receive_SetPtr_CommandState(&receive, &commandState);


    if (in->streamData.value->data.exists)
    {

    	decodeBuffer.originalBuffer = in->streamData.value->data.value;
    	decodeBuffer.originalBufferLength = Tcslen(in->streamData.value->data.value) * sizeof(MI_Char);
    	decodeBuffer.newBuffer = malloc(decodeBuffer.originalBufferLength);
    	decodeBuffer.newBufferCursorLength = 0;
    	decodeBuffer.newBufferCursor = (char *) decodeBuffer.newBuffer;
    	decodeBuffer.newBufferLength = decodeBuffer.originalBufferLength;

        /* Need to base-64 decode the data. We can decode in-place as decoded length is smaller  */
        /* TODO: Add length to structure so we don't need to strlen it */
        if (Base64Dec(decodeBuffer.originalBuffer,
        		decodeBuffer.originalBufferLength,
                Shell_Base64Dec_Callback, &decodeBuffer) == -1)
        {
        	printf("FAILED: Base-64 decode failed\n");
        }
        decodeBuffer.newBufferCursor[0] = MI_T('\0');

        encodeBuffer = decodeBuffer;
        encodeBuffer.originalBuffer = decodeBuffer.newBuffer;
        encodeBuffer.originalBufferLength = decodeBuffer.newBufferCursorLength;
        encodeBuffer.newBuffer = malloc(decodeBuffer.originalBufferLength);
        encodeBuffer.newBufferLength = decodeBuffer.originalBufferLength;
        encodeBuffer.newBufferCursorLength = 0;
        encodeBuffer.newBufferCursor = (char *) encodeBuffer.newBuffer;

        /* Re-encode it into the same buffer as it will be the correct length still */
        if (Base64Enc(encodeBuffer.originalBuffer,
        		encodeBuffer.originalBufferLength,
                Shell_Base64Enc_Callback, &encodeBuffer) == -1)
        {
        	printf("FAILED: Base-64 Encode failed\n");
        }

        encodeBuffer.newBufferCursor[0] = MI_T('\0');

        if (decodeBuffer.originalBufferLength != encodeBuffer.newBufferCursorLength)
        {
        	printf("FAILED: Before/after buffer lengths different\n");
        }
        else if (memcmp(decodeBuffer.originalBuffer, encodeBuffer.newBuffer, decodeBuffer.originalBufferLength))
        {
        	printf("FAILED: Before/after buffers different\n");
        }
    }
	Shell_Receive_SetPtr_Stream(&receive, in->streamData.value);


    Shell_Receive_Post(&receive, receiveContext);

	Shell_Receive_Destruct(&receive);
	CommandState_Destruct(&commandState);
	free(decodeBuffer.newBuffer);
	free(encodeBuffer.newBuffer);

	MI_Context_PostResult(receiveContext, MI_RESULT_OK);

    {
        Shell_Send send;
        Shell_Send_Construct(&send, context);
        Shell_Send_Set_MIReturn(&send, MI_RESULT_OK);
        Shell_Send_Post(&send, context);
        Shell_Send_Destruct(&send);
    }

error:
	MI_Context_PostResult(context, miResult);
}





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
    CondLock_Broadcast((ptrdiff_t) &thisShell->command->receiveContext);

    /* Posting to the context happens when output is received from the process on the thread */
    return;

error:
    MI_Context_PostResult(context, miResult);

}

void MI_CALL Shell_Invoke_Signal(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Signal* in)
{
    MI_Result miResult = MI_RESULT_OK;
    ShellData *thisShell = FindShell(self, instanceName->Name.value);

    printf("Signal request\n");

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

        printf("Sending signal response\n");

        Shell_Signal_Construct(&signal, context);
        Shell_Signal_Set_MIReturn(&signal, MI_RESULT_OK);
        miResult = Shell_Signal_Post(&signal, context);
        Shell_Signal_Destruct(&signal);
    }

error:
	MI_Context_PostResult(context, miResult);
}

void MI_CALL Shell_Invoke_Connect(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Connect* in)
{
    printf("connect\n");
    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

