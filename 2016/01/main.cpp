#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#define Assert(Value) if(!(Value)) { *(int *)0 = 0; }
#define ArrayCount(A) (sizeof(A)/sizeof(A[0]))
#define Minimum(A, B) (((A) < (B)) ? (A) : (B))
#define Maximum(A, B) (((A) > (B)) ? (A) : (B))

struct v2i
{
    int32_t X, Y;
};

inline v2i
Add(v2i A, v2i B)
{
    v2i Result = {A.X + B.X, A.Y + B.Y};
    return(Result);
}

inline v2i
Scale(v2i A, int32_t B)
{
    v2i Result = {A.X*B, A.Y*B};
    return(Result);
}

struct instruction
{
    int32_t Turn; // 1 - turn right, -1 - turn left
    int32_t Blocks;
};

struct instructions
{
    int32_t Count;
    instruction *Data;
};

static instructions
ParseInput(char *Input)
{
    instructions Result = {};

    int32_t CommaCount = 0;
    for(char *At = Input;
        *At;
        ++At)
    {
        if(*At == ',')
        {
            ++CommaCount;
        }
    }

    Result.Count = CommaCount + 1;
    Result.Data = (instruction *)malloc(Result.Count * sizeof(Result.Data[0]));

    char *Source = Input;
    instruction *Instruction = Result.Data + 0;

    for(int32_t InstructionIndex = 0;
        InstructionIndex < Result.Count;
        ++InstructionIndex)
    {
        char Turn;
        int AssignCount = sscanf(Source, "%c%d", &Turn, &Instruction->Blocks);
        Assert(AssignCount == 2);
        Instruction->Turn = (Turn == 'L') ? -1 : 1;
        do
        {
            ++Source;
        } while (*Source && (*Source != 'L') && (*Source != 'R'));
        ++Instruction;
    }
    return(Result);
}

static int32_t
GetDistance(char *Input)
{
    instructions Instructions = ParseInput(Input);
    v2i Position = {0, 0};
    v2i Moves[] =
    {
        // NOTE(slava): Up
        {0,1},
        // NOTE(slava): Right
        {1,0},
        // NOTE(slava): Down
        {0,-1},
        // NOTE(slava): Left
        {-1,0},
    };
    int32_t Facing = 0;
    for(int32_t InstructionIndex = 0;
        InstructionIndex < Instructions.Count;
        ++InstructionIndex)
    {
        instruction *Instruction = Instructions.Data + InstructionIndex;
        Facing = Facing + Instruction->Turn;
        if(Facing == -1)
        {
            Facing = ArrayCount(Moves) - 1;
        }
        else if(Facing == ArrayCount(Moves))
        {
            Facing = 0;
        }
        Position = Add(Position, Scale(Moves[Facing], Instruction->Blocks));
    }
    int32_t Distance = abs(Position.X) + abs(Position.Y);
    return(Distance);
}

struct visited
{
    int32_t DimX, DimY;
    int8_t *Data;
};

inline int32_t
GetIndex(visited *Visited, v2i P)
{
    Assert(0 <= P.X && P.X < Visited->DimX);
    Assert(0 <= P.Y && P.Y < Visited->DimY);
    int32_t Index = P.Y*Visited->DimX + P.X;
    Assert(Index < Visited->DimX*Visited->DimY);
    return(Index);
}

inline void
Visit(visited *Visited, v2i P)
{
    Visited->Data[GetIndex(Visited, P)] = 1;
}

inline bool
IsVisited(visited *Visited, v2i P)
{
    bool Result = (Visited->Data[GetIndex(Visited, P)] == 1);
    return(Result);
}

static int32_t
CalcBlocksToFirstLocationVisitedTwice(char *Input)
{
    instructions Instructions = ParseInput(Input);
    v2i Moves[] =
    {
        // NOTE(slava): Up
        {0,1},
        // NOTE(slava): Right
        {1,0},
        // NOTE(slava): Down
        {0,-1},
        // NOTE(slava): Left
        {-1,0},
    };

    int32_t MinX = 0;
    int32_t MinY = 0;
    int32_t MaxX = 0;
    int32_t MaxY = 0;

    v2i Position = {0, 0};
    int32_t Facing = 0;
    for(int32_t InstructionIndex = 0;
        InstructionIndex < Instructions.Count;
        ++InstructionIndex)
    {
        instruction *Instruction = Instructions.Data + InstructionIndex;

        Facing = Facing + Instruction->Turn;
        if(Facing == -1)
        {
            Facing = ArrayCount(Moves) - 1;
        }
        else if(Facing == ArrayCount(Moves))
        {
            Facing = 0;
        }

        Position.X += Moves[Facing].X*Instruction->Blocks;
        Position.Y += Moves[Facing].Y*Instruction->Blocks;

        MinX = Minimum(MinX, Position.X);
        MinY = Minimum(MinY, Position.Y);
        MaxX = Maximum(MaxX, Position.X);
        MaxY = Maximum(MaxY, Position.Y);
    }

    visited Visited;
    Visited.DimX = MaxX - MinX + 1;
    Visited.DimY = MaxY - MinY + 1;
    Visited.Data = (int8_t *)calloc(Visited.DimX*Visited.DimY, sizeof(Visited.Data[0]));
    Assert(Visited.Data);

    Position = {-MinX, -MinY};
    Facing = 0;
    Visit(&Visited, Position);
    bool Found = false;
    for(int32_t InstructionIndex = 0;
        !Found && (InstructionIndex < Instructions.Count);
        ++InstructionIndex)
    {
        instruction *Instruction = Instructions.Data + InstructionIndex;
        Facing = Facing + Instruction->Turn;
        if(Facing == -1)
        {
            Facing = ArrayCount(Moves) - 1;
        }
        else if(Facing == ArrayCount(Moves))
        {
            Facing = 0;
        }
        for(int32_t BlockIndex = 0;
            !Found && (BlockIndex < Instruction->Blocks);
            ++BlockIndex)
        {
            Position.X += Moves[Facing].X;
            Position.Y += Moves[Facing].Y;

            if(IsVisited(&Visited, Position))
            {
                Found = true;
            }
            else
            {
                Visit(&Visited, Position);
            }
        }
    }
    Assert(Found);
    Position.X += MinX;
    Position.Y += MinY;
    int32_t Distance = abs(Position.X) + abs(Position.Y);
    return(Distance);
}

static void
Test(char *Input, int32_t Expected)
{
    int32_t Distance = GetDistance(Input);
    Assert(Distance == Expected);
}

static void
Test2(char *Input, int32_t Expected)
{
    int32_t Distance = CalcBlocksToFirstLocationVisitedTwice(Input);
    Assert(Distance == Expected);
}

int main(void)
{
    Test("R2, L3", 5);
    Test("R2, R2, R2", 2);
    Test("R5, L5, R5, R3", 12);

    char *Input = "L1, L5, R1, R3, L4, L5, R5, R1, L2, L2, L3, R4, L2, R3, R1, L2, R5, R3, L4, R4, L3, R3, R3, L2, R1, L3, R2, L1, R4, L2, R4, L4, R5, L3, R1, R1, L1, L3, L2, R1, R3, R2, L1, R4, L4, R2, L189, L4, R5, R3, L1, R47, R4, R1, R3, L3, L3, L2, R70, L1, R4, R185, R5, L4, L5, R4, L1, L4, R5, L3, R2, R3, L5, L3, R5, L1, R5, L4, R1, R2, L2, L5, L2, R4, L3, R5, R1, L5, L4, L3, R4, L3, L4, L1, L5, L5, R5, L5, L2, L1, L2, L4, L1, L2, R3, R1, R1, L2, L5, R2, L3, L5, L4, L2, L1, L2, R3, L1, L4, R3, R3, L2, R5, L1, L3, L3, L3, L5, R5, R1, R2, L3, L2, R4, R1, R1, R3, R4, R3, L3, R3, L5, R2, L2, R4, R5, L4, L3, L1, L5, L1, R1, R2, L1, R3, R4, R5, R2, R3, L2, L1, L5";
    Test(Input, 253);

    Test2("R8, R4, R4, R8", 4);
    printf("%d\n", CalcBlocksToFirstLocationVisitedTwice(Input));

    return(0);
}
