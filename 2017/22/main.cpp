#include <stdint.h>
#include <stdio.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef int32_t b32;

#define internal static
#define local_persist static

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#define Assert(Condition) if(!(Condition)) { *(int *)0 = 0; }
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); } break

internal void
ReadEntireFileAndNullTerminate(char *FileName, u32 BufferSize, char *Buffer)
{
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    size_t ReadCount = fread(Buffer, 1, BufferSize - 1, File);
    Assert(ReadCount < BufferSize);
    Buffer[ReadCount] = 0;
    fclose(File);
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') || (C == '\t') || (C == '\r') || (C == '\n'));
    return(Result);
}

internal u32
ParseU32(char *String)
{
    u32 Result = 0;
    while(IsDigit(*String))
    {
        Result = 10*Result + (*String - '0');
        ++String;
    }
    return(Result);
}

#define GRID_WIDTH 1024
#define GRID_HEIGHT 1024

struct grid
{
    u8 Nodes[GRID_HEIGHT][GRID_WIDTH];
};

struct parser
{
    char *At;
};

inline void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

internal u32
CountCharsInLine(char *Line)
{
    u32 Result = 0;
    char *At = Line;
    for(;;)
    {
        char C = *At;
        if(C && (C != '\r') && (C != '\n'))
        {
            ++Result;
            ++At;
        }
        else
        {
            break;
        }
    }
    return(Result);
}

internal u32
CountNonEmptyLines(char *Text)
{
    u32 Result = 0;
    char *At = Text;
    while(*At)
    {
        if(IsWhitespace(*At))
        {
            ++At;
        }
        else
        {
            ++Result;
            while(*At && (*At != '\n'))
            {
                ++At;
            }
            if(*At)
            {
                ++At;
            }
        }
    }
    
    return(Result);
}

internal void
FillGrid(grid *Grid, char *Input)
{
    parser Parser;
    Parser.At = Input;

    SkipWhitespace(&Parser);

    u32 Width = CountCharsInLine(Parser.At);
    u32 Height = CountNonEmptyLines(Parser.At);

    u32 DstRowY = GRID_HEIGHT/2 - Height/2;
    u32 DstRowX = GRID_WIDTH/2 - Width/2;

    while(Parser.At[0])
    {
        for(u32 X = 0;
            X < Width;
            ++X)
        {
            if(Parser.At[0] == '#')
            {
                Assert((DstRowX + X) < GRID_WIDTH);
                Assert(DstRowY < GRID_HEIGHT);
                Grid->Nodes[DstRowY][DstRowX + X] = 1;
            }
            else
            {
                Assert(Parser.At[0] == '.');
            }
            ++Parser.At;
        }
        SkipWhitespace(&Parser);
        ++DstRowY;
    }

    Assert(DstRowY == (GRID_HEIGHT/2 + Height/2 + 1));
}

internal u32
DoWork(grid *Grid, u32 BurstCount)
{
    u32 NumBurstsThatCausedInfection = 0;
    s32 DirX = 0;
    s32 DirY = -1;
    s32 X = GRID_WIDTH/2;
    s32 Y = GRID_HEIGHT/2;
    for(u32 BurstIndex = 0;
        BurstIndex < BurstCount;
        ++BurstIndex)
    {
        if(Grid->Nodes[Y][X])
        {
            // NOTE(slava): The current node is infected

            // NOTE(slava): Turn right
            s32 OldDirX = DirX;
            DirX = -DirY;
            DirY = OldDirX;

            Grid->Nodes[Y][X] = 0;
        }
        else
        {
            // NOTE(slava): The current node is clean

            // NOTE(slava): Turn left
            s32 OldDirX = DirX;
            DirX = DirY;
            DirY = -OldDirX;

            Grid->Nodes[Y][X] = 1;
            ++NumBurstsThatCausedInfection;
        }
        X += DirX;
        Y += DirY;
    }
    return(NumBurstsThatCausedInfection);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    u32 BurstCount = ((ArgCount > 2) ? ParseU32(Args[2]) : 10000);

    local_persist char Input[4*1024];
    ReadEntireFileAndNullTerminate(InputFileName, sizeof(Input), Input);

    local_persist grid Grid;
    FillGrid(&Grid, Input);
    u32 NumBurstsThatCausedInfection = DoWork(&Grid, BurstCount);
    printf("%u\n", NumBurstsThatCausedInfection);

    return(0);
}
