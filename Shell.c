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
#include "xpress.h"

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

typedef struct _DecodeBuffer
{
	MI_Char *buffer;
	MI_Uint32 bufferLength;
	MI_Uint32 bufferUsed;
} DecodeBuffer;

static int Shell_Base64Dec_Callback(const void* data, size_t size,
        void* callbackData)
{
	DecodeBuffer *thisBuffer = callbackData;

	if ((thisBuffer->bufferUsed+size) > thisBuffer->bufferLength)
		return -1;

  	memcpy(thisBuffer->buffer+thisBuffer->bufferUsed, data, size);
  	thisBuffer->bufferUsed += size;
    return 0;
}
MI_Result Base64DecodeBuffer(DecodeBuffer *fromBuffer, DecodeBuffer *toBuffer)
{
	toBuffer->bufferLength = fromBuffer->bufferUsed;
	toBuffer->bufferUsed = 0;
	toBuffer->buffer = malloc(toBuffer->bufferLength);

	if (toBuffer->buffer == NULL)
		return MI_RESULT_SERVER_LIMITS_EXCEEDED;

    if (Base64Dec(fromBuffer->buffer,
    		fromBuffer->bufferLength,
            Shell_Base64Dec_Callback, toBuffer) == -1)
    {
    	return MI_RESULT_FAILED;
    }
	return MI_RESULT_OK;
}

static int Shell_Base64Enc_Callback(const char* data, size_t size,
        void* callbackData)
{
	DecodeBuffer *thisBuffer = callbackData;

	if (thisBuffer->bufferUsed+size > thisBuffer->bufferLength)
		return -1;

  	memcpy(thisBuffer->buffer+thisBuffer->bufferUsed, data, size);
	thisBuffer->bufferUsed += size;
    return 0;
}

MI_Result Base64EncodeBuffer(DecodeBuffer *fromBuffer, DecodeBuffer *toBuffer)
{
	toBuffer->bufferLength = ((fromBuffer->bufferUsed * 4) / 3) + sizeof(MI_Char) + (sizeof(MI_T('=')) * 2);
	toBuffer->bufferUsed = 0;
	toBuffer->buffer = malloc(toBuffer->bufferLength);

	if (toBuffer->buffer == NULL)
		return MI_RESULT_SERVER_LIMITS_EXCEEDED;

	if (Base64Enc(fromBuffer->buffer,
    		fromBuffer->bufferUsed,
            Shell_Base64Enc_Callback, toBuffer) == -1)
    {
		free(toBuffer->buffer);
		toBuffer->buffer = NULL;
    	return MI_RESULT_FAILED;
    }
	if ((toBuffer->bufferLength - toBuffer->bufferUsed) < sizeof(MI_Char))
	{
		/* failed to leave enough space on end */
		free(toBuffer->buffer);
		toBuffer->buffer = NULL;
		return MI_RESULT_FAILED;
	}
    return MI_RESULT_OK;
}

/* Compression of buffers splits the data into chunks. The code compressed 64K at a time and
 * prepends the CompressionHeader structure to hold the original uncompressed size and the
 * compressed size.
 * NOTE: The original protocol has a bug in it such that the two sizes are encoded incorrectly.
 *       The sizes in the protocol are 1 less than they actually are so the sizes need to be
 *       adjusted accordingly.
 */
typedef struct _CompressionHeader
{
	USHORT originalSize;
	USHORT compressedSize;
} CompressionHeader;

/* CalculateTotalUncompressedSize
 * This function enumerates the compressed buffer chunks to calculate the total
 * uncompressed size. It is used to allocate a buffer big enough for the full
 * uncompressed buffer.
 * NOTE: The CompressionHeader sizes are adjusted to accomodate the protocol bug.
 */
static ULONG CalculateTotalUncompressedSize( DecodeBuffer *compressedBuffer)
{
	CompressionHeader *header;
	PCUCHAR bufferCursor = (PCUCHAR) compressedBuffer->buffer;
	PCUCHAR endOfBuffer = bufferCursor + compressedBuffer->bufferLength;
	ULONG currentSize = 0;

	while (bufferCursor < endOfBuffer)
	{
		header = (CompressionHeader*) bufferCursor;
		currentSize += (header->originalSize + 1); /* On the wire size is off-by-one */

		/* Move to next block */
		bufferCursor += (header->compressedSize + 1); /* On the wire size is off-by-one */
		bufferCursor += sizeof(CompressionHeader);
	}

	return currentSize;
}

/* DecompressBuffer
 * Decompress the appended compressed chunks into a single buffer. This function
 * allocates the destination buffer and the caller needs to free the buffer.
 * NOTE: This code compensates for the protocol bug where the CompressionHeader values
 *       are encoded incorrectly.
 */
MI_Result DecompressBuffer(DecodeBuffer *fromBuffer, DecodeBuffer *toBuffer)
{
	ULONG wsCompressSize, wsDecompressSize;
	PVOID workspace = NULL;
	PUCHAR fromBufferCursor, fromBufferEnd;
	PUCHAR toBufferCursor;
	NTSTATUS status;
	MI_Result miResult = MI_RESULT_OK;

	memset(toBuffer, 0, sizeof(*toBuffer));

	if (RtlCompressWorkSpaceSizeXpressHuff(&wsCompressSize, &wsDecompressSize) != STATUS_SUCCESS)
	{
		GOTO_ERROR(MI_RESULT_FAILED);
	}

	workspace = malloc(wsDecompressSize);
	if (workspace == NULL)
	{
		GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
	}

	toBuffer->bufferUsed = 0;
	toBuffer->bufferLength = CalculateTotalUncompressedSize(fromBuffer);
	toBuffer->buffer = malloc(toBuffer->bufferLength);
	if (toBuffer->buffer == NULL)
	{
		GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
	}

	toBufferCursor = (PUCHAR) toBuffer->buffer;

	fromBufferCursor = (PUCHAR) fromBuffer->buffer;
	fromBufferEnd = fromBufferCursor + fromBuffer->bufferUsed;

	while (fromBufferCursor < fromBufferEnd)
	{
		ULONG bufferUsed = 0;
		CompressionHeader *compressionHeader = (CompressionHeader*) fromBufferCursor;

		if ((toBuffer->bufferUsed+compressionHeader->originalSize+1) > toBuffer->bufferLength)
		{
			GOTO_ERROR(MI_RESULT_FAILED);
		}

		fromBufferCursor += sizeof(CompressionHeader);

		if (compressionHeader->originalSize == compressionHeader->compressedSize)
		{
			/* When sizes are the same it means that the compression algorithm could not do any compression
			 * so the original buffer was used
			 */
			memcpy(toBufferCursor, fromBufferCursor, compressionHeader->originalSize+1);
			bufferUsed = compressionHeader->originalSize+1;
		}
		else
		{
			status = RtlDecompressBufferProgress (
					toBufferCursor,
					compressionHeader->originalSize+1,	/* Adjusting for incorrect compression header */
					fromBufferCursor,
					compressionHeader->compressedSize+1, /* Adjusting for incorrect compression header */
					&bufferUsed,
					workspace,
					NULL,
					NULL,
					0
				);
			if(status != STATUS_SUCCESS)
			{
				GOTO_ERROR(MI_RESULT_FAILED);
			}
		}

		toBuffer->bufferUsed += bufferUsed;
		toBufferCursor += bufferUsed;
		fromBufferCursor += (compressionHeader->compressedSize + 1); /* Adjusting for incorrect compression header */
	}

error:
	free(workspace);

	if (miResult != MI_RESULT_OK)
	{
		free(toBuffer->buffer);
		toBuffer->buffer = NULL;
	}
	return miResult;
}

/* Maximum concompressed buffer size is 64K */
#define MAX_COMPRESS_BUFFER_BLOCK (64*1024)

static ULONG min(ULONG a, ULONG b)
{
	if (a < b)
		return a;
	else
		return b;
}

/* CompressBuffer
 * Compresses the buffer into chunks, compressing each 64K chunk of data with its own
 * CompressionHeader prepended to each chunk.
 * NOTE: This code compensates for the protocol bug in CompressionHeader
 */
MI_Result CompressBuffer(DecodeBuffer *fromBuffer, DecodeBuffer *toBuffer, ULONG extraSpaceToAllocate)
{
	ULONG wsCompressSize, wsDecompressSize;
	PVOID workspace = NULL;
	PUCHAR fromBufferCursor, fromBufferEnd;
	PUCHAR toBufferCursor;
	ULONG toBufferMaxNumChunks;
	MI_Result miResult = MI_RESULT_OK;

	memset(toBuffer, 0, sizeof(*toBuffer));

	toBufferMaxNumChunks = fromBuffer->bufferUsed/MAX_COMPRESS_BUFFER_BLOCK;
	if (fromBuffer->bufferUsed%MAX_COMPRESS_BUFFER_BLOCK)
		toBufferMaxNumChunks++;

	toBuffer->bufferLength = (sizeof(CompressionHeader) * toBufferMaxNumChunks) + fromBuffer->bufferUsed + extraSpaceToAllocate;
	toBuffer->bufferUsed = 0;
	toBuffer->buffer = malloc(toBuffer->bufferLength);
	if (toBuffer->buffer == NULL)
	{
		GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
	}

	if (RtlCompressWorkSpaceSizeXpressHuff(&wsCompressSize, &wsDecompressSize) != STATUS_SUCCESS)
	{
		GOTO_ERROR(MI_RESULT_FAILED);
	}
	workspace = malloc(wsCompressSize);
	if (workspace == NULL)
	{
		GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
	}

	toBufferCursor = (PUCHAR) toBuffer->buffer;

	fromBufferCursor = (PUCHAR) fromBuffer->buffer;
	fromBufferEnd = fromBufferCursor + fromBuffer->bufferUsed;

	while (fromBufferCursor < fromBufferEnd)
	{
		/* Max compressed chunk size is MAX_COMPRESS_BUFFER_BLOCK or the uncompressed chunk size, whichever is smaller */
		/* We allocated enough space for the uncompressed chunk so if the buffer is not big enough for some reason we
		 * will just use the uncompressed buffer itself for this chunk.
		 */
        ULONG chunkSize = min((ULONG)(fromBufferEnd - fromBufferCursor), MAX_COMPRESS_BUFFER_BLOCK);
        ULONG actualToChunkSize = 0;
        CompressionHeader *compressionHeader = (CompressionHeader*) toBufferCursor;
        NTSTATUS status;

        if ((toBuffer->bufferUsed + chunkSize + sizeof(CompressionHeader)) > toBuffer->bufferLength)
        {
        	GOTO_ERROR(MI_RESULT_FAILED);
        }
        toBufferCursor += sizeof(CompressionHeader);
        toBuffer->bufferUsed += sizeof(CompressionHeader);

        status = RtlCompressBufferProgress(
				fromBufferCursor,
				chunkSize,
				toBufferCursor,
				chunkSize,
				&actualToChunkSize,
				workspace,
				NULL,
				0,
				0
				) ;
        if (status == STATUS_BUFFER_TOO_SMALL)
        {
        	/* Compressed buffer was going to be bigger than the uncompressed buffer so lets just
        	 * use the original.
        	 */
        	memcpy(toBufferCursor, fromBufferCursor, chunkSize);
        	actualToChunkSize = chunkSize;
        }
        else if (status != STATUS_SUCCESS)
        {
        	 GOTO_ERROR(MI_RESULT_FAILED);
        }

        /* NOTE: Size encodings on the wire were originally implemented incorrectly so we need
         * to adjust our encodings of the sizes as well.
         */
        compressionHeader->originalSize = chunkSize - 1;
        compressionHeader->compressedSize = actualToChunkSize - 1;

        toBuffer->bufferUsed += actualToChunkSize;
        toBufferCursor += actualToChunkSize;

        fromBufferCursor += chunkSize;
	}

error:
	if (miResult != MI_RESULT_OK)
	{
		free(toBuffer->buffer);
		toBuffer->buffer = NULL;
		toBuffer->bufferUsed = 0;
		toBuffer->bufferLength = 0;
	}
	free(workspace);

	return miResult;
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
	Stream receiveStream;
	CommandState commandState;
	DecodeBuffer decodeBuffer, decodedBuffer;
	memset(&decodeBuffer, 0, sizeof(decodeBuffer));
	memset(&decodedBuffer, 0, sizeof(decodedBuffer));

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

    Stream_Construct(&receiveStream, receiveContext);
    Stream_Set_endOfStream(&receiveStream, in->streamData.value->endOfStream.value);
    Stream_SetPtr_streamName(&receiveStream, in->streamData.value->streamName.value);
    if (in->streamData.value->commandId.exists)
    {
    	Stream_SetPtr_commandId(&receiveStream, in->streamData.value->commandId.value);
    }

	Shell_Receive_Construct(&receive, receiveContext);
	Shell_Receive_Set_MIReturn(&receive, MI_RESULT_OK);
    Shell_Receive_SetPtr_CommandState(&receive, &commandState);

    if (in->streamData.value->data.exists)
    {
    	decodeBuffer.buffer = (MI_Char*) in->streamData.value->data.value;
    	decodeBuffer.bufferLength = Tcslen(in->streamData.value->data.value) * sizeof(MI_Char);
    	decodeBuffer.bufferUsed = decodeBuffer.bufferLength;

        /* Need to base-64 decode the data. */
        /* TODO: Add length to structure so we don't need to strlen it */
    	/* TODO: This does not support unicode MI_Char strings */
        miResult = Base64DecodeBuffer(&decodeBuffer, &decodedBuffer);
        if (miResult != MI_RESULT_OK)
        {
        	/* decodeBuffer.buffer does not need deleting */
        	GOTO_ERROR(miResult);
        }

        /* decodeBuffer.buffer does not need freeing as it was from in parameter */
        decodeBuffer = decodedBuffer;
        memset(&decodedBuffer, 0, sizeof(decodedBuffer));

        /* Decompress it */
		miResult = DecompressBuffer(&decodeBuffer, &decodedBuffer);
        if (miResult != MI_RESULT_OK)
        {
        	free(decodeBuffer.buffer);
        	GOTO_ERROR(miResult);
        }

        /* Re-compress it */
        free(decodeBuffer.buffer);
        decodeBuffer = decodedBuffer;
        miResult = CompressBuffer(&decodeBuffer, &decodedBuffer, sizeof(MI_Char));
        if (miResult != MI_RESULT_OK)
        {
        	free(decodeBuffer.buffer);
        	GOTO_ERROR(miResult);
        }

        free(decodeBuffer.buffer);
        decodeBuffer = decodedBuffer;

        /* TODO: This does not support unicode MI_Char strings */
        /* NOTE: Base64EncodeBuffer allocates enough space for a NULL terminator */
        miResult = Base64EncodeBuffer(&decodeBuffer, &decodedBuffer);
        if (miResult != MI_RESULT_OK)
        {
        	free(decodeBuffer.buffer);
        	GOTO_ERROR(miResult);
        }

        free(decodeBuffer.buffer);

        /* Set the null terminator on the end of the buffer */
        memset(decodedBuffer.buffer+decodedBuffer.bufferUsed, 0, sizeof(MI_Char));

        Stream_SetPtr_data(&receiveStream, decodedBuffer.buffer);
    }

	Shell_Receive_SetPtr_Stream(&receive, &receiveStream);

    Shell_Receive_Post(&receive, receiveContext);

	Shell_Receive_Destruct(&receive);
	CommandState_Destruct(&commandState);


error:
	free(decodedBuffer.buffer);

	MI_Context_PostResult(receiveContext, miResult);

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

