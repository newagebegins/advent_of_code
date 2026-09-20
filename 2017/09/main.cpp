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

enum token_type
{
    Token_Unknown,
    Token_BeginGroup,
    Token_EndGroup,
    Token_Garbage,
    Token_Comma,
    Token_EndOfStream,
};

struct token
{
    token_type Type;
    u32 GarbageCharCount;
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
    token Result = {};
    SkipWhitespace(Parser);
    switch(Parser->At[0])
    {
        case 0:
        {
            Result.Type = Token_EndOfStream;
        } break;

        case '{':
        {
            Result.Type = Token_BeginGroup;
            ++Parser->At;
        } break;

        case '}':
        {
            Result.Type = Token_EndGroup;
            ++Parser->At;
        } break;

        case ',':
        {
            Result.Type = Token_Comma;
            ++Parser->At;
        } break;

        case '<':
        {
            Result.Type = Token_Garbage;
            ++Parser->At;
            while(Parser->At[0] && (Parser->At[0] != '>'))
            {
                if(Parser->At[0] == '!')
                {
                    ++Parser->At;
                    Assert(Parser->At[0]);
                }
                else
                {
                    ++Result.GarbageCharCount;
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

struct parsed_result
{
    u32 Score;
    u32 GarbageCharCount;
};

internal parsed_result
ParseInput(char *Input)
{
    parsed_result Result = {};
    parser Parser;
    Parser.At = Input;
    b32 Finished = false;
    u32 Depth = 0;
    while(!Finished)
    {
        token Token = GetToken(&Parser);
        switch(Token.Type)
        {
            case Token_BeginGroup:
            {
                ++Depth;
            } break;

            case Token_EndGroup:
            {
                Result.Score += Depth;
                --Depth;
            } break;

            case Token_Garbage:
            {
                Result.GarbageCharCount += Token.GarbageCharCount;
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
TestParseInput(char *Input, u32 ExpectedScore, u32 ExpectedGarbageCharCount)
{
    parsed_result Result = ParseInput(Input);
    Assert(Result.Score == ExpectedScore);
    Assert(Result.GarbageCharCount == ExpectedGarbageCharCount);
}

int
main(void)
{
    TestParseInput("{}", 1, 0);
    TestParseInput("{{{}}}", 6, 0);
    TestParseInput("{{},{}}", 5, 0);
    TestParseInput("{{{},{},{{}}}}", 16, 0);
    TestParseInput("{<a>,<a>,<a>,<a>}", 1, 4);
    TestParseInput("{{<ab>},{<ab>},{<ab>},{<ab>}}", 9, 8);
    TestParseInput("{{<!!>},{<!!>},{<!!>},{<!!>}}", 9, 0);
    TestParseInput("{{<a!>},{<a!>},{<a!>},{<ab>}}", 3, 17);

    char *Input = ReadEntireFileAndNullTerminate("input.txt");
    TestParseInput(Input, 13154, 6369);

    return(0);
}
