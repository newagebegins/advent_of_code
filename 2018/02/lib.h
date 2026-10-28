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

#define internal static

#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))
#define Assert(C) if(!(C)) { *(int *)0 = 0; }

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

inline void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
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
