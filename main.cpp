#include <stdint.h>
#include <stdio.h>

#define global_variable static
#define internal static
#define local_persist static

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef int32_t b32;

typedef float r32;
typedef double r64;

typedef size_t memory_index;

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

//
//
//

inline u32
StringLength(char *Str)
{
    char *At = Str;
    while(*At)
    {
        ++At;
    }
    u32 Count = (u32)(At - Str);
    return(Count);
}

inline b32
StringsAreEqual(char *A, char *B)
{
    b32 Result = (A == B);
    if(A && B)
    {
        while(*A && *B && (*A == *B))
        {
            ++A;
            ++B;
        }
        Result = ((*A == 0) && (*B == 0));
    }
    return(Result);
}

//
//
//

typedef u64 chunk;

#define BITS_PER_CHUNK (8*sizeof(chunk))
#define EFFECTIVE_TILES_PER_CHUNK (BITS_PER_CHUNK - 2)
#define MAX_CHUNK_COUNT 4
#define MAX_TILE_COUNT (MAX_CHUNK_COUNT*EFFECTIVE_TILES_PER_CHUNK)

struct row
{
    chunk Chunks[MAX_CHUNK_COUNT];
    u32 TileCount;
};

// 012345678901234567 Abs
// 123456123456123456 Rel
// 0     1     2      Chunk

// 76543210

// 012
// LCR
// 110 -> 011
// 011 -> 110
// 100 -> 100
// 001 -> 001

internal row
StringToRow(char *Str)
{
    row Result = {};
    u32 TilesPerRow = StringLength(Str);
    Result.TileCount = TilesPerRow;

    u32 ChunkCount = 1 + ((TilesPerRow - 1) / EFFECTIVE_TILES_PER_CHUNK);
    Assert(ChunkCount <= MAX_CHUNK_COUNT);

    for(u32 AbsTileIndex = 0;
        AbsTileIndex < TilesPerRow;
        ++AbsTileIndex)
    {
        u32 RelTileIndex = 1 + (AbsTileIndex % EFFECTIVE_TILES_PER_CHUNK);
        u32 ChunkIndex = AbsTileIndex / EFFECTIVE_TILES_PER_CHUNK;
        if(Str[AbsTileIndex] == '^')
        {
            Result.Chunks[ChunkIndex] |= (1ULL << RelTileIndex);
        }
    }

    for(u32 ChunkIndex = 0;
        ChunkIndex < ChunkCount;
        ++ChunkIndex)
    {
        if(ChunkIndex > 0)
        {
            chunk TileVal = ((Result.Chunks[ChunkIndex - 1] >> (BITS_PER_CHUNK - 2)) & 1);
            Result.Chunks[ChunkIndex] |= TileVal;
        }
        if((ChunkIndex + 1) < ChunkCount)
        {
            chunk TileVal = ((Result.Chunks[ChunkIndex + 1] >> 1) & 1);            
            Result.Chunks[ChunkIndex] |= (TileVal << (BITS_PER_CHUNK - 1));
        }
    }

    return(Result);
}

internal void
RowToString(row Row, char *Dest, u32 DestSize)
{
    Assert(DestSize > Row.TileCount);

    for(u32 AbsTileIndex = 0;
        AbsTileIndex < Row.TileCount;
        ++AbsTileIndex)
    {
        u32 RelTileIndex = 1 + (AbsTileIndex % EFFECTIVE_TILES_PER_CHUNK);
        u32 ChunkIndex = AbsTileIndex / EFFECTIVE_TILES_PER_CHUNK;
        b32 IsTrap = ((Row.Chunks[ChunkIndex] >> RelTileIndex) & 1);
        char Char = (IsTrap ? '^' : '.');
        Dest[AbsTileIndex] = Char;
    }

    Dest[Row.TileCount] = 0;
}

internal u32
CountSafeTilesInRow(row Row)
{
    u32 Result = 0;
    for(u32 AbsTileIndex = 0;
        AbsTileIndex < Row.TileCount;
        ++AbsTileIndex)
    {
        u32 RelTileIndex = 1 + (AbsTileIndex % EFFECTIVE_TILES_PER_CHUNK);
        u32 ChunkIndex = AbsTileIndex / EFFECTIVE_TILES_PER_CHUNK;
        b32 IsTrap = ((Row.Chunks[ChunkIndex] >> RelTileIndex) & 1);
        if(!IsTrap)
        {
            ++Result;
        }
    }
    return(Result);
}

internal row
CalculateNewRow(row PrevRow)
{
    row Result = {};
    Result.TileCount = PrevRow.TileCount;
    u32 TilesPerRow = PrevRow.TileCount;

    u32 ChunkCount = 1 + ((TilesPerRow - 1) / EFFECTIVE_TILES_PER_CHUNK);
    Assert(ChunkCount <= MAX_CHUNK_COUNT);

    for(u32 AbsTileIndex = 0;
        AbsTileIndex < TilesPerRow;
        ++AbsTileIndex)
    {
        u32 RelTileIndex = 1 + (AbsTileIndex % EFFECTIVE_TILES_PER_CHUNK);
        u32 ChunkIndex = AbsTileIndex / EFFECTIVE_TILES_PER_CHUNK;
        chunk PrevChunk = PrevRow.Chunks[ChunkIndex];
        chunk TilesToCheck = ((PrevChunk >> (RelTileIndex - 1)) & 7);
        if((TilesToCheck == 1) ||
           (TilesToCheck == 3) ||
           (TilesToCheck == 4) ||
           (TilesToCheck == 6))
        {
            Result.Chunks[ChunkIndex] |= (1ULL << RelTileIndex);
        }
    }

    for(u32 ChunkIndex = 0;
        ChunkIndex < ChunkCount;
        ++ChunkIndex)
    {
        if(ChunkIndex > 0)
        {
            chunk TileVal = ((Result.Chunks[ChunkIndex - 1] >> (BITS_PER_CHUNK - 2)) & 1);
            Result.Chunks[ChunkIndex] |= TileVal;
        }
        if((ChunkIndex + 1) < ChunkCount)
        {
            chunk TileVal = ((Result.Chunks[ChunkIndex + 1] >> 1) & 1);            
            Result.Chunks[ChunkIndex] |= (TileVal << (BITS_PER_CHUNK - 1));
        }
    }

    return(Result);
}

#if 0
internal void
RowsTestCase(char *Rows)
{
    char Buffer[2][MAX_TILE_COUNT + 1];
    char *At = Rows;
    At = SkipWhitespace(At);

    At += GetLine(At, Buffer[0], sizeof(Buffer[0]));
    row FirstRow = StringToRow(Buffer[0]);
    RowToString(FirstRow, Buffer[1], sizeof(Buffer[1]));
    Assert(StringsAreEqual(Buffer[0], Buffer[1]));

    At = SkipWhitespace(At);
    while(*At)
    {
        At += GetLine(At, Buffer[0], sizeof(Buffer[0]));
        row Row = CalculateNewRow(PrevRow);
        RowToString(Row, Buffer[1], sizeof(Buffer[1]));        
        Assert(StringsAreEqual(Buffer[0], Buffer[1]));
    }
}
#endif

internal void
TestCase(char *FirstRow, u32 TotalRows)
{
    char Buffer[MAX_TILE_COUNT + 1];
    printf("%s\n", FirstRow);
    row PrevRow = StringToRow(FirstRow);
    for(u32 RowIndex = 1;
        RowIndex < TotalRows;
        ++RowIndex)
    {
        row Row = CalculateNewRow(PrevRow);
        RowToString(Row, Buffer, sizeof(Buffer));
        printf("%s\n", Buffer);
        PrevRow = Row;
    }
}

internal u32
CountTotalSafeTiles(char *FirstRow, u32 TotalRows)
{
    row PrevRow = StringToRow(FirstRow);
    u32 Result = CountSafeTilesInRow(PrevRow);
    for(u32 RowIndex = 1;
        RowIndex < TotalRows;
        ++RowIndex)
    {
        row Row = CalculateNewRow(PrevRow);
        Result += CountSafeTilesInRow(Row);
        PrevRow = Row;
    }
    return(Result);
}

int main(void)
{
    TestCase("..^^.", 3);
    printf("\n");
    TestCase(".^^.^.^^^^", 10);

    Assert(CountTotalSafeTiles(".^^.^.^^^^", 10) == 38);

    char *Input = "......^.^^.....^^^^^^^^^...^.^..^^.^^^..^.^..^.^^^.^^^^..^^.^.^.....^^^^^..^..^^^..^^.^.^..^^..^^^..";
    u32 Count = CountTotalSafeTiles(Input, 40);
    Assert(Count == 1963);

    u32 Count2 = CountTotalSafeTiles(Input, 400000);
    Assert(Count2 == 20009568);

    return(0);
}
