#if !defined(LIB_H)
#define LIB_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef int32_t b32;

typedef size_t memory_index;

#define internal static
#define local_persist static

#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))

#define Assert(C) if(!(C)) { *(int *)0 = 0; }
#define InvalidCodePath Assert(!"Invalid code path")
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); }

#define Kilobytes(N) (N)*1024
#define Megabytes(N) Kilobytes(N)*1024
#define Gigabytes(N) Megabytes(N)*1024

//
//
//

struct memory_arena
{
    memory_index Used;
    memory_index Size;
    void *Base;
    u32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    memory_index OldUsed;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Used = 0;
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->TempCount = 0;
}

#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, Count*sizeof(type))
#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))

inline void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = (u8 *)Arena->Base + Arena->Used;
    Arena->Used += Size;
    return(Result);
}

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

//
//
//

struct str
{
    u32 Length;
    char *Chars;
};

struct parser
{
    char *At;
};

inline parser
MakeParser(char *Input)
{
    parser Result;
    Result.At = Input;
    return(Result);
}

inline b32
StillParsing(parser *Parser)
{
    b32 Result = (Parser->At[0] != 0);
    return(Result);
}

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') ||
                  (C == '\t') ||
                  (C == '\r') ||
                  (C == '\n'));
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

inline void
SkipRestOfLine(parser *Parser)
{
    while(Parser->At[0] && (Parser->At[0] != '\n'))
    {
        ++Parser->At;
    }
    if(Parser->At[0] == '\n')
    {
        ++Parser->At;
    }
}

inline void
SkipChar(parser *Parser, char C)
{
    Assert(Parser->At[0] == C);
    ++Parser->At;
}

inline str
ParseWord(parser *Parser)
{
    str Result;
    Result.Chars = Parser->At;
    while(Parser->At[0] && !IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
    Result.Length = (u32)(Parser->At - Result.Chars);
    return(Result);
}

inline u32
ParseU32(parser *Parser)
{
    u32 Result = 0;
    while(IsDigit(Parser->At[0]))
    {
        Result = 10*Result + (Parser->At[0] - '0');
        ++Parser->At;
    }
    return(Result);
}

inline u32
CountLines(char *Text)
{
    u32 Result = 0;
    for(char *At = Text;
        *At;
        ++At)
    {
        if(*At == '\n')
        {
            ++Result;
        }
    }
    return(Result);
}

inline b32
StringsAreEqual(str A, char *B)
{
    u32 AIndex = 0;
    while((AIndex != A.Length) && *B && (A.Chars[AIndex] == *B))
    {
        ++AIndex;
        ++B;
    }
    b32 Result = ((AIndex == A.Length) && !*B);
    return(Result);
}

internal char *
ReadEntireFileAndNullTerminate(char *FileName)
{
    char *Result = 0;
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        if(fseek(File, 0, SEEK_END) == 0)
        {
            long FileSize = ftell(File);
            if(FileSize > 0)
            {
                if(fseek(File, 0, SEEK_SET) == 0)
                {
                    Result = (char *)malloc((FileSize + 1)*sizeof(char));
                    if(Result)
                    {
                        size_t ReadCount = fread(Result, FileSize, 1, File);
                        if(ReadCount == 1)
                        {
                            Result[FileSize] = 0;
                        }
                        else
                        {
                            free(Result);
                            Result = 0;
                        }
                    }
                }
            }
        }
    }
    return(Result);
}

#endif
