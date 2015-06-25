#ifndef _xpress_h_
#define _xpress_h_

typedef unsigned char * PUCHAR;
typedef const unsigned char * PCUCHAR;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
typedef unsigned int ULONG ;
typedef short SHORT;
typedef ULONG* PULONG;
typedef ptrdiff_t ULONG_PTR;
typedef ptrdiff_t LONG_PTR;
typedef void* PVOID;
typedef ULONG NTSTATUS;
typedef void VOID;
typedef int INT;
typedef ULONG LOGICAL;
typedef ULONG *PLOGICAL;

#define UNALIGNED
#define IN
#define OUT
#define NTAPI
#define __assume(x)
//__declspec(noinline)
#define NOINLINE



#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#define STATUS_INVALID_USER_BUFFER       ((NTSTATUS)0xC00000E8L)
#define STATUS_BAD_COMPRESSION_BUFFER    ((NTSTATUS)0xC0000242L)


typedef
VOID
NTAPI
RTL_XPRESS_CALLBACK_FUNCTION (
    IN PVOID Context
    );
typedef RTL_XPRESS_CALLBACK_FUNCTION *PRTL_XPRESS_CALLBACK_FUNCTION;

NTSTATUS
RtlCompressBufferProgress (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace,
    IN PRTL_XPRESS_CALLBACK_FUNCTION Callback,
    IN PVOID CallbackContext,
    IN ULONG ProgressBytes
    );

NTSTATUS
RtlDecompressBufferProgress (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize,
    IN PVOID Workspace,
    IN PRTL_XPRESS_CALLBACK_FUNCTION Callback,
    IN PVOID CallbackContext,
    IN ULONG ProgressBytes
    );

NTSTATUS
RtlCompressWorkSpaceSizeXpressHuff (
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG DecompressBufferWorkSpaceSize
    );

#endif
