/*
**==============================================================================
**
** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
** for license information.
**
**==============================================================================
*/

#ifndef _xpress_h_
#define _xpress_h_


#include <MI.h>
typedef unsigned short USHORT;
typedef short SHORT;
typedef ptrdiff_t ULONG_PTR;
typedef ptrdiff_t LONG_PTR;
typedef int INT;
typedef MI_Uint32 LOGICAL;
typedef MI_Uint32 *PLOGICAL;
typedef MI_Sint32 BOOL;

#define UNALIGNED
#define __assume(x)



#define STATUS_SUCCESS  ((MI_Uint32)0x00000000L)
#define STATUS_BUFFER_TOO_SMALL          ((MI_Uint32)0xC0000023L)
#define STATUS_INVALID_USER_BUFFER       ((MI_Uint32)0xC00000E8L)
#define STATUS_BAD_COMPRESSION_BUFFER    ((MI_Uint32)0xC0000242L)


typedef
void
MI_CALL
XPRESS_CALLBACK_FUNCTION (
    _In_ void * Context
    );
typedef XPRESS_CALLBACK_FUNCTION *PXPRESS_CALLBACK_FUNCTION;

MI_Uint32
CompressBufferProgress (
    _In_ MI_Uint8 * UncompressedBuffer,
    _In_ MI_Uint32 UncompressedBufferSize,
    _Out_ MI_Uint8 * CompressedBuffer,
    _In_ MI_Uint32 CompressedBufferSize,
    _Out_ MI_Uint32* FinalCompressedSize,
    _In_ void * WorkSpace,
    _In_ PXPRESS_CALLBACK_FUNCTION Callback,
    _In_ void * CallbackContext,
    _In_ MI_Uint32 ProgressBytes
    );

MI_Uint32
DecompressBufferProgress (
    _Out_ MI_Uint8 * UncompressedBuffer,
    _In_ MI_Uint32 UncompressedBufferSize,
    _In_ MI_Uint8 * CompressedBuffer,
    _In_ MI_Uint32 CompressedBufferSize,
    _Out_ MI_Uint32* FinalUncompressedSize,
    _In_ void * Workspace,
    _In_ PXPRESS_CALLBACK_FUNCTION Callback,
    _In_ void * CallbackContext,
    _In_ MI_Uint32 ProgressBytes
    );

MI_Uint32
CompressWorkSpaceSizeXpressHuff (
    _Out_ MI_Uint32* CompressBufferWorkSpaceSize,
    _Out_ MI_Uint32* DecompressBufferWorkSpaceSize
    );

#endif
