#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define internal static

typedef uint32_t u32;
typedef int32_t b32;

#define Assert(C) if(!(C)) {*(int *)0 = 0;}
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); } break;

internal char *
ReadEntireFileAndNullTerminate(char *Filename)
{
    FILE *File = fopen(Filename, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);
    Assert(FileSize > 0);
    char *Result = (char *)malloc(FileSize + 1);
    Assert(Result);
    fread(Result, FileSize, 1, File);
    Result[FileSize] = 0;
    fclose(File);
    return(Result);
}

enum token
{
    Token_Unknown,
    Token_BeginGroup,
    Token_EndGroup,
    Token_Garbage,
    Token_Comma,
    Token_EndOfStream,
};

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

inline void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

internal token
GetToken(parser *Parser)
{
    token Result = Token_Unknown;
    SkipWhitespace(Parser);
    switch(Parser->At[0])
    {
        case 0:
        {
            Result = Token_EndOfStream;
        } break;

        case '{':
        {
            Result = Token_BeginGroup;
            ++Parser->At;
        } break;

        case '}':
        {
            Result = Token_EndGroup;
            ++Parser->At;
        } break;

        case ',':
        {
            Result = Token_Comma;
            ++Parser->At;
        } break;

        case '<':
        {
            Result = Token_Garbage;
            ++Parser->At;
            while(Parser->At[0] && (Parser->At[0] != '>'))
            {
                if(Parser->At[0] == '!')
                {
                    ++Parser->At;
                    Assert(Parser->At[0]);
                }
                ++Parser->At;
            }
            if(Parser->At[0])
            {
                ++Parser->At;
            }
        } break;

        InvalidDefaultCase;
    }
    return(Result);
}

internal u32
FindScore(char *Input)
{
    u32 Result = 0;
    parser Parser;
    Parser.At = Input;
    b32 Finished = false;
    u32 Depth = 0;
    while(!Finished)
    {
        token Token = GetToken(&Parser);
        switch(Token)
        {
            case Token_BeginGroup:
            {
                ++Depth;
            } break;

            case Token_EndGroup:
            {
                Result += Depth;
                --Depth;
            } break;

            case Token_Garbage:
            {
                // NOTE(slava): Nothing to do
            } break;

            case Token_Comma:
            {
                // NOTE(slava): Nothing to do
            } break;

            case Token_EndOfStream:
            {
                Finished = true;
            } break;

            InvalidDefaultCase;
        }
    }
    return(Result);
}

internal void
TestFindScore(char *Input, u32 ExpectedScore)
{
    u32 Score = FindScore(Input);
    Assert(Score == ExpectedScore);
}

int
main(void)
{
    TestFindScore("{}", 1);
    TestFindScore("{{{}}}", 6);
    TestFindScore("{{},{}}", 5);
    TestFindScore("{{{},{},{{}}}}", 16);
    TestFindScore("{<a>,<a>,<a>,<a>}", 1);
    TestFindScore("{{<ab>},{<ab>},{<ab>},{<ab>}}", 9);
    TestFindScore("{{<!!>},{<!!>},{<!!>},{<!!>}}", 9);
    TestFindScore("{{<a!>},{<a!>},{<a!>},{<ab>}}", 3);

    char *Input = ReadEntireFileAndNullTerminate("input.txt");
    TestFindScore(Input, 13154);

    return(0);
}
