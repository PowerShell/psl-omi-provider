/*
**==============================================================================
**
** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
** for license information.
**
**==============================================================================
*/

#include <MI.h>
#include <iconv.h>
#include "xpress.h"
#include <base/base64.h>
#include "BufferManipulation.h"

#define GOTO_ERROR(result) { miResult = result; goto error; }

static MI_Boolean ConvertString(Batch *batch,
                         char *fromFormat,
                         char *toFormat,
                         const char *from,
                         size_t fromBuffLen,
                         char **to,
                         size_t toBuffLen)
{
    char *toBufferCurrent;
    char *fromBuffer = (char*)from;
    size_t fromBytesLeft = fromBuffLen;
    size_t toBytesLeft = toBuffLen;
    size_t iconv_return;
    iconv_t iconvData;
    MI_Boolean returnValue = MI_FALSE;

    iconvData = iconv_open(toFormat, fromFormat);
    if (iconvData == (iconv_t)-1)
    {
        printf("Failed to create iconv, %d\n", errno);
        goto cleanup;
    }

    *to = Batch_Get(batch, toBytesLeft);
    if (*to == NULL)
        goto cleanup;
    toBufferCurrent = *to;

    iconv_return = iconv(iconvData, &fromBuffer, &fromBytesLeft, &toBufferCurrent, &toBytesLeft);
    if (iconv_return == (size_t)-1)
    {
        printf("Failed to convert string with iconv, %d\n", errno);
        goto cleanup;
    }

    returnValue = MI_TRUE;

cleanup:
    if (iconvData != (iconv_t)-1)
    {
        iconv_close(iconvData);
    }

    return returnValue;
}

MI_Boolean Utf8ToUtf16Le(Batch *batch, const char *from, MI_Char16 **to)
{
    size_t fromBuffLen = strlen(from)+1;
    return ConvertString(batch, "UTF-8", "UTF-16LE", from, fromBuffLen, (char**) to, fromBuffLen*2);
}

size_t Utf16LeStrLenBytes(const MI_Char16* str)
{
    size_t len = 0;

    while (str[len] != 0 )
    {
        len++;
    }

    return (len+1)*2; /* Return null terminator length as well */
}
MI_Boolean Utf16LeToUtf8(Batch *batch, const MI_Char16 *from, char **to)
{
    size_t fromBuffLen = Utf16LeStrLenBytes(from);
    return ConvertString(batch, "UTF-16LE", "UTF-8", (char*) from, fromBuffLen, to, fromBuffLen/2);
}

static int Shell_Base64Dec_Callback(const void* data, size_t size,
    void* callbackData)
{
    DecodeBuffer *thisBuffer = callbackData;

    if ((thisBuffer->bufferUsed + size) > thisBuffer->bufferLength)
        return -1;

    memcpy(thisBuffer->buffer + thisBuffer->bufferUsed, data, size);
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
        free(toBuffer->buffer);
        toBuffer->buffer = NULL;
        return MI_RESULT_FAILED;
    }
    return MI_RESULT_OK;
}

static int Shell_Base64Enc_Callback(const char* data, size_t size,
    void* callbackData)
{
    DecodeBuffer *thisBuffer = callbackData;

    if (thisBuffer->bufferUsed + size > thisBuffer->bufferLength)
        return -1;

    memcpy(thisBuffer->buffer + thisBuffer->bufferUsed, data, size);
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
static MI_Uint32 CalculateTotalUncompressedSize(DecodeBuffer *compressedBuffer)
{
    CompressionHeader *header;
    const MI_Uint8* bufferCursor = (const MI_Uint8*)compressedBuffer->buffer;
    const MI_Uint8* endOfBuffer = bufferCursor + compressedBuffer->bufferLength;
    MI_Uint32 currentSize = 0;

    while (bufferCursor < endOfBuffer)
    {
        header = (CompressionHeader*)bufferCursor;
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
    MI_Uint32 wsCompressSize, wsDecompressSize;
    void * workspace = NULL;
    MI_Uint8* fromBufferCursor;
    MI_Uint8* fromBufferEnd;
    MI_Uint8* toBufferCursor;
    MI_Uint32 status;
    MI_Result miResult = MI_RESULT_OK;

    memset(toBuffer, 0, sizeof(*toBuffer));


    /* Decompression code needs a working buffer. We really need to cache this buffer
    * so we don't need to keep reallocating and freeing it. We only need one for each
    * of Send/Receive */
    if (CompressWorkSpaceSizeXpressHuff(&wsCompressSize, &wsDecompressSize) != STATUS_SUCCESS)
    {
        GOTO_ERROR(MI_RESULT_FAILED);
    }

    workspace = malloc(wsDecompressSize);
    if (workspace == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Allocate the result buffer for decompression */
    toBuffer->bufferUsed = 0;
    toBuffer->bufferLength = CalculateTotalUncompressedSize(fromBuffer);
    toBuffer->buffer = malloc(toBuffer->bufferLength);
    if (toBuffer->buffer == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    toBufferCursor = (MI_Uint8*)toBuffer->buffer;

    fromBufferCursor = (MI_Uint8*)fromBuffer->buffer;
    fromBufferEnd = fromBufferCursor + fromBuffer->bufferUsed;

    /* We decompress one chunk of data at a time */
    while (fromBufferCursor < fromBufferEnd)
    {
        MI_Uint32 bufferUsed = 0;
        CompressionHeader *compressionHeader = (CompressionHeader*)fromBufferCursor;

        /* Shouldn't fail but to be same make sure we have enough buffer */
        if ((toBuffer->bufferUsed + compressionHeader->originalSize + 1) > toBuffer->bufferLength)
        {
            GOTO_ERROR(MI_RESULT_FAILED);
        }

        fromBufferCursor += sizeof(CompressionHeader);

        if (compressionHeader->originalSize == compressionHeader->compressedSize)
        {
            /* When sizes are the same it means that the compression algorithm could not do any compression
            * so the original buffer was used
            */
            memcpy(toBufferCursor, fromBufferCursor, compressionHeader->originalSize + 1);
            bufferUsed = compressionHeader->originalSize + 1;
        }
        else
        {
            /* Need to actually decompress now */
            status = DecompressBufferProgress(
                toBufferCursor,
                compressionHeader->originalSize + 1,    /* Adjusting for incorrect compression header */
                fromBufferCursor,
                compressionHeader->compressedSize + 1, /* Adjusting for incorrect compression header */
                &bufferUsed,
                workspace,
                NULL,
                NULL,
                0
                );
            if (status != STATUS_SUCCESS)
            {
                GOTO_ERROR(MI_RESULT_FAILED);
            }
        }

        /* Update lengths and cursors ready for next iteration */
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

static size_t min(size_t a, size_t b)
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
MI_Result CompressBuffer(DecodeBuffer *fromBuffer, DecodeBuffer *toBuffer, MI_Uint32 extraSpaceToAllocate)
{
    MI_Uint32 wsCompressSize, wsDecompressSize;
    void * workspace = NULL;
    MI_Uint8* fromBufferCursor;
    MI_Uint8* fromBufferEnd;
    MI_Uint8* toBufferCursor;
    MI_Uint32 toBufferMaxNumChunks;
    MI_Result miResult = MI_RESULT_OK;

    memset(toBuffer, 0, sizeof(*toBuffer));

    toBufferMaxNumChunks = fromBuffer->bufferUsed / MAX_COMPRESS_BUFFER_BLOCK;
    if (fromBuffer->bufferUsed%MAX_COMPRESS_BUFFER_BLOCK)
        toBufferMaxNumChunks++;

    /* We don't know how small it will be but it may end up being the same size, but chunked, so we need to work out how
    * many chunks there are so we can account for the size of each chunks header.
    */
    toBuffer->bufferLength = (sizeof(CompressionHeader) * toBufferMaxNumChunks) + fromBuffer->bufferUsed + extraSpaceToAllocate;
    toBuffer->bufferUsed = 0;
    toBuffer->buffer = malloc(toBuffer->bufferLength);
    if (toBuffer->buffer == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Get the compression workspace size and allocate it. We really need to cache this */
    if (CompressWorkSpaceSizeXpressHuff(&wsCompressSize, &wsDecompressSize) != STATUS_SUCCESS)
    {
        GOTO_ERROR(MI_RESULT_FAILED);
    }
    workspace = malloc(wsCompressSize);
    if (workspace == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    toBufferCursor = (MI_Uint8*)toBuffer->buffer;

    fromBufferCursor = (MI_Uint8*)fromBuffer->buffer;
    fromBufferEnd = fromBufferCursor + fromBuffer->bufferUsed;

    while (fromBufferCursor < fromBufferEnd)
    {
        /* Max compressed chunk size is MAX_COMPRESS_BUFFER_BLOCK or the uncompressed chunk size, whichever is smaller */
        /* We allocated enough space for the uncompressed chunk so if the buffer is not big enough for some reason we
        * will just use the uncompressed buffer itself for this chunk.
        */
        size_t chunkSize = min((size_t)(fromBufferEnd - fromBufferCursor), MAX_COMPRESS_BUFFER_BLOCK);
        MI_Uint32 actualToChunkSize = 0;
        CompressionHeader *compressionHeader = (CompressionHeader*)toBufferCursor;
        MI_Uint32 status;

        if ((toBuffer->bufferUsed + chunkSize + sizeof(CompressionHeader)) > toBuffer->bufferLength)
        {
            GOTO_ERROR(MI_RESULT_FAILED);
        }
        toBufferCursor += sizeof(CompressionHeader);
        toBuffer->bufferUsed += sizeof(CompressionHeader);

        status = CompressBufferProgress(
            fromBufferCursor,
            chunkSize,
            toBufferCursor,
            chunkSize,
            &actualToChunkSize,
            workspace,
            NULL,
            0,
            0
            );
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
