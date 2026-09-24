#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define internal static

#define Assert(C) if(!(C)) { *(int *)0 = 0; }
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); } break

typedef int32_t s32;
typedef int32_t b32;
typedef uint32_t u32;

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

struct v3i
{
    s32 x, y, z;
};

inline b32
operator==(v3i A, v3i B)
{
    b32 Result = ((A.x == B.x) &&
                  (A.y == B.y) &&
                  (A.z == B.z));
    return(Result);
}

inline u32
AbsoluteValue(s32 X)
{
    u32 Result = ((X < 0) ? -X : X);
    return(Result);
}

/**
   NOTE(slava):

   +x is south-east, -x is north-west
   +y is north-east, -y is south-west
   +z is north, -z is south

   x = y - z
   y = x + z
   z = y - x
 */

inline v3i
ToFewestSteps(v3i P)
{
    while((P.x > 0) && (P.z > 0))
    {
        --P.x;
        --P.z;
        ++P.y;
    }
    while((P.x < 0) && (P.z < 0))
    {
        ++P.x;
        ++P.z;
        --P.y;
    }
    while((P.y > 0) && (P.z < 0))
    {
        --P.y;
        ++P.z;
        ++P.x;
    }
    while((P.y < 0) && (P.z > 0))
    {
        ++P.y;
        --P.z;
        --P.x;
    }
    while((P.y > 0) && (P.x < 0))
    {
        --P.y;
        ++P.x;
        ++P.z;
    }
    while((P.y < 0) && (P.x > 0))
    {
        ++P.y;
        --P.x;
        --P.z;
    }
    return(P);
}

inline u32
GetDistanceFromOrigin(v3i P)
{
    u32 Result = AbsoluteValue(P.x) + AbsoluteValue(P.y) + AbsoluteValue(P.z);
    return(Result);
}

struct parser
{
    char *At;
};

enum token
{
    Token_Unknown,
    Token_Comma,
    Token_SouthEast,
    Token_NorthEast,
    Token_North,
    Token_NorthWest,
    Token_SouthWest,
    Token_South,
    Token_EndOfFile,
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
            Result = Token_EndOfFile;
        } break;

        case ',':
        {
            Result = Token_Comma;
            ++Parser->At;
        } break;

        case 's':
        {
            switch(Parser->At[1])
            {
                case 'e':
                {
                    Result = Token_SouthEast;
                    Parser->At += 2;
                } break;

                case 'w':
                {
                    Result = Token_SouthWest;
                    Parser->At += 2;
                } break;

                default:
                {
                    Result = Token_South;
                    ++Parser->At;
                } break;
            }
        } break;

        case 'n':
        {
            switch(Parser->At[1])
            {
                case 'e':
                {
                    Result = Token_NorthEast;
                    Parser->At += 2;
                } break;

                case 'w':
                {
                    Result = Token_NorthWest;
                    Parser->At += 2;
                } break;

                default:
                {
                    Result = Token_North;
                    ++Parser->At;
                } break;
            }
        } break;

        InvalidDefaultCase;
    }
    return(Result);
}

struct parsed_path
{
    u32 FinalDistance;
    u32 FurthestDistance;
};

internal parsed_path
ParsePath(char *Path)
{
    parsed_path Result = {};
    parser Parser;
    Parser.At = Path;
    token Token;
    v3i P = {};
    while((Token = GetToken(&Parser)) != Token_EndOfFile)
    {
        switch(Token)
        {
            case Token_SouthEast:
            {
                ++P.x;
            } break;

            case Token_NorthEast:
            {
                ++P.y;
            } break;

            case Token_North:
            {
                ++P.z;
            } break;

            case Token_NorthWest:
            {
                --P.x;
            } break;

            case Token_SouthWest:
            {
                --P.y;
            } break;

            case Token_South:
            {
                --P.z;
            } break;

            case Token_Comma:
            {
                // NOTE(slava): Do nothing
            } break;

            InvalidDefaultCase;
        }

        P = ToFewestSteps(P);
        Result.FinalDistance = GetDistanceFromOrigin(P);
        if(Result.FurthestDistance < Result.FinalDistance)
        {
            Result.FurthestDistance = Result.FinalDistance;
        }
    }
    return(Result);
}

internal void
Test(char *Path, u32 ExpectedFinalDistance, u32 ExpectedFurthestDistance)
{
    parsed_path ParsedPath = ParsePath(Path);
    Assert(ParsedPath.FinalDistance == ExpectedFinalDistance);
    Assert(ParsedPath.FurthestDistance == ExpectedFurthestDistance);
}

int
main(void)
{
    Test("ne,ne,ne", 3, 3);
    Test("ne,ne,sw,sw", 0, 2);
    Test("ne,ne,s,s", 2, 2);
    Test("se,sw,se,sw,sw", 3, 3);

    char *Path = ReadEntireFileAndNullTerminate("input.txt");
    Test(Path, 808, 1556);

    return(0);
}
