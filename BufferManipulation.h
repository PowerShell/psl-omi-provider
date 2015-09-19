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

