#include "main.h"

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

internal node *
GetNode(memory_arena *Arena, grid *Grid, s32 X, s32 Y)
{
    u32 HashIndex = ((X*7 + Y*13) & HASH_MASK);
    node *Node = 0;

    for (Node = Grid->Nodes[HashIndex];
         Node;
         Node = Node->NextInHash)
    {
        if((Node->X == X) && (Node->Y == Y))
        {
            break;
        }
    }

    if(!Node)
    {
        Node = PushStruct(Arena, node);
        Node->State = Node_Clean;
        Node->X = X;
        Node->Y = Y;
        Node->NextInHash = Grid->Nodes[HashIndex];
        Grid->Nodes[HashIndex] = Node;
    }

    return(Node);
}

internal grid *
CreateGrid(memory_arena *Arena, char *Input)
{
    grid *Grid = PushStruct(Arena, grid);
    ZeroStruct(*Grid);

    parser Parser;
    Parser.At = Input;

    SkipWhitespace(&Parser);

    s32 Width = CountCharsInLine(Parser.At);
    s32 Height = CountNonEmptyLines(Parser.At);

    s32 DstRowY = -Height/2;
    s32 DstRowX = -Width/2;

    while(Parser.At[0])
    {
        for(s32 X = 0;
            X < Width;
            ++X)
        {
            if(Parser.At[0] == '#')
            {
                node *Node = GetNode(Arena, Grid, DstRowX + X, DstRowY);
                Node->State = Node_Infected;
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

    return(Grid);
}

internal u32
DoWork(memory_arena *Arena, grid *Grid, u32 BurstCount)
{
    u32 NumBurstsThatCausedInfection = 0;
    s32 DirX = 0;
    s32 DirY = -1;
    s32 X = 0;
    s32 Y = 0;
    for(u32 BurstIndex = 0;
        BurstIndex < BurstCount;
        ++BurstIndex)
    {
        node *Node = GetNode(Arena, Grid, X, Y);
        if(Node->State == Node_Infected)
        {
            // NOTE(slava): The current node is infected

            // NOTE(slava): Turn right
            s32 OldDirX = DirX;
            DirX = -DirY;
            DirY = OldDirX;

            Node->State = Node_Clean;
        }
        else
        {
            // NOTE(slava): The current node is clean
            Assert(Node->State == Node_Clean);

            // NOTE(slava): Turn left
            s32 OldDirX = DirX;
            DirX = DirY;
            DirY = -OldDirX;

            Node->State = Node_Infected;
            ++NumBurstsThatCausedInfection;
        }
        X += DirX;
        Y += DirY;
    }
    return(NumBurstsThatCausedInfection);
}

internal u32
DoWork2(memory_arena *Arena, grid *Grid, u32 BurstCount)
{
    u32 NumBurstsThatCausedInfection = 0;
    s32 DirX = 0;
    s32 DirY = -1;
    s32 X = 0;
    s32 Y = 0;
    for(u32 BurstIndex = 0;
        BurstIndex < BurstCount;
        ++BurstIndex)
    {
        node *Node = GetNode(Arena, Grid, X, Y);

        switch(Node->State)
        {
            case Node_Clean:
            {
                // NOTE(slava): Turn left
                s32 OldDirX = DirX;
                DirX = DirY;
                DirY = -OldDirX;

                Node->State = Node_Weakened;
            } break;

            case Node_Infected:
            {
                // NOTE(slava): Turn right
                s32 OldDirX = DirX;
                DirX = -DirY;
                DirY = OldDirX;

                Node->State = Node_Flagged;
            } break;

            case Node_Weakened:
            {
                // NOTE(slava): Continue moving in the same direction

                Node->State = Node_Infected;
                ++NumBurstsThatCausedInfection;
            } break;

            case Node_Flagged:
            {
                // NOTE(slava): Reverse direction
                DirX = -DirX;
                DirY = -DirY;

                Node->State = Node_Clean;
            } break;

            InvalidDefaultCase;
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

    local_persist char Input[4*1024];
    ReadEntireFileAndNullTerminate(InputFileName, sizeof(Input), Input);

    local_persist u8 ArenaMemory[64*1024*1024];
    memory_arena Arena;
    InitializeArena(&Arena, sizeof(ArenaMemory), ArenaMemory);

    // NOTE(slava): Part 1
    {
        Clear(&Arena);
        grid *Grid = CreateGrid(&Arena, Input);
        u32 NumBurstsThatCausedInfection = DoWork(&Arena, Grid, 10000);
        printf("%u\n", NumBurstsThatCausedInfection);
    }

    // NOTE(slava): Part 2
    {
        Clear(&Arena);
        grid *Grid = CreateGrid(&Arena, Input);
        u32 NumBurstsThatCausedInfection = DoWork2(&Arena, Grid, 10000000);
        printf("%u\n", NumBurstsThatCausedInfection);
    }

    return(0);
}
