#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t b32;

#define internal static
#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))
#define Assert(C) if(!(C)) {*(int *)0 = 0;}

internal void
Reverse(u32 ListSize, u32 *List, u32 StartPosition, u32 Length)
{
    for(u32 Offset = 0;
        Offset < (Length / 2);
        ++Offset)
    {
        u32 LeftIndex = (StartPosition + Offset) % ListSize;
        u32 RightIndex = (StartPosition + Length - Offset - 1) % ListSize;
        Assert(RightIndex >= 0);
        u32 Temp = List[LeftIndex];
        List[LeftIndex] = List[RightIndex];
        List[RightIndex] = Temp;
    }
}

internal void
FindHash(u32 ListSize, u32 *List, u32 LengthCount, u32 *Lengths)
{
    for(u32 ElementIndex = 0;
        ElementIndex < ListSize;
        ++ElementIndex)
    {
        List[ElementIndex] = ElementIndex;
    }

    u32 CurrentPosition = 0;
    u32 SkipSize = 0;

    for(u32 LengthIndex = 0;
        LengthIndex < LengthCount;
        ++LengthIndex)
    {
        u32 Length = Lengths[LengthIndex];
        Reverse(ListSize, List, CurrentPosition, Length);
        CurrentPosition = (CurrentPosition + (Length + SkipSize)) % ListSize;
        ++SkipSize;
    }
}

struct knot_hash_state
{
    u8 *List;
    u32 LengthCount;
    u8 *Lengths;
    u8 CurrentPosition;
    u8 SkipSize;
};

internal u32
PreprocessLengths(u32 MaxLengthCount, u8 *Lengths, char *Input)
{
    u32 LengthCount = 0;
    for(char *At = Input;
        *At;
        ++At)
    {
        Lengths[LengthCount++] = *At;
    }
    u8 Suffix[] = {17, 31, 73, 47, 23};
    for(u32 SuffixIndex = 0;
        SuffixIndex < ArrayCount(Suffix);
        ++SuffixIndex)
    {
        Lengths[LengthCount++] = Suffix[SuffixIndex];        
    }
    Assert(LengthCount <= MaxLengthCount);
    return(LengthCount);
}

inline void
ReverseU8(u8 *List, u8 StartPosition, u8 Length)
{
    for(u8 Offset = 0;
        Offset < (Length / 2);
        ++Offset)
    {
        u8 LeftIndex = StartPosition + Offset;
        u8 RightIndex = StartPosition + Length - Offset - 1;
        u8 Temp = List[LeftIndex];
        List[LeftIndex] = List[RightIndex];
        List[RightIndex] = Temp;
    }
}

inline void
DoKnotHashRound(knot_hash_state *State)
{
    for(u32 LengthIndex = 0;
        LengthIndex < State->LengthCount;
        ++LengthIndex)
    {
        u8 Length = State->Lengths[LengthIndex];
        ReverseU8(State->List, State->CurrentPosition, Length);
        State->CurrentPosition += (Length + State->SkipSize);
        ++State->SkipSize;
    }
}

internal void
SparseToDenseHash(u8 *Sparse, u8 *Dense)
{
    for(u32 DenseIndex = 0;
        DenseIndex < 16;
        ++DenseIndex)
    {
        u8 DenseValue = 0;
        u32 StartSparseIndex = 16*DenseIndex;
        u32 EndSparseIndex = StartSparseIndex + 16;
        for(u32 SparseIndex = StartSparseIndex;
            SparseIndex < EndSparseIndex;
            ++SparseIndex)
        {
            DenseValue ^= Sparse[SparseIndex];
        }
        Dense[DenseIndex] = DenseValue;
    }
}

inline char
NibbleToChar(u8 Nibble)
{
    char Result;
    if(Nibble < 0xa)
    {
        Result = '0' + Nibble;
    }
    else
    {
        Result = 'a' + (Nibble - 0xa);
    }
    return(Result);
}

internal void
ToHexString(u32 ByteCount, u8 *Bytes, char *Buffer)
{
    char *At = Buffer;
    for(u32 ByteIndex = 0;
        ByteIndex < ByteCount;
        ++ByteIndex)
    {
        u8 Byte = Bytes[ByteIndex];
        u8 HighNibble = (Byte >> 4);
        u8 LowNibble = (Byte & 0xf);
        *At++ = NibbleToChar(HighNibble);
        *At++ = NibbleToChar(LowNibble);
    }
    *At = 0;
    Assert((At - Buffer) == 32);
}

internal b32
StringsAreEqual(char *A, char *B)
{
    while(*A && *B && (*A == *B))
    {
        ++A;
        ++B;
    }
    b32 Result = (!*A && !*B);
    return(Result);
}

internal void
TestKnotHash(char *Input, char *Expected)
{
    u8 List[256];
    u8 Lengths[64];

    for(u32 ElementIndex = 0;
        ElementIndex < ArrayCount(List);
        ++ElementIndex)
    {
        List[ElementIndex] = (u8)ElementIndex;
    }

    knot_hash_state State = {};
    State.List = List;
    State.LengthCount = PreprocessLengths(ArrayCount(Lengths), Lengths, Input);
    State.Lengths = Lengths;
    for(u32 RoundIndex = 0;
        RoundIndex < 64;
        ++RoundIndex)
    {
        DoKnotHashRound(&State);
    }

    u8 DenseHash[16];
    SparseToDenseHash(List, DenseHash);
    char DenseHashString[33];
    ToHexString(ArrayCount(DenseHash), DenseHash, DenseHashString);
    Assert(StringsAreEqual(DenseHashString, Expected));
}

int
main(void)
{
    // NOTE(slava): Part 1

    {
        u32 List[5];
        u32 Lengths[] = {3,4,1,5};
        FindHash(ArrayCount(List), List, ArrayCount(Lengths), Lengths);
        Assert((List[0] * List[1]) == 12);
    }

    {
        u32 List[256];
        u32 Lengths[] = {14,58,0,116,179,16,1,104,2,254,167,86,255,55,122,244};
        FindHash(ArrayCount(List), List, ArrayCount(Lengths), Lengths);
        Assert((List[0] * List[1]) == 1935);
    }

    // NOTE(slava): Part 2

    TestKnotHash("", "a2582a3a0e66e6e86e3812dcb672a272");
    TestKnotHash("AoC 2017", "33efeb34ea91902bb2f59c9920caa6cd");
    TestKnotHash("1,2,3", "3efbe78a8d82f29979031a4aa0b16a9d");
    TestKnotHash("1,2,4", "63960835bcdc130f0b66d7ff4f6a5a8e");
    TestKnotHash("14,58,0,116,179,16,1,104,2,254,167,86,255,55,122,244", "dc7e7dee710d4c7201ce42713e6b8359");

    return(0);
}
