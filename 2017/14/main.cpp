#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t s32;
typedef int32_t b32;

#define internal static
#define local_persist static
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

struct grid
{
    u8 Squares[128][128];
};

internal void
FillGrid(char *KeyString, grid *Grid)
{
    char StringToHash[32];
    u8 DenseHash[16];
    string_builder Builder;
    for(u32 Row = 0;
        Row < 128;
        ++Row)
    {
        InitializeStringBuilder(&Builder, sizeof(StringToHash), StringToHash);
        AppendString(&Builder, KeyString);
        AppendChar(&Builder, '-');
        AppendU32(&Builder, Row);
        NullTerminate(&Builder);
        KnotHash(StringToHash, DenseHash);
        u32 Col = 0;
        for(u32 ByteIndex = 0;
            ByteIndex < ArrayCount(DenseHash);
            ++ByteIndex)
        {
            u8 Byte = DenseHash[ByteIndex];
            for(s32 Shift = 7;
                Shift >= 0;
                --Shift, ++Col)
            {
                Grid->Squares[Row][Col] = ((Byte >> Shift) & 1);
            }
        }
    }
}

struct v2i
{
    s32 x, y;
};

internal void
VisitRegion(grid *Grid, grid *Visited, u32 StartRow, u32 StartCol)
{
    local_persist v2i ToVisit[128];
    ToVisit[0] = {(s32)StartCol, (s32)StartRow};
    u32 ToVisitCount = 1;
    v2i Directions[] =
    {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    };
    while(ToVisitCount)
    {
        --ToVisitCount;
        s32 Row = ToVisit[ToVisitCount].y;
        s32 Col = ToVisit[ToVisitCount].x;
        if(!Visited->Squares[Row][Col])
        {
            Visited->Squares[Row][Col] = true;
            for(u32 DirectionIndex = 0;
                DirectionIndex < ArrayCount(Directions);
                ++DirectionIndex)
            {
                v2i Direction = Directions[DirectionIndex];
                s32 NewRow = Row + Direction.y;
                s32 NewCol = Col + Direction.x;
                if((NewRow >= 0) && (NewRow < 128) &&
                   (NewCol >= 0) && (NewCol < 128) &&
                   !Visited->Squares[NewRow][NewCol] &&
                   Grid->Squares[NewRow][NewCol])
                {
                    Assert(ToVisitCount < ArrayCount(ToVisit));
                    ToVisit[ToVisitCount++] = {NewCol, NewRow};
                }
            }
        }
    }
}

internal u32
CountRegions(grid *Grid)
{
    u32 Result = 0;
    local_persist grid Visited;
    for(u32 Row = 0;
        Row < 128;
        ++Row)
    {
        for(u32 Col = 0;
            Col < 128;
            ++Col)
        {
            Visited.Squares[Row][Col] = false;
        }
    }
    for(u32 Row = 0;
        Row < 128;
        ++Row)
    {
        for(u32 Col = 0;
            Col < 128;
            ++Col)
        {
            if(!Visited.Squares[Row][Col] && Grid->Squares[Row][Col])
            {
                ++Result;
                VisitRegion(Grid, &Visited, Row, Col);
            }
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

internal void
TestCountRegions(char *KeyString, u32 ExpectedCount)
{
    local_persist grid Grid;
    FillGrid(KeyString, &Grid);
    u32 Count = CountRegions(&Grid);
    Assert(Count == ExpectedCount);
}

int
main(void)
{
    char *TestInput = "flqrgnkx";
    char *PuzzleInput = "ffayrhll";

    // NOTE(slava): Part 1

    TestCountUsedSquares(TestInput, 8108);
    TestCountUsedSquares(PuzzleInput, 8190);

    // NOTE(slava): Part 2

    TestCountRegions(TestInput, 1242);
    TestCountRegions(PuzzleInput, 1134);

    return(0);
}
