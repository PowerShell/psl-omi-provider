#ifndef _BufferManipulation_h_
#define _BufferManipulation_h_
#include <MI.h>
#include <base/batch.h>


typedef struct _DecodeBuffer
{
    MI_Char *buffer;
    MI_Uint32 bufferLength;
    MI_Uint32 bufferUsed;
} DecodeBuffer; 

MI_Result Base64DecodeBuffer(DecodeBuffer *fromBuffer, DecodeBuffer *toBuffer);
MI_Result Base64EncodeBuffer(DecodeBuffer *fromBuffer, DecodeBuffer *toBuffer);
MI_Result DecompressBuffer(DecodeBuffer *fromBuffer, DecodeBuffer *toBuffer);
MI_Result CompressBuffer(DecodeBuffer *fromBuffer, DecodeBuffer *toBuffer, MI_Uint32 extraSpaceToAllocate);

MI_Boolean Utf8ToUtf16Le(Batch *batch, const char *from, MI_Char16 **to);
MI_Boolean Utf16LeToUtf8(Batch *batch, const MI_Char16 *from, char **to);


#endif /* _BufferManipulation_h_ */
