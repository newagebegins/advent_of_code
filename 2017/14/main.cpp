#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t s32;
typedef int32_t b32;

#define internal static
#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))
#define Assert(C) if(!(C)) {*(int *)0 = 0;}

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

internal void
KnotHash(char *Input, u8 *DenseHash)
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

    SparseToDenseHash(List, DenseHash);
}

struct string_builder
{
    u32 AtIndex;
    u32 BufferSize;
    char *Buffer;
};

inline void
InitializeStringBuilder(string_builder *Builder, u32 BufferSize, char *Buffer)
{
    Builder->AtIndex = 0;
    Builder->BufferSize = BufferSize;
    Builder->Buffer = Buffer;
}

inline void
AppendChar(string_builder *Builder, char Char)
{
    Assert(Builder->AtIndex < Builder->BufferSize);
    Builder->Buffer[Builder->AtIndex++] = Char;
}

inline void
AppendString(string_builder *Builder, char *String)
{
    for(char *At = String;
        *At;
        ++At)
    {
        AppendChar(Builder, *At);
    }
}

inline void
AppendU32(string_builder *Builder, u32 Number)
{
    char ReverseText[16];
    u32 DigitCount = 0;
    if(Number)
    {
        while(Number)
        {
            ReverseText[DigitCount++] = '0' + (Number % 10);
            Number /= 10;
        }
    }
    else
    {
        ReverseText[DigitCount++] = '0';
    }
    for(s32 ReverseIndex = DigitCount - 1;
        ReverseIndex >= 0;
        --ReverseIndex)
    {
        AppendChar(Builder, ReverseText[ReverseIndex]);
    }
}

inline void
NullTerminate(string_builder *Builder)
{
    AppendChar(Builder, 0);
}

inline u32
CountOnes(u8 Byte)
{
    u32 Result = 0;
    for(u32 Shift = 0;
        Shift < 8;
        ++Shift)
    {
        Result += ((Byte >> Shift) & 1);
    }
    return(Result);
}

internal u32
CountUsedSquares(char *KeyString)
{
    u32 Result = 0;
    char StringToHash[32];
    u8 DenseHash[16];
    string_builder Builder;
    for(u32 RowIndex = 0;
        RowIndex < 128;
        ++RowIndex)
    {
        InitializeStringBuilder(&Builder, sizeof(StringToHash), StringToHash);
        AppendString(&Builder, KeyString);
        AppendChar(&Builder, '-');
        AppendU32(&Builder, RowIndex);
        NullTerminate(&Builder);
        KnotHash(StringToHash, DenseHash);
        for(u32 ByteIndex = 0;
            ByteIndex < ArrayCount(DenseHash);
            ++ByteIndex)
        {
            Result += CountOnes(DenseHash[ByteIndex]);
        }
    }
    return(Result);
}

internal void
TestCountUsedSquares(char *KeyString, u32 ExpectedCount)
{
    u32 Count = CountUsedSquares(KeyString);
    Assert(Count == ExpectedCount);
}

int
main(void)
{
    TestCountUsedSquares("flqrgnkx", 8108);
    TestCountUsedSquares("ffayrhll", 8190);

    return(0);
}
