/*++

Copyright (c) 2010  Microsoft Corporation

Module Name:

    xpress.c

Abstract:

    This module implements Xpress compression and decompression.

Author:

    Ben Mickle     [benmick]    July 2010

Revision History:

--*/


//#include "ntrtlp.h"

//
// Optimize for speed, not size.
//

//#pragma optimize("t", on)
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "xpress.h"



static __inline unsigned char
_rotl8(unsigned char _Value, unsigned char _Shift) {
  _Shift &= 0x7;
  return _Shift ? (_Value << _Shift) | (_Value >> (8 - _Shift)) : _Value;
}
static __inline unsigned char
_rotr8(unsigned char _Value, unsigned char _Shift) {
  _Shift &= 0x7;
  return _Shift ? (_Value >> _Shift) | (_Value << (8 - _Shift)) : _Value;
}

#define ALIGN_DOWN_POINTER_BY(address, alignment) \
    ((PVOID)((ULONG_PTR)(address) & ~((ULONG_PTR)alignment - 1)))

#define ALIGN_UP_POINTER_BY(address, alignment) \
    (ALIGN_DOWN_POINTER_BY(((ULONG_PTR)(address) + alignment - 1), alignment))

#define ALIGN_UP_POINTER(address, type) \
    ALIGN_UP_POINTER_BY(address, sizeof(type))

static PUCHAR min(PUCHAR a, PUCHAR b)
{
	if (a < b)
		return a;
	else
		return b;
}

#define TRUE 1
#define FALSE 0

typedef
VOID
NTAPI
RTL_XPRESS_CALLBACK_FUNCTION (
    IN PVOID Context
    );
typedef RTL_XPRESS_CALLBACK_FUNCTION *PRTL_XPRESS_CALLBACK_FUNCTION;

const USHORT XpressHashFunction[3][256] = {
    {5732,  14471,  24297,  25128,  11502,  22712,  22856,  21969,  16029,  23951,  1785,  13794,  3705,  1145,  16537,  7129,  26156,  31870,  23418,  20567,  29626,  218,  7372,  1940,  17184,  18323,  10119,  8604,  23621,  13384,  29712,  9146,  20553,  27100,  5850,  23503,  23176,  28142,  4676,  27280,  31756,  8706,  15559,  27626,  18819,  3281,  10692,  4048,  20259,  10493,  27969,  7260,  24815,  17549,  18994,  16693,  31156,  16887,  8459,  31718,  32718,  12366,  7181,  15493,  24345,  26476,  7355,  14175,  13292,  4822,  22793,  215,  24723,  706,  21435,  19075,  25807,  32292,  7815,  29338,  10789,  9301,  21695,  25084,  25689,  31272,  17247,  14211,  9189,  26756,  18524,  14732,  6188,  8560,  13667,  26189,  32660,  4089,  3976,  23040,  29476,  20495,  7315,  5110,  28586,  17798,  29904,  344,  22387,  27111,  9782,  6030,  18509,  7423,  9197,  31788,  25654,  20364,  18354,  5971,  10938,  366,  15563,  18371,  10683,  3657,  18511,  15438,  5520,  422,  13442,  29830,  27837,  15463,  14806,  12921,  3883,  22276,  16387,  31729,  21655,  19236,  20470,  20492,  11751,  24686,  25844,  2840,  9189,  9869,  19238,  24848,  15480,  25809,  20681,  11470,  25986,  9409,  24755,  16524,  24107,  21258,  13853,  28403,  6006,  21415,  23280,  2570,  12055,  20113,  5235,  3562,  7055,  733,  30365,  9470,  24356,  9178,  21362,  28496,  11136,  3731,  3047,  16021,  11840,  22838,  907,  18609,  4721,  18943,  14439,  26839,  2422,  15312,  11641,  25979,  199,  31769,  5234,  23588,  23401,  15374,  1558,  14750,  24149,  31127,  13862,  26020,  31010,  1888,  11434,  1688,  5562,  9959,  2280,  8742,  1443,  25709,  10904,  24657,  23884,  6380,  4008,  20069,  28033,  6303,  19272,  30233,  17279,  17817,  24129,  28759,  935,  6698,  16287,  32578,  14420,  17587,  18645,  17233,  8481,  15766,  3346,  4080,  25415,  11667,  15445,  3149,  17290,  14358,  28128,  29424,  17465,  20225,  8897,  19481},
    {89,  24313,  14591,  8306,  22828,  18884,  7990,  26457,  24877,  30705,  24165,  7973,  16319,  9286,  22677,  30021,  7901,  7880,  18035,  4008,  2401,  29585,  21569,  12793,  23315,  9286,  10852,  26539,  13186,  26156,  26964,  21604,  8305,  15916,  17767,  15170,  26565,  26259,  11693,  31316,  28980,  22665,  496,  16463,  12261,  5111,  1943,  1660,  23930,  13257,  4757,  31563,  15469,  18763,  17906,  21517,  29723,  18324,  20062,  23554,  9039,  24669,  31775,  16771,  17119,  31060,  2587,  8261,  22033,  17027,  5593,  1729,  214,  2914,  21849,  18432,  7429,  21490,  26117,  29863,  22371,  18408,  792,  19364,  21668,  1809,  21594,  4948,  20456,  2019,  29290,  22887,  29078,  22335,  9187,  5096,  17146,  1432,  19366,  22510,  7422,  25502,  13009,  11542,  7875,  2647,  7784,  12205,  12243,  19397,  15740,  5364,  4113,  26061,  25497,  7856,  1925,  32327,  12210,  6254,  27122,  27016,  20121,  12018,  4980,  9272,  5017,  10708,  31072,  25162,  25851,  13203,  15909,  30345,  26602,  19838,  12429,  4695,  2383,  17491,  524,  31579,  1318,  12371,  19282,  19629,  1772,  22021,  12775,  25516,  2257,  26301,  10519,  27741,  5552,  27266,  13548,  28363,  18524,  31245,  5982,  17294,  21585,  13591,  31302,  31804,  12702,  15533,  29640,  23889,  24312,  16503,  15054,  22097,  17733,  4557,  22693,  3477,  16700,  18113,  11692,  10926,  2215,  9617,  24248,  3956,  22701,  24952,  938,  13889,  4191,  24275,  9101,  15744,  19304,  12082,  6459,  17626,  32298,  2736,  8529,  28611,  15671,  3892,  26773,  25900,  6541,  24135,  20603,  24870,  27926,  4019,  28502,  28252,  10220,  5251,  25639,  26053,  25351,  9722,  3020,  4086,  29133,  25585,  23781,  19564,  29020,  23744,  1752,  30531,  24484,  30451,  25913,  10908,  15852,  19700,  14122,  26590,  17988,  5299,  23511,  22145,  26960,  9847,  5119,  18466,  6431,  3592,  6992,  7398,  9792,  24368,  19780,  27824,  16766,  770},
    {29141,  2944,  21483,  667,  28990,  23448,  12644,  7839,  21929,  19747,  16616,  17046,  19188,  32762,  25138,  25039,  19337,  724,  29934,  4914,  22687,  841,  14193,  22961,  1775,  6902,  23188,  19240,  7069,  25600,  15642,  4994,  21651,  3594,  27731,  19933,  11672,  20837,  21867,  2547,  30691,  5021,  4084,  3381,  20986,  2656,  7110,  13821,  7795,  758,  20780,  20822,  32649,  9811,  2267,  25889,  11350,  27423,  2944,  7104,  22471,  31485,  31150,  9359,  30674,  13639,  31985,  20817,  11744,  16516,  11270,  24524,  3193,  18291,  5290,  7973,  25154,  32008,  17754,  3315,  27005,  21741,  15695,  20415,  8565,  4083,  23560,  24858,  24228,  13255,  14780,  14373,  22361,  20804,  2970,  16847,  8003,  25347,  6633,  29140,  25152,  16751,  10005,  8413,  31873,  12712,  28180,  23299,  16433,  3658,  7784,  28886,  19894,  18771,  675,  588,  901,  24092,  1755,  30519,  11912,  15045,  15684,  9183,  10056,  16848,  16248,  32429,  2555,  11360,  11926,  32162,  19499,  10997,  20341,  5905,  16620,  32124,  27807,  19460,  24198,  905,  4976,  14495,  17752,  15076,  31994,  11620,  27478,  16025,  31463,  25965,  28887,  18086,  3806,  11346,  6701,  27480,  30042,  61,  1846,  16527,  9096,  5811,  3284,  1002,  21170,  16860,  21152,  4570,  10196,  32752,  9201,  22647,  16755,  32259,  29729,  23205,  19906,  20825,  31181,  3237,  931,  25156,  20188,  16427,  14394,  18993,  7857,  25179,  26064,  1679,  23786,  32761,  10299,  1891,  14039,  1035,  19354,  6436,  15366,  14679,  26868,  19947,  4862,  19105,  7407,  13039,  4013,  22970,  16180,  14412,  3405,  4984,  26696,  7035,  5361,  11923,  20784,  23477,  9498,  8836,  25922,  32629,  27125,  30994,  18141,  21981,  27383,  23834,  24366,  10855,  6149,  22048,  11990,  13549,  4315,  3591,  1901,  21868,  23189,  25251,  28174,  6620,  11566,  31561,  5909,  10506,  5137,  8212,  20000,  14345,  17393,  7349,  17202,  15562}};

const UCHAR XpressHighBitIndexTable[256] = {0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

#define FIRST_HASH_COEFF    4
#define FIRST_HASH_COEFF2   1

#define HASH_TABLE_SIZE     (255 * FIRST_HASH_COEFF * 2 + 255 * FIRST_HASH_COEFF2 * 2 + 255 + 1)

#define HASH_COEFF_HIGH     8
#define HASH_COEFF_LOW      2

#define HASH_TABLE_EX_SIZE  ((255 + 255) * HASH_COEFF_HIGH + 255 * HASH_COEFF_LOW + 255 + 1)

typedef struct _XPRESS_LZ_WORKSPACE {
    PUCHAR HashTable[HASH_TABLE_SIZE];
    PUCHAR HashTableEx[HASH_TABLE_EX_SIZE];
} XPRESS_LZ_WORKSPACE, *PXPRESS_LZ_WORKSPACE;

//
// The rest of these #defines and structs are for the Huffman encoder/decoder.
//

#define HUFFMAN_BLOCK_SIZE_LOG       16
#define HUFFMAN_BLOCK_SIZE           (1 << HUFFMAN_BLOCK_SIZE_LOG)

#define LEN_MULT                     16

#define HUFF_WINDOW_SIZE             (1 << 16)

#define HUFFMAN_ALPHABET_SIZE        512
#define MAX_ENCODING_LENGTH          15

#define HUFFMAN_DECODE_LENGTH        10

typedef struct _HUFFMAN_NODE {
    ULONG_PTR Frequency;
    union {
        struct {
            ULONG_PTR BitLength;
        };
        struct {
            ULONG_PTR NullPtr;
            ULONG_PTR Symbol;
        };
        PVOID Children[2];
    };
} HUFFMAN_NODE, *PHUFFMAN_NODE;

typedef struct _HUFFMAN_STACK_ENTRY {
    HUFFMAN_NODE* Node;
    ULONG_PTR BitLength;
} HUFFMAN_STACK_ENTRY, *PHUFFMAN_STACK_ENTRY;

typedef struct _HUFFMAN_ENCODING {
    USHORT BitLength;
    USHORT Code;
} HUFFMAN_ENCODING, *PHUFFMAN_ENCODING;

typedef struct _HUFFMAN_WORKSPACE {
    HUFFMAN_ENCODING Encodings[HUFFMAN_ALPHABET_SIZE];

    HUFFMAN_NODE NodeBuffer[2 * HUFFMAN_ALPHABET_SIZE + 1];

    union {
        struct {
            USHORT RadixBuckets[257];
            USHORT RadixBucketsHi[257];
            USHORT RadixPass1Sorted[HUFFMAN_ALPHABET_SIZE];
        };
        HUFFMAN_STACK_ENTRY Stack[HUFFMAN_BLOCK_SIZE_LOG + 3];
    };

    UCHAR SymbolToBitLength[HUFFMAN_ALPHABET_SIZE];

    ULONG Frequencies[HUFFMAN_ALPHABET_SIZE];

    UCHAR CompactBitLengths[HUFFMAN_ALPHABET_SIZE / 2];
} HUFFMAN_WORKSPACE, *PHUFFMAN_WORKSPACE;

typedef struct _XPRESS_HUFF_WORKSPACE {
    XPRESS_LZ_WORKSPACE Lz;
    HUFFMAN_WORKSPACE Huffman;

    //
    // How big does the LzPass buffer need to be to ensure that we can process
    // one Huffman block without filling it up?  If we encode every byte as a
    // literal, then it will take up 9 bits per input byte.  A three-byte match
    // takes up 25 bits, but this is better than the literal case, which is 27
    // bits.  Plus, we might have a match at the end that can take up 10 bytes
    // itself.  And we might use 4 extra bytes for unused tags.  So it needs to
    // be at least:
    //
    //    HUFFMAN_BLOCK_SIZE * 9 / 8 + 10 + 4
    //

    UCHAR LzPass[HUFFMAN_BLOCK_SIZE * 9 / 8 + 20];
} XPRESS_HUFF_WORKSPACE, *PXPRESS_HUFF_WORKSPACE;


typedef struct _XPRESS_HUFF_DECODE_WORKSPACE {
    USHORT LengthSortedListNext[HUFFMAN_ALPHABET_SIZE];
    USHORT LengthSortedListHead[MAX_ENCODING_LENGTH + 1];
    SHORT DecodeTable[1 << HUFFMAN_DECODE_LENGTH];
    SHORT DecodeTree[HUFFMAN_ALPHABET_SIZE * 2 + 1];
} XPRESS_HUFF_DECODE_WORKSPACE, *PXPRESS_HUFF_DECODE_WORKSPACE;

typedef struct _XPRESS_CALLBACK_PARAMS {
    PRTL_XPRESS_CALLBACK_FUNCTION Callback;
    PVOID CallbackContext;
    ULONG ProgressBytes;
} XPRESS_CALLBACK_PARAMS, *PXPRESS_CALLBACK_PARAMS;



NTSTATUS
RtlCompressBufferXpressHuffStandard (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalCompressedSize,
    IN PXPRESS_HUFF_WORKSPACE Workspace,
    IN PRTL_XPRESS_CALLBACK_FUNCTION Callback,
    IN PVOID CallbackContext,
    IN ULONG ProgressBytes
    );


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
    )

/*++

Routine Description:

    This routine takes as input an uncompressed buffer and produces its
    compressed equivalent provided the compressed data fits within the specified
    destination buffer.

    An output variable indicates the number of bytes used to store the
    compressed buffer.

Arguments:

    UncompressedBuffer - Supplies a pointer to the uncompressed data.

    UncompressedBufferSize - Supplies the size, in bytes, of the uncompressed
        buffer.

    CompressedBuffer - Supplies a pointer to where the compressed data is to be
        stored.

    CompressedBufferSize - Supplies the size, in bytes, of the compressed
        buffer.

    FinalCompressedSize - Receives the number of bytes needed in the compressed
        buffer to store the compressed data.

    WorkSpace - A buffer for compression data structures.

    Callback - The function to call back periodically.

    CallbackContext - A PVOID that will be passed to the Callback function.

    ProgressBytes - The callback will be invoked each time this many bytes are
        compressed.  (Note that this is the uncompressed size, not the
        compressed size).  So, if the caller wants the callback to be called
        about 7 times, it should specify ProgressBytes as
        UncompressedBufferSize/8.

Return Value:

    STATUS_SUCCESS - Compression was successful.

    STATUS_BUFFER_TOO_SMALL - The compressed buffer is too small to hold the
        compressed data.

--*/

{
    PXPRESS_HUFF_WORKSPACE HuffStandardWorkspace;
    PVOID AlignedWorkspace;

    AlignedWorkspace = ALIGN_UP_POINTER(WorkSpace, ULONG_PTR);

	HuffStandardWorkspace = (PXPRESS_HUFF_WORKSPACE)AlignedWorkspace;

	return RtlCompressBufferXpressHuffStandard(UncompressedBuffer,
											   UncompressedBufferSize,
											   CompressedBuffer,
											   CompressedBufferSize,
											   FinalCompressedSize,
											   HuffStandardWorkspace,
											   Callback,
											   CallbackContext,
											   ProgressBytes);
}

NOINLINE
PUCHAR
RtlpMakeXpressCallback (
    IN PXPRESS_CALLBACK_PARAMS Params,
    IN PUCHAR SafeEnd,
    IN PUCHAR Pos
    )

/*++

Routine Description:

    This routine exists as a performance optimization.  It enables the caller to
    avoid directly referencing several variables (Callback, CallbackContext, and
    ProgressBytes), thus improving the register allocation in the surrounding
    code.  This only works if this function is not inlined by the compiler.

    The caller would have code like this:

    if (Pos >= ProgressMark) {

        if (Pos >= SafeEnd) {
            goto End;
        }

        // The compiler doesn't know that this code is executed infrequently.
        // So it tries to optimize this code at the expense of the surrounding
        // code, leading to worse overall performance even if ProgressMark ==
        // SafeEnd (in which case, the below two lines are never run).

        Callback(CallbackContext);

        ProgressMark = min(SafeEnd, Pos + ProgressBytes);
    }

    Instead, it uses this function like so:

    (at the top of the function):

    Params.Callback = Callback;
    Params.CallbackContext = CallbackContext;
    Params.ProgressBytes = ProgressBytes;

    (in the middle of the function):

    if (Pos >= ProgressMark) {

        if (Pos >= SafeEnd) {
            goto End;
        }

        // Avoid referencing ProgressBytes, etc. so that the compiler doesn't
        // waste a register on it.

        ProgressMark = RtlpMakeXpressCallback(&Params, SafeEnd, Pos);
    }

Arguments:

    Params - A struct initialized with the callback parameters.

    SafeEnd - An upper bound on the return value.

    Pos - The current position in the buffer.

Return Value:

    min(SafeEnd, Pos + Params->ProgressBytes)

--*/

{
    Params->Callback(Params->CallbackContext);
    return min(SafeEnd, Pos + Params->ProgressBytes);
}



NTSTATUS
RtlCompressWorkSpaceSizeXpressHuff (
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG DecompressBufferWorkSpaceSize
    )

/*++

Routine Description:

    This routine returns to the caller the size in bytes of the different work
    space buffers need to perform the compression.

Arguments:

    CompressBufferWorkSpaceSize - Receives the size in bytes needed to compress
        a buffer.

    DecompressBufferWorkSpaceSize - Receives the size in bytes need to
        decompress a buffer.

Return Value:


--*/

{
	//
	// Make the size big enough to let us align it to a pointer boundary.
	//

	*CompressBufferWorkSpaceSize = sizeof(XPRESS_HUFF_WORKSPACE) +
								   sizeof(ULONG_PTR) - 1;

	*DecompressBufferWorkSpaceSize = sizeof(XPRESS_HUFF_DECODE_WORKSPACE) +
									 sizeof(ULONG_PTR) - 1;

	return STATUS_SUCCESS;

}




ULONG_PTR
XpressBuildHuffmanEncodings (
    IN PHUFFMAN_WORKSPACE Workspace
    )

/*++

Routine Description:

    Takes symbol frequencies, and computes canonical Huffman codes.

Arguments:

    Workspace - Contains input symbol frequencies and output code tables.

Return Value:

    The total bit length of the data (the bit length of each symbol) * (the
    number of times that symbol occurs, aka its frequency)

--*/

{
    HUFFMAN_NODE* NextFreeNode;
    HUFFMAN_NODE* Node;
    HUFFMAN_NODE* FirstQueueEnd;
    HUFFMAN_NODE* FirstQueue;
    HUFFMAN_NODE* SecondQueue;
    ULONG_PTR i;
    ULONG_PTR Freq;
    ULONG_PTR Symbol;
    ULONG_PTR SortedPos;
    ULONG_PTR SymbolCount;
    ULONG_PTR BitLen;
    ULONG_PTR StackPtr;
    ULONG_PTR MinBitCount;
    ULONG_PTR MaxBitCount;
    ULONG_PTR CurrentMask;
    ULONG_PTR TotalBitCount;
    ULONG_PTR TotalFrequency;
    UCHAR* CurrentCompactBitLen;
    HUFFMAN_ENCODING* Code;

    //
    // First, sort the symbols by frequency.  The maximum frequency this
    // function supports is 2^16-1, so we will sort with a two-pass radix sort.
    // This seems to be faster than quicksort.  If frequencies could be greater
    // than 2^16-1, quicksort would probably be a better choice.
    //

    memset(&Workspace->RadixBuckets[0], 0, sizeof(Workspace->RadixBuckets));
    memset(&Workspace->RadixBucketsHi[0], 0, sizeof(Workspace->RadixBucketsHi));

    //
    // Start "allocating" nodes from the beginning of the node array.
    //

    NextFreeNode = &Workspace->NodeBuffer[0];

    //
    // Initialize the symbol array.
    //

    memset(&Workspace->SymbolToBitLength[0], 0, sizeof(Workspace->SymbolToBitLength));

    //
    // Count for the radix sort.
    //

    for (i = 0; i < HUFFMAN_ALPHABET_SIZE; ++i) {

        Freq = Workspace->Frequencies[i];

        if (Freq != 0) {

            NextFreeNode->Frequency = Freq;

            ++Workspace->RadixBuckets[(Freq & 0xff) + 1];
            ++Workspace->RadixBucketsHi[(Freq >> 8) + 1];

            NextFreeNode->NullPtr = 0;
            NextFreeNode->Symbol = i;
            ++NextFreeNode;
        }
    }

    memset(&Workspace->CompactBitLengths[0], 0,
                  sizeof(Workspace->CompactBitLengths));

    if (NextFreeNode <= &Workspace->NodeBuffer[1]) {

        if (NextFreeNode == &Workspace->NodeBuffer[0]) {

            //
            // No symbol has Frequency > 0.  This is not expected, but we'll
            // handle it anyway.
            //

            return 0;
        }

        //
        // Only one symbol has Frequency > 0.  This is legal.  Give the symbol a
        // bit length of one and a code of zero.
        //

        Symbol = Workspace->NodeBuffer[0].Symbol;
        if (Symbol % 2) {
            Workspace->CompactBitLengths[Symbol/2] = 16;
        } else {
            Workspace->CompactBitLengths[Symbol/2] = 1;
        }

        Workspace->Encodings[Symbol].BitLength = 1;
        Workspace->Encodings[Symbol].Code = 0;

        return Workspace->NodeBuffer[0].Frequency;
    }

    //
    // Convert the radix counts to cumulative sums.
    //

    for (i = 1; i < 256; ++i) {
        Workspace->RadixBuckets[i] += Workspace->RadixBuckets[i-1];
        Workspace->RadixBucketsHi[i] += Workspace->RadixBucketsHi[i-1];
    }

    //
    // The first pass of the radix sort: a bucket sort by the lower 8 bits.
    //

    for (Node = &Workspace->NodeBuffer[0]; Node < NextFreeNode; ++Node) {
        Workspace->RadixPass1Sorted[
            Workspace->RadixBuckets[Node->Frequency & 0xff]++
            ] = (USHORT)Node->Symbol;
    }

    SymbolCount = NextFreeNode - &Workspace->NodeBuffer[0];

    //
    // The second pass of the radix sort: bucket sort by the high 8 bits, and
    // walk the symbols in the order provided by the first pass.
    //

    for (i = 0; i < SymbolCount; ++i) {
        Symbol = Workspace->RadixPass1Sorted[i];
        Freq = Workspace->Frequencies[Symbol];
        SortedPos = Workspace->RadixBucketsHi[Freq >> 8]++;

        Workspace->NodeBuffer[SortedPos].Symbol = Symbol;
        Workspace->NodeBuffer[SortedPos].Frequency = Freq;
    }

    //
    // Now that the symbols are sorted, we will build the Huffman tree.  Use the
    // "two queues" method.  The first queue consists of the raw symbols that we
    // just sorted.  The second queue starts empty, and we'll add all new nodes
    // to the end of the second queue.
    //

    //
    // Remember the end of the first queue so we can tell when it's empty.
    //

    FirstQueueEnd = NextFreeNode;

    //
    // Normally, this loop will only run once.  We might have build the tree
    // multiple times if any symbols get a code that exceeds
    // MAX_ENCODING_LENGTH.
    //

    for (;;) {

        //
        // Start the second queue immediately after the first queue in the
        // array.  Populate it with one node that is the parent of the two
        // least-frequent symbols.  We already made sure that the first queue
        // contains at least two nodes.
        //

        NextFreeNode = FirstQueueEnd;
        FirstQueue = &Workspace->NodeBuffer[2];
        SecondQueue = NextFreeNode;

        SecondQueue->Children[0] = &Workspace->NodeBuffer[0];
        SecondQueue->Children[1] = &Workspace->NodeBuffer[1];
        SecondQueue->Frequency = Workspace->NodeBuffer[0].Frequency +
                                 Workspace->NodeBuffer[1].Frequency;

        for (;;) {

            //
            // At this point, NextFreeNode always points to the last element of
            // the second queue.
            //

            assert(SecondQueue <= NextFreeNode);

            if (FirstQueue == FirstQueueEnd) {

                if (SecondQueue == NextFreeNode) {

                    //
                    // The first queue is empty, and the second queue has only
                    // node.  This means we're done building the tree, and the
                    // one remaining node is the root.
                    //

                    break;
                }
            }

            //
            // Advance NextFreeNode to the actual next free node.
            //

            ++NextFreeNode;

            //
            // We want to find the least-frequent parent-less node.  Since both
            // queues are sorted by frequency, it must be at the head of one of
            // the queues.
            //

            assert(FirstQueue + 1 >= FirstQueueEnd ||
                      FirstQueue->Frequency <= (FirstQueue + 1)->Frequency);
            assert(SecondQueue + 1 >= NextFreeNode ||
                      SecondQueue->Frequency <= (SecondQueue + 1)->Frequency);

            if (FirstQueue == FirstQueueEnd ||
                (SecondQueue < NextFreeNode &&
                 SecondQueue->Frequency < FirstQueue->Frequency))
            {

                //
                // In this case, the second queue contains the least-frequent
                // node.  Make it a child of the new node, and pop it from the
                // queue.
                //
                // If the first queue is empty, we already made sure the second
                // queue contains at least two nodes, so we can assert that it
                // isn't empty.
                //

                assert(SecondQueue != NextFreeNode);
                NextFreeNode->Children[0] = SecondQueue;
                NextFreeNode->Frequency = SecondQueue->Frequency;
                ++SecondQueue;

            } else {

                //
                // In this case, the first queue contains the least-frequent node.
                //

                NextFreeNode->Children[0] = FirstQueue;
                NextFreeNode->Frequency = FirstQueue->Frequency;
                ++FirstQueue;
            }

            assert(SecondQueue <= NextFreeNode);
            assert(FirstQueue < FirstQueueEnd || SecondQueue < NextFreeNode);

            assert(FirstQueue + 1 >= FirstQueueEnd ||
                      FirstQueue->Frequency <= (FirstQueue + 1)->Frequency);
            assert(SecondQueue + 1 >= NextFreeNode ||
                      SecondQueue->Frequency <= (SecondQueue + 1)->Frequency);

            //
            // Again, choose the least-frequent parent-less node.  Make it the
            // other child of the new node.  At its frequency to the frequency
            // of the other child.
            //

            if (FirstQueue == FirstQueueEnd ||
                (SecondQueue < NextFreeNode &&
                 SecondQueue->Frequency < FirstQueue->Frequency))
            {
                //
                // The second queue cannot be empty if (FirstQueue ==
                // FirstQueueEnd) because it started with at least two elements,
                // and we could have only removed one.
                //

                assert(SecondQueue != NextFreeNode);
                NextFreeNode->Children[1] = SecondQueue;
                NextFreeNode->Frequency += SecondQueue->Frequency;
                ++SecondQueue;

            } else {

                NextFreeNode->Children[1] = FirstQueue;
                NextFreeNode->Frequency += FirstQueue->Frequency;
                ++FirstQueue;
            }
        }

        //
        // At this point, NextFreeNode actually points to the root of the tree.
        //

        Node = NextFreeNode;
        BitLen = 0;
        StackPtr = 0;

        //
        // We need to traverse the entire tree and mark each node with its
        // depth, which represents the number of bits we'll use to encode it.
        //

        for (;;) {

            //
            // A useful invariant: each node has either two children or no
            // children.
            //

            if (Node->Children[0] == NULL) {

                //
                // This node is a leaf.  Set its bit length to the value we've
                // been maintaining.
                //

                Node->BitLength = BitLen;
                Workspace->SymbolToBitLength[Node->Symbol] = (UCHAR)BitLen;

                //
                // Continue the traversal by popping a node from the stack.  If
                // the stack is empty, we're done.
                //

                if (StackPtr == 0) {
                    break;
                }

                --StackPtr;
                Node = Workspace->Stack[StackPtr].Node;
                BitLen = Workspace->Stack[StackPtr].BitLength;

            } else {

                //
                // The node has two children.  Put the first child on the stack,
                // and follow the second child.  Node that the size of the stack
                // cannot exceed log_2 (HUFFMAN_BLOCK_SIZE + 1) * 2, which is
                // 18.
                //

                ++BitLen;
                assert(StackPtr < HUFFMAN_BLOCK_SIZE_LOG + 2);
                Workspace->Stack[StackPtr].BitLength = BitLen;
                Workspace->Stack[StackPtr].Node = Node->Children[0];
                ++StackPtr;

                Node = Node->Children[1];
            }
        }

        //
        // The least-frequent symbol must have the longest bit length.  Since we
        // still have the nodes in sorted order, we can easily check if any
        // symbol has a length that exceeds MAX_ENCODING_LENGTH.
        //

        if (Workspace->NodeBuffer[0].BitLength <= MAX_ENCODING_LENGTH) {
            break;
        }

        //
        // The longest encoding is too long.  Half all the frequencies, but
        // don't let any non-zero frequencies go to zero.  Rebuild the tree.
        // We don't need to re-sort.
        //

        for (Node = &Workspace->NodeBuffer[0]; Node < FirstQueueEnd; ++Node) {
            Node->NullPtr = 0;
            Node->Frequency = (Node->Frequency + 1) / 2;
        }
    }

    //
    // Now that we have the bit lengths of each symbol, we need to determine the
    // canonical encoding of each symbol.  We also need to populate the
    // CompactBitLengths array, which will be encoded in the data so that we can
    // determine the encodings at decompression time.
    //

    //
    // Determine the max and min bit lengths by looking at the least-frequent
    // and most-frequent symbols.
    //

    MaxBitCount = Workspace->NodeBuffer[0].BitLength;
    MinBitCount = (FirstQueueEnd - 1)->BitLength;

    assert(MaxBitCount == Workspace->NodeBuffer[1].BitLength);

    CurrentMask = 0;
    TotalBitCount = 0;

    for (BitLen = MinBitCount; BitLen <= MaxBitCount; ++BitLen) {

        //
        // For each possible bit length, enumerate (in "alphabetical" order) all
        // the symbols with that bit length.  Maintain the canonical code by
        // incrementing it for each symbol and doubling it for each new bit
        // length.  Add up the frequencies to compute the total bit length of
        // the data.
        //

        TotalFrequency = 0;
        CurrentCompactBitLen = &Workspace->CompactBitLengths[0];

        for (i = 0; i < HUFFMAN_ALPHABET_SIZE; ++i) {

            if (Workspace->SymbolToBitLength[i] == BitLen) {

                assert(Workspace->Frequencies[i] > 0);

                TotalFrequency += Workspace->Frequencies[i];

                CurrentCompactBitLen[0] |= BitLen;

                Code = &Workspace->Encodings[i];
                Code->BitLength = (USHORT)BitLen;
                Code->Code = (USHORT)CurrentMask;
                ++CurrentMask;
            }

            ++i;

            if (Workspace->SymbolToBitLength[i] == BitLen) {

                assert(Workspace->Frequencies[i] > 0);

                TotalFrequency += Workspace->Frequencies[i];

                CurrentCompactBitLen[0] |= BitLen * 16;

                Code = &Workspace->Encodings[i];
                Code->BitLength = (USHORT)BitLen;
                Code->Code = (USHORT)CurrentMask;
                ++CurrentMask;
            }

            ++CurrentCompactBitLen;
        }

        CurrentMask <<= 1;
        TotalBitCount += TotalFrequency * BitLen;
    }

    return TotalBitCount;
}

PUCHAR
XpressDoHuffmanPass (
    IN PHUFFMAN_WORKSPACE Workspace,
    IN PUCHAR LzInputPos,
    IN PUCHAR LzEnd,
    IN PUCHAR HuffOutputPos,
    IN LOGICAL WriteEof
    )

/*++

Routine Description:

    Encodes LZ77-encoded data with the given Huffman symbol encodings.

Arguments:

    Workspace - Contains Huffman encodings built by XpressBuildHuffmanEncodings.

    LzInputPos - The beginning of the LZ data.

    LzEnd - The end of the LZ data.

    HuffOutputPos - The place to start writing the Huffman output.  The caller
        must have already verified that the buffer is large enough.

    WriteEof - If true, writes the EOF symbol at the end of the buffer.

Return Value:

    A pointer to the next unused byte in the output buffer.

--*/

{
    ULONG_PTR CurrentShift;
    USHORT NextShort;
    PUCHAR HuffOutputPos1;
    PUCHAR HuffOutputPos2;
    INT Tags;
    HUFFMAN_ENCODING* HuffCode;
    UCHAR HuffValue;
    ULONG_PTR MatchLen;

    //
    // Copy the Huffman table to the output buffer.  TODO: It might be
    // faster to put this table directly in the output buffer from
    // XpressBuildHuffmanEncodings.
    //

    memcpy(HuffOutputPos, &Workspace->CompactBitLengths[0],
                  sizeof(Workspace->CompactBitLengths));

    HuffOutputPos += sizeof(Workspace->CompactBitLengths);

    //
    // Perform the Huffman encoding of the first-pass buffer.
    //

    CurrentShift = 16;
    NextShort = 0;

    HuffOutputPos1 = HuffOutputPos;
    HuffOutputPos += sizeof(USHORT);
    HuffOutputPos2 = HuffOutputPos;
    HuffOutputPos += sizeof(USHORT);

    goto HuffEncodeGetTags;

    for (;;) {

        for (;;) {

            if (Tags < 0) {
                break;
            }

            Tags *= 2;

        HuffmanEncodeProcessLiteral:

            HuffCode = &Workspace->Encodings[LzInputPos[0]];
            ++LzInputPos;

            if (CurrentShift >= HuffCode->BitLength) {

                //
                // This symbols fits in the current USHORT.  Make room for it
                // and OR it into the lower bits.
                //

                CurrentShift -= HuffCode->BitLength;
                NextShort <<= HuffCode->BitLength;
                NextShort |= HuffCode->Code;

            } else {

                //
                // This symbol will have to straddle two USHORTs.  First, OR in
                // the high bits.
                //

                NextShort <<= CurrentShift;
                NextShort |= HuffCode->Code >>
                             (HuffCode->BitLength - CurrentShift);
                CurrentShift -= HuffCode->BitLength;

                //
                // Write out the now-filled USHORT.  We need three HuffOutputPos
                // variables for the match length encoding to work properly.
                // The full symbol encoding must come before the length bytes,
                // and it might straddle two USHORTs, so we need to reserve the
                // second USHORT in advance.
                //

                *((USHORT UNALIGNED *)HuffOutputPos1) = NextShort;
                HuffOutputPos1 = HuffOutputPos2;
                HuffOutputPos2 = HuffOutputPos;
                HuffOutputPos += sizeof(USHORT);

                //
                // Put the low bits of the symbol into the next USHORT.
                //

                CurrentShift += 16;
                NextShort = HuffCode->Code;
            }
        }

        Tags *= 2;

        if (Tags == 0) {

        HuffEncodeGetTags:

            //
            // Load the next 32 tags.
            //

#if defined(_ARM_) || defined(_ARM64_)
            __prefetch(LzInputPos + 48);
            __prefetch(LzInputPos + 64);
#endif

            Tags = *((INT UNALIGNED *)LzInputPos);
            LzInputPos += sizeof(Tags);

            if (Tags < 0) {
                Tags = Tags * 2 + 1;
            } else {
                Tags = Tags * 2 + 1;
                goto HuffmanEncodeProcessLiteral;
            }
        }

        //
        // The first pass always ends with a tag of '1'.
        //

        if (LzInputPos >= LzEnd) {
            break;
        }

        //
        // Encode the match symbol.
        //

        HuffValue = LzInputPos[0];
        ++LzInputPos;

        HuffCode = &Workspace->Encodings[HuffValue + 256];

        if (CurrentShift >= HuffCode->BitLength) {
            CurrentShift -= HuffCode->BitLength;
            NextShort <<= HuffCode->BitLength;
            NextShort |= HuffCode->Code;
        } else {
            NextShort <<= CurrentShift;
            NextShort |= HuffCode->Code >>
                        (HuffCode->BitLength - CurrentShift);
            CurrentShift -= HuffCode->BitLength;
            *((USHORT UNALIGNED *)HuffOutputPos1) = NextShort;
            HuffOutputPos1 = HuffOutputPos2;
            HuffOutputPos2 = HuffOutputPos;
            HuffOutputPos += sizeof(USHORT);
            CurrentShift += 16;
            NextShort = HuffCode->Code;
        }

        if ((HuffValue % LEN_MULT) == LEN_MULT - 1) {

            //
            // Copy over the extra length bytes.
            //

            MatchLen = LzInputPos[0];
            ++LzInputPos;

            HuffOutputPos[0] = (UCHAR)MatchLen;
            ++HuffOutputPos;

            if (MatchLen == 255) {

                MatchLen = *((USHORT UNALIGNED *)LzInputPos);

                HuffOutputPos[0] = LzInputPos[0];
                HuffOutputPos[1] = LzInputPos[1];

                HuffOutputPos += 2;
                LzInputPos += 2;

                if (MatchLen == 0) {

                    HuffOutputPos[0] = LzInputPos[0];
                    HuffOutputPos[1] = LzInputPos[1];
                    HuffOutputPos[2] = LzInputPos[2];
                    HuffOutputPos[3] = LzInputPos[3];

                    HuffOutputPos += 4;
                    LzInputPos += 4;
                }
            }
        }

        //
        // Compute the bit length of the offset so we know how many bits to
        // write.
        //

        HuffValue /= LEN_MULT;

        //
        // Write the offset.
        //

        if (CurrentShift >= HuffValue) {
            CurrentShift -= HuffValue;
            NextShort <<= HuffValue;
            NextShort |= *((USHORT UNALIGNED *)LzInputPos);
        } else {
            NextShort <<= CurrentShift;
            NextShort |= *((USHORT UNALIGNED *)LzInputPos) >>
                         (HuffValue - CurrentShift);
            CurrentShift -= HuffValue;
            *((USHORT UNALIGNED *)HuffOutputPos1) = NextShort;
            HuffOutputPos1 = HuffOutputPos2;
            HuffOutputPos2 = HuffOutputPos;
            HuffOutputPos += sizeof(USHORT);
            CurrentShift += 16;
            NextShort = *((USHORT UNALIGNED *)LzInputPos);
        }

        LzInputPos += sizeof(USHORT);
    }

    if (WriteEof) {

        //
        // If this is the end of input buffer, append the EOF symbol.
        //

        HuffCode = &Workspace->Encodings[256];

        if (CurrentShift >= HuffCode->BitLength) {
            CurrentShift -= HuffCode->BitLength;
            NextShort <<= HuffCode->BitLength;
            NextShort |= HuffCode->Code;
        } else {
            NextShort <<= CurrentShift;
            NextShort |= HuffCode->Code >>
                         (HuffCode->BitLength - CurrentShift);
            CurrentShift -= HuffCode->BitLength;
            *((USHORT UNALIGNED *)HuffOutputPos1) = NextShort;
            HuffOutputPos1 = HuffOutputPos2;
            HuffOutputPos2 = HuffOutputPos;
            HuffOutputPos += sizeof(USHORT);
            CurrentShift += 16;
            NextShort = HuffCode->Code;
        }
    }

    //
    // Write out the unfinished USHORT.
    //

    NextShort <<= CurrentShift;
    *((USHORT UNALIGNED *)HuffOutputPos1) = NextShort;
    *((USHORT UNALIGNED *)HuffOutputPos2) = 0;

    return HuffOutputPos;
}

NTSTATUS
RtlCompressBufferXpressHuffStandard (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalCompressedSize,
    IN PXPRESS_HUFF_WORKSPACE Workspace,
    IN PRTL_XPRESS_CALLBACK_FUNCTION Callback,
    IN PVOID CallbackContext,
    IN ULONG ProgressBytes
    )

/*++

Routine Description:

    This routine takes as input an uncompressed buffer and produces its
    compressed equivalent provided the compressed data fits within the specified
    destination buffer.

    An output variable indicates the number of bytes used to store the
    compressed buffer.

    This function uses a fast LZ77+Huffman method.

Arguments:

    UncompressedBuffer - Supplies a pointer to the uncompressed data.

    UncompressedBufferSize - Supplies the size, in bytes, of the uncompressed
        buffer.

    CompressedBuffer - Supplies a pointer to where the compressed data is to be
        stored.

    CompressedBufferSize - Supplies the size, in bytes, of the compressed
        buffer.

    FinalCompressedSize - Receives the number of bytes needed in the compressed
        buffer to store the compressed data.

    WorkSpace - A buffer for compression data structures.

    Callback - The function to call back periodically.

    CallbackContext - A PVOID that will be passed to the Callback function.

    ProgressBytes - The callback will be invoked each time this many bytes are
        compressed.  (Note that this is the uncompressed size, not the
        compressed size).  So, if the caller wants the callback to be called
        about 7 times, it should specify ProgressBytes as
        UncompressedBufferSize/8.

Return Value:

    STATUS_SUCCESS - Compression was successful.

    STATUS_BUFFER_TOO_SMALL - The compressed buffer is too small to hold the
        compressed data.

--*/

{
    PUCHAR SafeBufferEnd;
    PUCHAR InputBufferEnd;
    PUCHAR OutputBufferEnd;
    PUCHAR InputPos;
    PUCHAR OutputPos;
    PUCHAR HuffOutputPos;
    PUCHAR Match;
    PUCHAR MatchEx;
    PUCHAR SavedInputPos;
    PUCHAR HuffBlockEnd;
    PUCHAR SafeHuffBlockEnd;
    PUCHAR ProgressMark;
    ULONG RunningTags;
    ULONG UNALIGNED * TagsOut;
    ULONG UNALIGNED * InputPosLong;
    ULONG UNALIGNED * MatchLong;
    PUCHAR* HashTableTemp;
    ULONG_PTR i;
    ULONG_PTR HashValue;
    ULONG_PTR ExHashValue;
    ULONG_PTR MatchLen;
    ULONG_PTR MatchOffset;
    ULONG_PTR OffsetBitSize;
    ULONG_PTR OffsetBits;
    ULONG_PTR NonHuffmanBytes;
    ULONG_PTR BlockBitSize;
    ULONG_PTR BlockByteSize;
    UCHAR HuffValue;
    LOGICAL ReachedEnd;
    XPRESS_CALLBACK_PARAMS CallbackParams;

    InputBufferEnd = UncompressedBuffer + UncompressedBufferSize;
    OutputBufferEnd = CompressedBuffer + CompressedBufferSize;

    if (CompressedBufferSize < 300) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    //
    // Initialize hash tables.
    //

    for (i = 0; i < HASH_TABLE_SIZE; ++i) {
        Workspace->Lz.HashTable[i] = UncompressedBuffer;
    }
    for (i = 0; i < HASH_TABLE_EX_SIZE; ++i) {
        Workspace->Lz.HashTableEx[i] = UncompressedBuffer;
    }

    //
    // The "safe" buffer is 40 bytes: 32 for literals, plus 7 for hashing
    // without bound-checking, plus one for good luck.
    //

    SafeBufferEnd = InputBufferEnd - 40;

    InputPos = UncompressedBuffer;
    HuffOutputPos = CompressedBuffer;

    if (Callback == NULL ||
        ProgressBytes > UncompressedBufferSize)
    {
        ProgressBytes = UncompressedBufferSize;
    }

    CallbackParams.Callback = Callback;
    CallbackParams.CallbackContext = CallbackContext;
    CallbackParams.ProgressBytes = ProgressBytes;

    for (;;) {

        //
        // This code runs two passes over the data.  The first pass performs
        // LZ77 compression, and the second pass performs Huffman encoding.  The
        // first pass writes its output to a temporary buffer in the workspace.
        // In order to keep the workspace reasonably small, while still
        // supporting arbitrarily large input buffers, we must use a strategy
        // like this:
        //
        // 1. Do LZ77 on first 64k of input data, writing to workspace.
        // 2. Do Huffman on LZ77 data.
        // 3. Do LZ77 on second 64k, overwriting previous LZ77 data.
        // 4. Do Huffman on new LZ77 data.
        // 5. Etc... Until the entire input has been processed.
        //
        // The older version of Xpress does not accept input buffers larger than
        // 64k.  For buffers that are not larger than 64k, we must be
        // compatible, so we must use one LZ77/Huffman iteration.
        //
        // Whereas the older version of Xpress had only one Huffman table per
        // buffer, we may have multiple Huffman tables if the input buffer is
        // larger than 64kb.
        //
        // The compression format looks like this: first, 256 bytes for the
        // Huffman table, with every 4 bits representing the bit length of the
        // corresponding symbol (we have 512 symbols in the Huffman alphabet).
        // After the table, we have Huffman-encoded symbols, for an uncompressed
        // size of about 64kb (less only if the input buffer is smaller than
        // 64kb, and more only if the last symbol is a match whose length
        // extends beyond the 64kb boundary).  After that, the format repeats
        // itself as many times as necessary to encode the entire input buffer:
        // (Huffman table, encoded symbols, Huffman table, encoded symbols, ...).
        //
        // Here is a description of the Huffman alphabet: the first 256 symbols
        // are literals corresponding to the 256 possible raw byte values, and
        // the next 256 symbols are matches.  The 256 match symbols are split
        // into two 4-bit values: the high 4 bits represent the bit length of
        // the match offset, and the low 4 bits represent the match length minus
        // 3.  If the match length minus 3 is greater than or equal to 15, then
        // a value of 15 is encoded in the low 4 bits, and the full match length
        // is encoded in the next byte of the stream (this is hard to describe
        // precisely because it depends on processing the symbols in 16-bit
        // words; the easiest way to understand it is to look at the
        // decompression function).  After a match symbol, the offset of the
        // match is encoded (directly in the "Huffman" stream, unlike the extra
        // length bits) using the number of bits specified in the high 4 bits of
        // the match symbol.
        //

        //
        // By keeping track of the number of bits we use to encode offsets and
        // extra lengths, we can calculate exactly how big the Huffman-encoded
        // data will be, so that we can avoid doing output-bound checks.
        //

        OffsetBits = 0;
        NonHuffmanBytes = 0;

        //
        // Initialize the frequency counts of the Huffman symbols.
        //

        memset(&Workspace->Huffman.Frequencies[0], 0,
                      sizeof(Workspace->Huffman.Frequencies));

        OutputPos = &Workspace->LzPass[0];

        //
        // Set up the end of the Huffman block.
        //

        HuffBlockEnd = InputPos + HUFFMAN_BLOCK_SIZE;

        if (HuffBlockEnd > InputBufferEnd) {
            HuffBlockEnd = InputBufferEnd;
        }

        SafeHuffBlockEnd = HuffBlockEnd - 40;

        assert(SafeHuffBlockEnd <= SafeBufferEnd);

        ProgressMark = min(SafeHuffBlockEnd, InputPos + ProgressBytes);

        RunningTags = 1;
        TagsOut = (ULONG UNALIGNED *)OutputPos;
        OutputPos += sizeof(ULONG);

        if (InputPos == UncompressedBuffer) {

            //
            // The first byte in the input can never be a match.  Encode it as a
            // literal.  This is necessary to prevent ourselves from matching
            // the first byte to itself (since the hash table was initialized
            // with the UncompressedBuffer pointer).
            //

            ++Workspace->Huffman.Frequencies[InputPos[0]];
            OutputPos[0] = InputPos[0];
            ++InputPos;
            ++OutputPos;
            RunningTags *= 2;
        }

        if (InputPos >= SafeHuffBlockEnd) {
            goto SafeDone;
        }

        for (;;) {

            for (;;) {

                //
                // Try to find a three-byte match.
                //

                HashTableTemp = &Workspace->Lz.HashTable[InputPos[0]];
                HashValue = InputPos[2] * FIRST_HASH_COEFF2 +
                            InputPos[1] * FIRST_HASH_COEFF;

                Match = HashTableTemp[HashValue * 2];
                HashTableTemp[HashValue * 2] = InputPos;

                if (Match[0] == InputPos[0] &&
                    Match[1] == InputPos[1] &&
                    Match[2] == InputPos[2] &&
                    InputPos - Match < HUFF_WINDOW_SIZE)
                {
                    break;
                }

            EncodeLiteral:

                //
                // Encode a literal.
                //

                ++Workspace->Huffman.Frequencies[InputPos[0]];
                OutputPos[0] = InputPos[0];
                ++OutputPos;
                ++InputPos;

                if ((INT)RunningTags > 0) {
                    RunningTags *= 2;
                    continue;
                }

                //
                // Start a new chunk of tags.
                //

                RunningTags *= 2;
                *TagsOut = RunningTags;
                RunningTags = 1;
                TagsOut = (ULONG UNALIGNED *)OutputPos;
                OutputPos += sizeof(ULONG);

                //
                // Make sure we can process one tag-chunk worth of literals
                // without checking bounds.
                //

                if (InputPos >= ProgressMark) {

                    if (InputPos >= SafeHuffBlockEnd) {
                        goto SafeDone;
                    }

                    ProgressMark = RtlpMakeXpressCallback(&CallbackParams,
                                                          SafeHuffBlockEnd,
                                                          InputPos);
                }
            }

            SavedInputPos = InputPos;

            if (InputPos[3] != Match[3]) {

                //
                // This match is only three bytes.  Use the secondary hash table
                // to find a four-byte match.
                //

                ExHashValue = HashValue * 2 + InputPos[0];

                MatchEx = Workspace->Lz.HashTableEx[ExHashValue + InputPos[3]];
                Workspace->Lz.HashTableEx[ExHashValue + Match[3]] = Match;

                if (InputPos - MatchEx < HUFF_WINDOW_SIZE &&
                    *((ULONG UNALIGNED *)InputPos) == *((ULONG UNALIGNED *)MatchEx))
                {
                    Workspace->Lz.HashTableEx[ExHashValue + InputPos[3]] = InputPos;
                    Match = MatchEx;
                } else {
                    InputPos += 3;
                    Match += 3;
                    goto MatchDone;
                }
            }

            if (InputPos[4] != Match[4]) {

                ExHashValue = _rotl8(InputPos[0] ^ _rotr8(InputPos[2] + InputPos[1] - 159, 1),
                                     3) * HASH_COEFF_HIGH
                              +
                              _rotr8(InputPos[1] ^ _rotl8(InputPos[3] + InputPos[0], 3),
                                     1) * HASH_COEFF_LOW;

                MatchEx = Workspace->Lz.HashTableEx[ExHashValue + InputPos[4]];
                Workspace->Lz.HashTableEx[ExHashValue + Match[4]] = Match;

                if (InputPos - MatchEx < HUFF_WINDOW_SIZE &&
                    *((ULONG UNALIGNED *)InputPos) == *((ULONG UNALIGNED *)MatchEx) &&
                    InputPos[4] == MatchEx[4] &&
                    InputPos != MatchEx)
                {
                    Workspace->Lz.HashTableEx[ExHashValue + InputPos[4]] = InputPos;
                    Match = MatchEx;
                } else {
                    InputPos += 4;
                    Match += 4;
                    goto MatchDone;
                }
            }

            if (InputPos[5] != Match[5]) {

                ExHashValue = (_rotl8((InputPos[4] ^ _rotr8(InputPos[0], 1)) - 187, 3)
                               + _rotr8(InputPos[1] ^ _rotl8(InputPos[3], 3), 1))
                              * HASH_COEFF_HIGH
                              + (InputPos[2] ^ _rotl8(InputPos[0], InputPos[4]))
                              * HASH_COEFF_LOW;

                MatchEx = Workspace->Lz.HashTableEx[ExHashValue + InputPos[5]];
                Workspace->Lz.HashTableEx[ExHashValue + Match[5]] = Match;

                if (InputPos - MatchEx < HUFF_WINDOW_SIZE &&
                    *((ULONG UNALIGNED *)InputPos) == *((ULONG UNALIGNED *)MatchEx) &&
                    InputPos[4] == MatchEx[4] &&
                    InputPos[5] == MatchEx[5] &&
                    InputPos != MatchEx)
                {
                    Workspace->Lz.HashTableEx[ExHashValue + InputPos[5]] = InputPos;
                    Match = MatchEx;
                } else {
                    InputPos += 5;
                    Match += 5;
                    goto MatchDone;
                }
            }

            InputPos += 6;
            Match += 6;

            InputPosLong = (ULONG UNALIGNED *)InputPos;
            MatchLong = (ULONG UNALIGNED *)Match;

            while (InputPos < SafeBufferEnd) {
                if (InputPosLong[0] != MatchLong[0])
                { goto FinishMatch; }
                if (InputPosLong[1] != MatchLong[1])
                { InputPos += 4; Match += 4; goto FinishMatch; }
                if (InputPosLong[2] != MatchLong[2])
                { InputPos += 8; Match += 8; goto FinishMatch; }
                if (InputPosLong[3] != MatchLong[3])
                { InputPos += 12; Match += 12; goto FinishMatch; }
                if (InputPosLong[4] != MatchLong[4])
                { InputPos += 16; Match += 16; goto FinishMatch; }
                if (InputPosLong[5] != MatchLong[5])
                { InputPos += 20; Match += 20; goto FinishMatch; }
                if (InputPosLong[6] != MatchLong[6])
                { InputPos += 24; Match += 24; goto FinishMatch; }
                if (InputPosLong[7] != MatchLong[7])
                { InputPos += 28; Match += 28; goto FinishMatch; }
                InputPos += 32;
                Match += 32;
                InputPosLong = (ULONG UNALIGNED *)InputPos;
                MatchLong = (ULONG UNALIGNED *)Match;
            }

            while (InputPos < InputBufferEnd && InputPos[0] == Match[0]) {
                ++InputPos;
                ++Match;
            }
            goto MatchDone;

        FinishMatch:
            if (InputPos[0] != Match[0])
            { goto MatchDone; }
            if (InputPos[1] != Match[1])
            { InputPos += 1; Match += 1; goto MatchDone; }
            if (InputPos[2] != Match[2])
            { InputPos += 2; Match += 2; goto MatchDone; }
            InputPos += 3;
            Match += 3;

        MatchDone:

            MatchLen = InputPos - SavedInputPos;
            MatchOffset = InputPos - Match;

            assert(MatchOffset < HUFF_WINDOW_SIZE);

            if (MatchLen == 3 && MatchOffset > (1 << 12)) {

                //
                // We tend to get better compression ratios by not encoding
                // three-byte long-distance matches.
                //

                InputPos = SavedInputPos;
                goto EncodeLiteral;
            }

            //
            // Compute the bit length of the offset.
            //

            if (MatchOffset >= 256) {
                OffsetBitSize = XpressHighBitIndexTable[MatchOffset >> 8] + 8;
            } else {
                OffsetBitSize = XpressHighBitIndexTable[MatchOffset];
            }

            //
            // Reset the high bit because we won't encode it.
            //

            MatchOffset -= ((ULONG_PTR)1 << OffsetBitSize);

            //
            // Keep track of the extra size in the output buffer.
            //

            OffsetBits += OffsetBitSize;

            //
            // Compute the alphabet value that we'll encode.
            //

            HuffValue = (UCHAR)(OffsetBitSize * LEN_MULT);

            MatchLen -= 3;

            if (MatchLen >= LEN_MULT - 1) {

                //
                // Use an extra byte to encode the long length.
                //

                HuffValue += LEN_MULT - 1;

                MatchLen -= LEN_MULT - 1;

                OutputPos[0] = HuffValue;
                ++OutputPos;

                if (MatchLen < 255) {

                    OutputPos[0] = (UCHAR)MatchLen;
                    ++OutputPos;
                    ++NonHuffmanBytes;

                } else {

                    //
                    // Use two more bytes.
                    //

                    OutputPos[0] = 255;

                    MatchLen += LEN_MULT - 1;

                    if (MatchLen < (1 << 16)) {

                        *((USHORT UNALIGNED *)(OutputPos + 1)) = (USHORT)MatchLen;
                        OutputPos += 3;
                        NonHuffmanBytes += 3;

                    } else {

                        //
                        // Use four more bytes.  Since the data type of
                        // UncompressedBufferSize is ULONG, we don't need to
                        // worry about lengths that don't fit in four bytes.
                        //

                        *((USHORT UNALIGNED *)(OutputPos + 1)) = 0;
                        OutputPos += 3;
                        NonHuffmanBytes += 3;

                        *((ULONG UNALIGNED *)OutputPos) = (ULONG)MatchLen;
                        OutputPos += 4;
                        NonHuffmanBytes += 4;
                    }
                }

            } else {

                //
                // Write the symbol value to the first-pass buffer.
                //

                HuffValue += (UCHAR)MatchLen;
                OutputPos[0] = HuffValue;
                ++OutputPos;
            }

            //
            // Update the frequency of the symbol.
            //

            ++Workspace->Huffman.Frequencies[HuffValue + 256];

            //
            // Write the offset to the first-pass buffer.
            //

            *((USHORT UNALIGNED *)OutputPos) = (USHORT)MatchOffset;
            OutputPos += sizeof(USHORT);

            if ((int)RunningTags > 0) {

                RunningTags = RunningTags * 2 + 1;

            } else {

                RunningTags = RunningTags * 2 + 1;
                *TagsOut = RunningTags;
                RunningTags = 1;
                TagsOut = (ULONG UNALIGNED *)OutputPos;
                OutputPos += sizeof(ULONG);
            }

            if (InputPos >= ProgressMark) {

                if (InputPos >= SafeHuffBlockEnd) {
                    break;
                }

                ProgressMark = RtlpMakeXpressCallback(&CallbackParams,
                                                      SafeHuffBlockEnd,
                                                      InputPos);
            }
        }

    SafeDone:

        while (InputPos < HuffBlockEnd) {

            //
            // Encode the "unsafe" remainder as literals.
            //

            ++Workspace->Huffman.Frequencies[InputPos[0]];
            OutputPos[0] = InputPos[0];
            ++OutputPos;
            ++InputPos;

            if ((INT)RunningTags > 0) {

                RunningTags *= 2;

            } else {

                RunningTags *= 2;
                *TagsOut = RunningTags;
                RunningTags = 1;
                TagsOut = (ULONG UNALIGNED *)OutputPos;
                OutputPos += sizeof(ULONG);
            }
        }

        while ((INT)RunningTags > 0) {
            RunningTags = RunningTags * 2 + 1;
        }

        RunningTags = RunningTags * 2 + 1;

        *TagsOut = RunningTags;

        if (InputPos >= InputBufferEnd) {

            ReachedEnd = TRUE;

            //
            // If we reached the end of the input buffer, increment the
            // frequency of symbol 256, which is the EOF symbol.  (This is a
            // back-compat thing.)
            //

            ++Workspace->Huffman.Frequencies[256];

        } else {

            ReachedEnd = FALSE;
        }

        //
        // Build the canonical Huffman encodings.
        //

        BlockBitSize = XpressBuildHuffmanEncodings(&Workspace->Huffman);

        //
        // Compute the byte size of the Huffman chunk we're about to write, and
        // make sure it fits in the space we have left in the output buffer.
        //

        BlockBitSize += OffsetBits;

        BlockByteSize = (BlockBitSize + 31) / 32 * 4 + NonHuffmanBytes + 2
                        + sizeof(Workspace->Huffman.CompactBitLengths);

        if (HuffOutputPos + BlockByteSize >= OutputBufferEnd) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        //
        // Re-encode the LzPass buffer using the Huffman encodings we just
        // computed.
        //

        HuffOutputPos = XpressDoHuffmanPass(&Workspace->Huffman,
                                            &Workspace->LzPass[0],
                                            OutputPos,
                                            HuffOutputPos,
                                            ReachedEnd);

        if (ReachedEnd) {
            break;
        }
    }

    assert(HuffOutputPos <= OutputBufferEnd);

    *FinalCompressedSize = (ULONG)(HuffOutputPos - CompressedBuffer);

    return STATUS_SUCCESS;
}

NTSTATUS
XpressBuildHuffmanDecodingTable (
    OUT PXPRESS_HUFF_DECODE_WORKSPACE Workspace,
    IN PUCHAR BitLengths
    )

/*++

Routine Description:

    Takes symbol bit lengths, and computes canonical Huffman codes in a format
    convenient for efficient decoding.

Arguments:

    Workspace - The decoding table, tree, and misc extra space.

    BitLengths - The CompactBitLengths array produced by
                 XpressBuildHuffmanEncodings.

Return Value:

    STATUS_BAD_COMPRESSION_BUFFER - The Huffman table is not valid.

--*/

{
    ULONG_PTR i;
    ULONG_PTR j;
    ULONG_PTR k;
    ULONG_PTR BitLen;
    ULONG_PTR Code;
    ULONG_PTR TableLo;
    ULONG_PTR NextTreeNode;
    SHORT PrevLevelPointer;
    SHORT PrevLevelEnd;
    SHORT DecodeValue = 0;
    LOGICAL FilledTable;

    //
    // This function builds a hybrid table/tree structure for canonical Huffman
    // decoding.  The idea is to use the table to decode short (frequent)
    // symbols, and to use the more-space-efficient tree to decode long symbols.
    //
    // Each 16-bit entry in the table/tree follows this protocol:
    //   If the high bit is zero, the table/tree value is
    //   (decoded symbol) * 16 + (symbol bit length)
    //   If the high bit is one, the table/tree value is
    //   -(index of tree node to decode remaining bits)
    //
    // The code for constructing the table/tree is somewhat hard to understand
    // because it's very fast.
    //

    //
    // First, initialize our lists.
    //

    for (i = 0; i <= MAX_ENCODING_LENGTH; ++i) {
        Workspace->LengthSortedListHead[i] = HUFFMAN_ALPHABET_SIZE;
    }

    //
    // We want to bucketize the symbols according to bit length (between 1 and
    // 15) in lists that are sorted by symbol value in reverse order.
    //

    for (i = 0; i < HUFFMAN_ALPHABET_SIZE / 2; ++i) {

        BitLen = BitLengths[i] & 15;

        if (BitLen) {

            //
            // Insert this symbol into the appropriate single-linked list.  The
            // symbol value is 2 * i.
            //

            Workspace->LengthSortedListNext[2 * i] =
                Workspace->LengthSortedListHead[BitLen];
            Workspace->LengthSortedListHead[BitLen] = (USHORT)(2 * i);
        }

        BitLen = BitLengths[i] >> 4;

        if (BitLen) {

            //
            // The symbol value is now 2 * i + 1.  Note that we do larger symbol
            // second so that they will be properly sorted if they have the same
            // bit length.
            //

            Workspace->LengthSortedListNext[2 * i + 1] =
                Workspace->LengthSortedListHead[BitLen];
            Workspace->LengthSortedListHead[BitLen] = (USHORT)(2 * i + 1);
        }
    }

    //
    // Next we're going to build the tree.  We do this from the bottom up.
    // Here's an example of how the algorithm works: suppose we have 20 symbols
    // with a bit length of 15.  First, create the leaf nodes for each of these
    // 20 symbols.  Then, move to the length-14 case.  Create 10 non-leaf nodes
    // whose children are the 20 initial leaf nodes (in order).  Create (let's
    // say 30) leaf nodes for the length-14 symbols.  Create 20 ([30 + 10] / 2)
    // non-leaf nodes linking to the existing 40 nodes.  And so on, for each bit
    // length down to 11.  Then, create entries in the decoding *table* (not
    // tree) linking to all the remaining top-level nodes in the tree.
    //
    // The number of nodes in the tree cannot exceed 2 * HUFFMAN_ALPHABET_SIZE.
    //

    NextTreeNode = HUFFMAN_ALPHABET_SIZE * 2;
    PrevLevelEnd = -HUFFMAN_ALPHABET_SIZE * 2;
    PrevLevelPointer = -HUFFMAN_ALPHABET_SIZE * 2 + 1;

    for (i = MAX_ENCODING_LENGTH; i > HUFFMAN_DECODE_LENGTH; --i) {

        //
        // Create non-leaf nodes linking to every other existing node.  Note
        // that a node's children are -DecodeTree[node] and -DecodeTree[node]+1.
        // So we only specify the '0' child, and the '1' child is implied.
        //

        while (PrevLevelPointer < PrevLevelEnd) {

            //
            // Note that PrevLevelPointer and PrevLevelEnd are negative values.
            // This prevents us from having to do a negate in each loop
            // iteration.
            //

            assert(NextTreeNode > 0);
            assert(NextTreeNode < (ULONG)-PrevLevelPointer);
            Workspace->DecodeTree[NextTreeNode] = PrevLevelPointer;
            --NextTreeNode;
            PrevLevelPointer += 2;
        }

        if (PrevLevelPointer == PrevLevelEnd) {
            return STATUS_BAD_COMPRESSION_BUFFER;
        }

        //
        // The last node we referenced in the above loop was PrevLevelEnd.  The
        // very next node will be the first node we will reference on the next
        // loop iteration.
        //

        PrevLevelPointer = PrevLevelEnd + 1;

        //
        // Create leaf nodes for all the symbols with bit length i.  Note that
        // we walk the symbols in reverse-sorted order and we allocate the nodes
        // in reverse order.  This is consistent with the canonical decoding
        // where symbols are in sorted order, and we add one to the node index
        // if the next bit is 1.
        //

        for (j = Workspace->LengthSortedListHead[i];
             j != HUFFMAN_ALPHABET_SIZE;
             j = Workspace->LengthSortedListNext[j])
        {
            assert(NextTreeNode > 0);
            Workspace->DecodeTree[NextTreeNode] = (SHORT)(j * 16 + i);
            --NextTreeNode;
        }

        //
        // Remember the end of this level so we know where to stop referencing
        // in the next loop iteration.
        //

        PrevLevelEnd = -(SHORT)NextTreeNode;
    }

    //
    // Fill in the table entries that need tree decoding.  This is conveniently
    // simple because all the entries must be at the end of the table.  And, if
    // the Huffman table is well-formed, the number of entries will simply be
    // half the number of top-level nodes in the tree.
    //
    // Note that we don't have to worry about the Code < 0 case, because that
    // would imply that there were 2^10 symbols with a length of 11 or more, and
    // since the total number of symbols is 2^9, this cannot happen.
    //

    Code = (1 << HUFFMAN_DECODE_LENGTH) - 1;

    while (PrevLevelPointer < PrevLevelEnd) {
        assert(PrevLevelPointer < 0);
        Workspace->DecodeTable[Code] = PrevLevelPointer;
        assert(Code > 0);
        --Code;
        PrevLevelPointer += 2;
    }

    if (PrevLevelPointer == PrevLevelEnd) {
        return STATUS_BAD_COMPRESSION_BUFFER;
    }

    //
    // Now, fill in the table for symbols whose bit length is less than or equal
    // to HUFFMAN_DECODE_LENGTH.
    //

    FilledTable = FALSE;
    assert(Code > 0);

    for (i = HUFFMAN_DECODE_LENGTH; i > 0; --i) {

        for (j = Workspace->LengthSortedListHead[i];
             j != HUFFMAN_ALPHABET_SIZE;
             j = Workspace->LengthSortedListNext[j])
        {
            //
            // Compute the table value: (decoded symbol) * 16 + (bit length)
            //

            DecodeValue = (SHORT)(j * 16 + i);

            //
            // If we already filled the table, and there are more symbols, the
            // Huffman table is invalid.  Return an error in this case.
            //

            if (FilledTable) {
                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            //
            // If the area we would fill would extend beyond the table size, the
            // Huffman table must be invalid.
            //

            if ((Code + 1) << (HUFFMAN_DECODE_LENGTH - i) >
                (1 << HUFFMAN_DECODE_LENGTH))
            {
                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            //
            // We want to fill in (1 << (HUFFMAN_DECODE_LENGTH - i)) table
            // entries, starting from (Code << (HUFFMAN_DECODE_LENGTH - i)).
            //
            // The switch statement with unrolled loops seems to be the fastest
            // way to do this.
            //

            TableLo = Code << (HUFFMAN_DECODE_LENGTH - i);

            switch (HUFFMAN_DECODE_LENGTH - i) {
            case 9:
                for (k = 0; k < (1 << 9); k += 4) {
                    Workspace->DecodeTable[TableLo] = DecodeValue;
                    Workspace->DecodeTable[TableLo+1] = DecodeValue;
                    Workspace->DecodeTable[TableLo+2] = DecodeValue;
                    Workspace->DecodeTable[TableLo+3] = DecodeValue;
                    TableLo += 4;
                }
                break;
            case 8:
                for (k = 0; k < (1 << 8); k += 4) {
                    Workspace->DecodeTable[TableLo] = DecodeValue;
                    Workspace->DecodeTable[TableLo+1] = DecodeValue;
                    Workspace->DecodeTable[TableLo+2] = DecodeValue;
                    Workspace->DecodeTable[TableLo+3] = DecodeValue;
                    TableLo += 4;
                }
                break;
            case 7:
                for (k = 0; k < (1 << 7); k += 4) {
                    Workspace->DecodeTable[TableLo] = DecodeValue;
                    Workspace->DecodeTable[TableLo+1] = DecodeValue;
                    Workspace->DecodeTable[TableLo+2] = DecodeValue;
                    Workspace->DecodeTable[TableLo+3] = DecodeValue;
                    TableLo += 4;
                }
                break;
            case 6:
                for (k = 0; k < (1 << 6); k += 4) {
                    Workspace->DecodeTable[TableLo] = DecodeValue;
                    Workspace->DecodeTable[TableLo+1] = DecodeValue;
                    Workspace->DecodeTable[TableLo+2] = DecodeValue;
                    Workspace->DecodeTable[TableLo+3] = DecodeValue;
                    TableLo += 4;
                }
                break;
            case 5:
                for (k = 0; k < (1 << 5); k += 4) {
                    Workspace->DecodeTable[TableLo] = DecodeValue;
                    Workspace->DecodeTable[TableLo+1] = DecodeValue;
                    Workspace->DecodeTable[TableLo+2] = DecodeValue;
                    Workspace->DecodeTable[TableLo+3] = DecodeValue;
                    TableLo += 4;
                }
                break;
            case 4:
                for (k = 0; k < (1 << 4); k += 4) {
                    Workspace->DecodeTable[TableLo] = DecodeValue;
                    Workspace->DecodeTable[TableLo+1] = DecodeValue;
                    Workspace->DecodeTable[TableLo+2] = DecodeValue;
                    Workspace->DecodeTable[TableLo+3] = DecodeValue;
                    TableLo += 4;
                }
                break;
            case 3:
                Workspace->DecodeTable[TableLo+7] = DecodeValue;
                Workspace->DecodeTable[TableLo+6] = DecodeValue;
                Workspace->DecodeTable[TableLo+5] = DecodeValue;
                Workspace->DecodeTable[TableLo+4] = DecodeValue;
            case 2:
                Workspace->DecodeTable[TableLo+3] = DecodeValue;
                Workspace->DecodeTable[TableLo+2] = DecodeValue;
            case 1:
                Workspace->DecodeTable[TableLo+1] = DecodeValue;
            case 0:
                Workspace->DecodeTable[TableLo] = DecodeValue;
                break;
            default:
                __assume(0);
            }

            //
            // We're walking the symbols in reverse because that was the
            // convenient way to build the tree.  So we need to decrement the
            // canonical code.  This may wrap.  If there are no more symbols
            // left, it is normal and expected.  If there are symbols left,
            // we'll return an error when we encounter them.
            //

            if (Code == 0) {
                FilledTable = TRUE;
            }

            --Code;
        }

        //
        // We're going from the longest bit length to the shortest, so we need
        // to half the canonical code.
        //

        Code /= 2;
    }

    if (FilledTable == FALSE) {

        //
        // If we didn't fill the decoding table, the Huffman table was invalid.
        //
        // There is one special case here: when only one symbol has a non-zero
        // bit length.  In this case, we don't know which symbol to decode for a
        // bit value of '1'.  We're going to handle this case by decoding a '1'
        // with the same symbol we used to decode '0'.  But we don't expect a
        // '1' to occur in the input.
        //

        for (i = 2; i <= MAX_ENCODING_LENGTH; ++i) {

            if (Workspace->LengthSortedListHead[i] != HUFFMAN_ALPHABET_SIZE) {

                //
                // There is a symbol with a bit length other than one.  This is
                // not the special case.
                //

                return STATUS_BAD_COMPRESSION_BUFFER;
            }
        }

        if (Workspace->LengthSortedListHead[1] == HUFFMAN_ALPHABET_SIZE) {

            //
            // There is no symbol with a bit length of one.
            //

            return STATUS_BAD_COMPRESSION_BUFFER;
        }

        for (i = 0; i < (1 << (HUFFMAN_DECODE_LENGTH - 1)); ++i) {

            //
            // We already filled in the second half of the table.  We need to
            // fill in the first half.
            //

            Workspace->DecodeTable[i] = DecodeValue;
        }
    }

    return STATUS_SUCCESS;
}



NTSTATUS
RtlDecompressBufferProgress (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkspacePvoid,
    IN PRTL_XPRESS_CALLBACK_FUNCTION Callback,
    IN PVOID CallbackContext,
    IN ULONG ProgressBytes
    )

/*++

Routine Description:

    This routine takes as input a compressed buffer and produces its
    uncompressed equivalent provided the uncompressed data fits within the
    specified destination buffer.

    An output variable indicates the number of bytes used to store the
    uncompressed data.

Arguments:

    UncompressedBuffer - Supplies a pointer to where the uncompressed data is to
        be stored.

    UncompressedBufferSize - Supplies the size, in bytes, of the uncompressed
        buffer.  This must be the size of the uncompressed data (the same number
        as the UncompressedBufferSize passed to RtlCompressBuffer for this
        buffer).

    CompressedBuffer - Supplies a pointer to the compressed data.

    CompressedBufferSize - Supplies the size, in bytes, of the compressed
        buffer.  This must be the size of the compressed data (as opposed to the
        size of some large buffer that contains some compressed data at the
        beginning): the exact number returned by RtlCompressBuffer in the
        FinalCompressedSize field.

    FinalUncompressedSize - Receives the number of bytes needed in the
        uncompressed buffer to store the uncompressed data.

    WorkspacePvoid - A buffer of the size specified by the
        CompressFragmentWorkSpaceSize parameter of
        RtlCompressWorkSpaceSizeXpressHuff.

    Callback - The function to call back periodically.

    CallbackContext - A PVOID that will be passed to the Callback function.

    ProgressBytes - The callback will be invoked each time this many bytes are
        decompressed.  (Note that this is the uncompressed size, not the
        compressed size).  So, if the caller wants the callback to be called
        about 7 times, it should specify ProgressBytes as
        UncompressedBufferSize/8.

Return Value:

    STATUS_SUCCESS - Success.

    STATUS_BAD_COMPRESSION_BUFFER - the input compressed buffer is ill-formed.

--*/

{
    NTSTATUS Status;
    PXPRESS_HUFF_DECODE_WORKSPACE Workspace;
    LONG_PTR CurrentShift;
    ULONG NextBits;
    PUCHAR InputPos;
    PUCHAR OutputPos;
    PUCHAR InputEnd;
    PUCHAR OutputEnd;
    PUCHAR MatchSrc;
    PUCHAR HuffBlockEnd;
    PUCHAR SafeHuffBlockEnd;
    PUCHAR ProgressOutputMark;
    ULONG_PTR TableIndex;
    SHORT DecodeValue;
    ULONG_PTR OffsetBitLength;
    ULONG_PTR MatchLen;
    ULONG_PTR MatchOffset;
    LONG_PTR DecodedBitCount;
    PVOID AlignedWorkspace;
    XPRESS_CALLBACK_PARAMS CallbackParams;

    //
    // Make sure our Workspace pointer is pointer-aligned.
    //

    if (WorkspacePvoid == NULL) {

        //
        // We can't check if the workspace is actually valid memory, but if the
        // caller used RtlDecompressBuffer instead of RtlDecompressBufferEx,
        // we'll return an error instead of having an AV.
        //

        return STATUS_INVALID_USER_BUFFER;
    }

    AlignedWorkspace = ALIGN_UP_POINTER(WorkspacePvoid, ULONG_PTR);

    Workspace = (PXPRESS_HUFF_DECODE_WORKSPACE)AlignedWorkspace;

    InputPos = CompressedBuffer;
    OutputPos = UncompressedBuffer;
    InputEnd = CompressedBuffer + CompressedBufferSize;
    OutputEnd = UncompressedBuffer + UncompressedBufferSize;

    if (Callback == NULL ||
        ProgressBytes > UncompressedBufferSize)
    {
        ProgressBytes = UncompressedBufferSize;
    }

    CallbackParams.Callback = Callback;
    CallbackParams.CallbackContext = CallbackContext;
    CallbackParams.ProgressBytes = ProgressBytes;

    for (;;) {

    HuffmanBlockDone:

        //
        // Does the buffer have enough remaining space to hold a Huffman table?
        //

        if (InputEnd - InputPos < HUFFMAN_ALPHABET_SIZE / 2 + 4) {

            //
            // This can happen for well-formed buffers if we haven't decoded the
            // EOF symbol yet.  If we're at the end of the output buffer, we'll
            // return success.  This doesn't verify that the EOF symbol exists,
            // like we usually do, but I think this is okay because could easily
            // return success for buffers that have actually been modified (for
            // example: if a bit of some match offset was changed, we'll copy
            // the wrong data without knowing it).
            //

            if (OutputPos == OutputEnd) {
                break;
            }

            return STATUS_BAD_COMPRESSION_BUFFER;
        }

        //
        // Build the Huffman decoding table and tree for this block.
        //

        Status = XpressBuildHuffmanDecodingTable(Workspace, InputPos);

        if (Status != STATUS_SUCCESS) {
            return STATUS_BAD_COMPRESSION_BUFFER;
        }

        CurrentShift = 16;
        InputPos += HUFFMAN_ALPHABET_SIZE / 2;
        NextBits = *((USHORT UNALIGNED *)InputPos);
        InputPos += sizeof(USHORT);
        NextBits <<= 16;
        NextBits += *((USHORT UNALIGNED *)InputPos);
        InputPos += sizeof(USHORT);

        //
        // The semantics of Huffman blocks are set up so that we can
        // conveniently (and efficiently) combine the "are we at the end of the
        // Huffman block?" checks with the "are we at the end of the buffer?"
        // checks.
        //

        HuffBlockEnd = OutputPos + HUFFMAN_BLOCK_SIZE;

        if (HuffBlockEnd > OutputEnd) {
            HuffBlockEnd = OutputEnd;
        }

        //
        // 11 * 17 comes from (11 bytes copied per match without bounds
        // checking) * (16 one-bit symbols per USHORT between bound checks).
        //

        SafeHuffBlockEnd = HuffBlockEnd - 11 * 17 - 1;

        if (OutputPos >= SafeHuffBlockEnd) {
            goto SafeDecode;
        }

        ProgressOutputMark = min(SafeHuffBlockEnd, OutputPos + ProgressBytes);

        for (;;) {

            for (;;) {

                //
                // Get the next 10 bits and look up their table entry.
                //

                TableIndex = NextBits >> (32 - HUFFMAN_DECODE_LENGTH);

                DecodeValue = Workspace->DecodeTable[TableIndex];

                if (DecodeValue <= 0) {

                    //
                    // This means the symbol is longer than 10 bits.  Decode the
                    // remainder using the tree.
                    //

                    NextBits <<= HUFFMAN_DECODE_LENGTH;
                    CurrentShift -= HUFFMAN_DECODE_LENGTH;

                    do {

                        assert(DecodeValue != 0);

                        DecodeValue = -DecodeValue;

                        //
                        // This could be: if (NextBits < 0) ++DecodeValue;
                        //
                        // But this is an unpredictable (hence, slow) branch.
                        //

                        DecodeValue += (NextBits >> 31);

                        NextBits *= 2;
                        --CurrentShift;

                        DecodeValue = Workspace->DecodeTree[DecodeValue];

                    } while (DecodeValue <= 0);

                } else {

                    //
                    // Shift the next bits according the actual length of the
                    // symbol.
                    //

                    DecodedBitCount = DecodeValue & 15;

                    NextBits <<= DecodedBitCount;
                    CurrentShift -= DecodedBitCount;
                }

                //
                // Shift down 4 to get the symbol value (the lower 4 bits are
                // the bit length of the symbol).
                //

                DecodeValue >>= 4;

                //
                // If this is a match, we'll want to subtract 256.  If it's not
                // a match, subtracting 256 doesn't change the value of
                // (UCHAR)DecodeValue.
                //

                DecodeValue -= 256;

                if (CurrentShift < 0) {

                    //
                    // Get the next 16 bits from the data stream.
                    //

                    if (OutputPos >= ProgressOutputMark) {

                        if (OutputPos >= SafeHuffBlockEnd) {
                            goto SafeDecodeEntry1;
                        }

                        ProgressOutputMark = RtlpMakeXpressCallback(&CallbackParams,
                                                                    SafeHuffBlockEnd,
                                                                    OutputPos);
                    }

                    if (InputPos + 1 >= InputEnd) {
                        return STATUS_BAD_COMPRESSION_BUFFER;
                    }

#if defined(_ARM_) || defined(_ARM64_)
                    __prefetch(InputPos + 12);
#endif

                    NextBits += ((ULONG)(*((USHORT UNALIGNED *)InputPos)))
                                << (-CurrentShift);
                    InputPos += sizeof(USHORT);
                    CurrentShift += 16;
                }

                if (DecodeValue >= 0) {

                    //
                    // This is a match.
                    //

                    if (DecodeValue == 0) {

                        //
                        // This could be the EOF if (OutputPos == OutputEnd).
                        // It could also be a match of length 3 and offset 1.
                        //

                        if (InputPos >= InputEnd && OutputPos == OutputEnd) {
                            goto DecodeDone;
                        }
                    }

                    break;
                }

                //
                // This is a literal.  Just copy the byte and continue to the
                // next symbol.
                //

                OutputPos[0] = (UCHAR)DecodeValue;
                ++OutputPos;
            }

            //
            // Extract the match length and the offset bit length.
            //

            OffsetBitLength = DecodeValue / LEN_MULT;
            MatchLen = DecodeValue % LEN_MULT;

            if (MatchLen == LEN_MULT - 1) {

                if (InputPos >= InputEnd) {
                    return STATUS_BAD_COMPRESSION_BUFFER;
                }

                MatchLen = InputPos[0];
                ++InputPos;

                if (MatchLen == 255) {

                    if (InputPos + 1 >= InputEnd) {
                        return STATUS_BAD_COMPRESSION_BUFFER;
                    }

                    MatchLen = *((USHORT UNALIGNED *)InputPos);
                    InputPos += sizeof(USHORT);

                    if (MatchLen == 0) {

                        //
                        // This is an extension of the classic Xpress format to
                        // support match lengths longer than (1 << 16).
                        //

                        if (InputPos + 3 >= InputEnd) {
                            return STATUS_BAD_COMPRESSION_BUFFER;
                        }

                        MatchLen = *((ULONG UNALIGNED *)InputPos);
                        InputPos += sizeof(ULONG);
                    }

                    if (MatchLen < LEN_MULT - 1 ||
                        OutputPos + MatchLen + 3 < OutputPos)
                    {
                        return STATUS_BAD_COMPRESSION_BUFFER;
                    }

                    MatchLen -= LEN_MULT - 1;
                }

                MatchLen += LEN_MULT - 1;
            }

            MatchLen += 3;

            //
            // Be careful about the OffsetBitLength == 0 case.  Shifting by 32
            // does nothing, while one might expect it to zero-out the value.
            //

            MatchOffset = (NextBits >> (31 - OffsetBitLength)) >> 1;
            MatchOffset += (((ULONG_PTR)1) << OffsetBitLength);

            NextBits <<= OffsetBitLength;
            CurrentShift -= OffsetBitLength;

            if (CurrentShift < 0) {

                if (OutputPos >= SafeHuffBlockEnd) {
                    goto SafeDecodeEntry2;
                }
                if (InputPos + 1 >= InputEnd) {
                    return STATUS_BAD_COMPRESSION_BUFFER;
                }

#if defined(_ARM_) || defined(_ARM64_)
                __prefetch(InputPos + 12);
#endif

                NextBits += ((ULONG)(*((USHORT UNALIGNED *)InputPos)))
                            << (-CurrentShift);
                InputPos += sizeof(USHORT);
                CurrentShift += 16;
            }

            MatchSrc = OutputPos - MatchOffset;

            if (MatchSrc < UncompressedBuffer) {
                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            if (MatchOffset < 4) {

                //
                // When the offset is less than four, copying four bytes at once
                // doesn't do what we want.  Fix this by copying the first part
                // of the match, then fall through to the normal case.
                //

                switch (MatchOffset) {

                case 1:
                    OutputPos[0] = MatchSrc[0];
                    OutputPos[1] = MatchSrc[0];
                    OutputPos[2] = MatchSrc[0];
                    MatchLen -= 3;
                    OutputPos += 3;
                    break;
                case 2:
                    OutputPos[0] = MatchSrc[0];
                    OutputPos[1] = MatchSrc[1];
                    MatchLen -= 2;
                    OutputPos += 2;
                    break;
                case 3:
                    OutputPos[0] = MatchSrc[0];
                    OutputPos[1] = MatchSrc[1];
                    OutputPos[2] = MatchSrc[2];
                    OutputPos += 3;
                    MatchLen -= 3;
                    break;
                default:
                    __assume(0);
                }

                if (MatchLen == 0) {
                    continue;
                }
            }

            for (;;) {

                //
                // Copy eight bytes then see if that was enough.  Note that the
                // safe output bound lets us copy short matches without checking
                // for the end of the output buffer.
                //

                *((ULONG UNALIGNED *)OutputPos) = *((ULONG UNALIGNED *)MatchSrc);
                *((ULONG UNALIGNED *)(OutputPos+4)) = *((ULONG UNALIGNED *)(MatchSrc+4));

                if (MatchLen < 9) {
                    OutputPos += MatchLen;
                    break;
                }

                OutputPos += 8;
                MatchSrc += 8;
                MatchLen -= 8;

                for (;;) {

                    //
                    // Now we must test the safe bound.
                    //

                    if (OutputPos >= ProgressOutputMark) {

                        if (OutputPos >= SafeHuffBlockEnd) {

                            if (OutputPos + MatchLen > OutputEnd) {
                                return STATUS_BAD_COMPRESSION_BUFFER;
                            }

                            memcpy(OutputPos, MatchSrc, MatchLen);
                            OutputPos += MatchLen;

                            goto SafeDecode;
                        }

                        ProgressOutputMark = RtlpMakeXpressCallback(&CallbackParams,
                                                                    SafeHuffBlockEnd,
                                                                    OutputPos);
                    }

#if defined(_ARM_) || defined(_ARM64_)
                    if (MatchLen > 32) {
                        __prefetch(MatchSrc + 32);
                    }
#endif
                    //
                    // Copy 16 bytes at a time so we can process long matches
                    // quickly.
                    //

                    *((ULONG UNALIGNED *)OutputPos) = *((ULONG UNALIGNED *)MatchSrc);
                    *((ULONG UNALIGNED *)(OutputPos+4)) = *((ULONG UNALIGNED *)(MatchSrc+4));
                    *((ULONG UNALIGNED *)(OutputPos+8)) = *((ULONG UNALIGNED *)(MatchSrc+8));
                    *((ULONG UNALIGNED *)(OutputPos+12)) = *((ULONG UNALIGNED *)(MatchSrc+12));

                    if (MatchLen < 17) {
                        OutputPos += MatchLen;
                        break;
                    }

                    OutputPos += 16;
                    MatchSrc += 16;
                    MatchLen -= 16;
                }

                break;
            }
        }

    SafeDecode:

        for (;;) {

            for (;;) {

                //
                // Are we done with this Huffman block?
                //

                if (OutputPos >= HuffBlockEnd) {
                    goto HuffmanBlockDone;
                }

                TableIndex = NextBits >> (32 - HUFFMAN_DECODE_LENGTH);

                DecodeValue = Workspace->DecodeTable[TableIndex];

                if (DecodeValue <= 0) {

                    NextBits <<= HUFFMAN_DECODE_LENGTH;
                    CurrentShift -= HUFFMAN_DECODE_LENGTH;

                    do {

                        assert(DecodeValue != 0);

                        DecodeValue = -DecodeValue + (NextBits >> 31);

                        NextBits *= 2;
                        --CurrentShift;

                        DecodeValue = Workspace->DecodeTree[DecodeValue];

                    } while (DecodeValue <= 0);

                } else {

                    DecodedBitCount = DecodeValue & 15;

                    NextBits <<= DecodedBitCount;
                    CurrentShift -= DecodedBitCount;
                }

                DecodeValue >>= 4;
                DecodeValue -= 256;

                if (CurrentShift < 0) {
                SafeDecodeEntry1:
                    if (InputPos + 1 >= InputEnd) {
                        return STATUS_BAD_COMPRESSION_BUFFER;
                    }
                    NextBits += ((ULONG)(*((USHORT UNALIGNED *)InputPos)))
                                << (-CurrentShift);
                    InputPos += sizeof(USHORT);
                    CurrentShift += 16;
                }

                if (DecodeValue >= 0) {
                    if (DecodeValue == 0) {
                        if (InputPos >= InputEnd && OutputPos == OutputEnd) {
                            goto DecodeDone;
                        }
                    }
                    break;
                }

                assert(OutputPos < OutputEnd);

                OutputPos[0] = (UCHAR)DecodeValue;
                ++OutputPos;
            }

            OffsetBitLength = DecodeValue / LEN_MULT;
            MatchLen = DecodeValue % LEN_MULT;

            if (MatchLen == LEN_MULT - 1) {

                if (InputPos >= InputEnd) {
                    return STATUS_BAD_COMPRESSION_BUFFER;
                }

                MatchLen = InputPos[0];
                ++InputPos;

                if (MatchLen == 255) {

                    if (InputPos + 1 >= InputEnd) {
                        return STATUS_BAD_COMPRESSION_BUFFER;
                    }

                    MatchLen = *((USHORT UNALIGNED *)InputPos);
                    InputPos += sizeof(USHORT);

                    if (MatchLen == 0) {

                        if (InputPos + 3 >= InputEnd) {
                            return STATUS_BAD_COMPRESSION_BUFFER;
                        }

                        MatchLen = *((ULONG UNALIGNED *)InputPos);
                        InputPos += sizeof(ULONG);
                    }

                    if (MatchLen < LEN_MULT - 1 ||
                        OutputPos + MatchLen + 3 < OutputPos)
                    {
                        return STATUS_BAD_COMPRESSION_BUFFER;
                    }

                    MatchLen -= LEN_MULT - 1;
                }

                MatchLen += LEN_MULT - 1;
            }

            MatchLen += 3;

            MatchOffset = (NextBits >> (31 - OffsetBitLength)) >> 1;
            MatchOffset += (((ULONG_PTR)1) << OffsetBitLength);

            NextBits <<= OffsetBitLength;
            CurrentShift -= OffsetBitLength;

            if (CurrentShift < 0) {
            SafeDecodeEntry2:
                if (InputPos + 1 >= InputEnd) {
                    return STATUS_BAD_COMPRESSION_BUFFER;
                }
                NextBits += ((ULONG)(*((USHORT UNALIGNED *)InputPos)))
                            << (-CurrentShift);
                InputPos += sizeof(USHORT);
                CurrentShift += 16;
            }

            MatchSrc = OutputPos - MatchOffset;

            if (MatchSrc < UncompressedBuffer) {
                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            if (OutputPos + MatchLen > OutputEnd) {
                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            //
            // The movsb instruction seems to be an efficient way to copy in a
            // precise boundary-aware way.
            //

            memcpy(OutputPos, MatchSrc, MatchLen);
            OutputPos += MatchLen;
        }
    }

DecodeDone:

    *FinalUncompressedSize = (ULONG)(OutputPos - UncompressedBuffer);

    return STATUS_SUCCESS;
}

