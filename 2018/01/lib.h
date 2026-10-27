#if !defined(LIB_H)
#define LIB_H

#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef int32_t b32;

typedef size_t memory_index;

#define internal static
#define local_persist static

#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))

#define Assert(C) if(!(C)) { *(int *)0 = 0; }
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); } break

struct memory_arena
{
    memory_index Used;
    memory_index Size;
    void *Base;
    u32 TempCount;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Used = 0;
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->TempCount = 0;
}

struct temporary_memory
{
    memory_arena *Arena;
    memory_index OldUsed;
};

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    Result.Arena = Arena;
    Result.OldUsed = Arena->Used;
    ++Arena->TempCount;
    return(Result);
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
    TempMem.Arena->Used = TempMem.OldUsed;
    --TempMem.Arena->TempCount;
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, Count*sizeof(type))

inline void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = (u8 *)Arena->Base + Arena->Used;
    Arena->Used += Size;
    return(Result);
}

internal char *
ReadEntireFileAndNullTerminate(memory_arena *Arena, char *InputFile)
{
    char *Result = 0;
    FILE *File = fopen(InputFile, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);
    Result = PushArray(Arena, FileSize + 1, char);
    size_t ReadItems = fread(Result, FileSize, 1, File);
    Assert(ReadItems == 1);
    Result[FileSize] = 0;
    return(Result);
}

struct parser
{
    char *At;
};

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') || (C == '\t') || (C == '\r') || (C == '\n'));
    return(Result);
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

inline void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

internal s32
ParseS32(parser *Parser)
{
    s32 Result = 0;
    s32 Sign = 1;

    if(Parser->At[0] == '+')
    {
        ++Parser->At;
    }
    else if(Parser->At[0] == '-')
    {
        ++Parser->At;
        Sign = -1;
    }

    while(IsDigit(Parser->At[0]))
    {
        Result = 10*Result + (Parser->At[0] - '0');
        ++Parser->At;
    }

    Result *= Sign;
    return(Result);
}

#endif
