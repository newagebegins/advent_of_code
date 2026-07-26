#include "lib.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>

struct v2i
{
    s32 X;
    s32 Y;
};

inline v2i
V2i(s32 X, s32 Y)
{
    v2i Result = {X, Y};
    return(Result);
}

inline v2i
operator+(v2i A, v2i B)
{
    v2i Result = {A.X + B.X, A.Y + B.Y};
    return(Result);
}

struct check_cell
{
    v2i P;
    u32 StepsFromStart;
};

struct queue
{
    check_cell *Cells;
    u32 MaxCount;
    u32 Count;
    u32 BeginIndex;
    u32 OnePastEndIndex;
};

internal void
InitializeQueue(queue *Queue, memory_arena *Arena, u32 MaxCount)
{
    Queue->Cells = PushArray(Arena, MaxCount, check_cell);
    Queue->MaxCount = MaxCount;
    Queue->Count = 0;
    Queue->BeginIndex = 0;
    Queue->OnePastEndIndex = 0;
}

internal void
Enqueue(queue *Queue, v2i P, u32 StepsFromStart)
{
    Assert(Queue->Count < Queue->MaxCount);
    ++Queue->Count;

    check_cell *Cell = Queue->Cells + Queue->OnePastEndIndex++;
    if(Queue->OnePastEndIndex == Queue->MaxCount)
    {
        Queue->OnePastEndIndex = 0;
    }
    Cell->P = P;
    Cell->StepsFromStart = StepsFromStart;
}

internal check_cell
Dequeue(queue *Queue)
{
    Assert(Queue->Count);
    --Queue->Count;
    check_cell Result = Queue->Cells[Queue->BeginIndex++];
    if(Queue->BeginIndex == Queue->MaxCount)
    {
        Queue->BeginIndex = 0;
    }
    return(Result);
}

inline u32
CountOnes(s32 Val)
{
    u32 Result = 0;
    for(u32 BitIndex = 0;
        BitIndex < 32;
        ++BitIndex)
    {
        if(Val & (1 << BitIndex))
        {
            ++Result;
        }
    }
    return(Result);
}

inline b32
IsOpenSpace(s32 X, s32 Y, s32 FavoriteNumber)
{
    s32 Val = X*X + 3*X + 2*X*Y + Y + Y*Y;
    Val += FavoriteNumber;
    u32 OnesCount = CountOnes(Val);
    b32 Result = (OnesCount % 2 == 0);
    return(Result);
}

internal s32
GetStepsToGoal(memory_arena *Arena, s32 FavoriteNumber, s32 GoalX, s32 GoalY)
{
    s32 Result = -1;
    v2i Directions[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    queue Queue;
    InitializeQueue(&Queue, Arena, 1024);
    Enqueue(&Queue, V2i(1, 1), 0);

    s32 MaxX = 512;
    s32 MaxY = 512;
    s32 MaxCellCount = MaxX*MaxY;
    u8 *Visited = PushArray(Arena, MaxCellCount, u8);
    ZeroArray(Visited, MaxCellCount);

    for(;;)
    {
        check_cell Cell = Dequeue(&Queue);
        s32 CellIndex = MaxX*Cell.P.Y + Cell.P.X;
        Assert(CellIndex < MaxCellCount);
        if(!Visited[CellIndex])
        {
            Visited[CellIndex] = 1;

            if((Cell.P.X == GoalX) && (Cell.P.Y == GoalY))
            {
                Result = Cell.StepsFromStart;
                break;
            }

            for(u32 DirectionIndex = 0;
                DirectionIndex < ArrayCount(Directions);
                ++DirectionIndex)
            {
                v2i Direction = Directions[DirectionIndex];
                v2i NewP = Cell.P + Direction;
                Assert(NewP.X < MaxX);
                Assert(NewP.Y < MaxY);
                s32 NewCellIndex = MaxX*NewP.Y + NewP.X;

                if((NewP.X >= 0) && (NewP.Y >= 0) && !Visited[NewCellIndex] && IsOpenSpace(NewP.X, NewP.Y, FavoriteNumber))
                {
                    Enqueue(&Queue, NewP, Cell.StepsFromStart + 1);
                }
            }
        }
    }

    EndTemporaryMemory(TempMem);

    return(Result);
}

internal s32
CountLocationsWithin50Steps(memory_arena *Arena, s32 FavoriteNumber)
{
    s32 Result = 0;
    v2i Directions[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    queue Queue;
    InitializeQueue(&Queue, Arena, 1024);
    Enqueue(&Queue, V2i(1, 1), 0);

    s32 MaxX = 512;
    s32 MaxY = 512;
    s32 MaxCellCount = MaxX*MaxY;
    u8 *Visited = PushArray(Arena, MaxCellCount, u8);
    ZeroArray(Visited, MaxCellCount);

    for(;;)
    {
        check_cell Cell = Dequeue(&Queue);
        s32 CellIndex = MaxX*Cell.P.Y + Cell.P.X;
        Assert(CellIndex < MaxCellCount);
        if(!Visited[CellIndex])
        {
            Visited[CellIndex] = 1;

            if(Cell.StepsFromStart > 50)
            {
                break;
            }

            ++Result;

            for(u32 DirectionIndex = 0;
                DirectionIndex < ArrayCount(Directions);
                ++DirectionIndex)
            {
                v2i Direction = Directions[DirectionIndex];
                v2i NewP = Cell.P + Direction;
                Assert(NewP.X < MaxX);
                Assert(NewP.Y < MaxY);
                s32 NewCellIndex = MaxX*NewP.Y + NewP.X;

                if((NewP.X >= 0) && (NewP.Y >= 0) && !Visited[NewCellIndex] && IsOpenSpace(NewP.X, NewP.Y, FavoriteNumber))
                {
                    Enqueue(&Queue, NewP, Cell.StepsFromStart + 1);
                }
            }
        }
    }

    EndTemporaryMemory(TempMem);

    return(Result);
}

int main(void)
{
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(64);
    void *ArenaBase = malloc(ArenaSize);
    Assert(ArenaBase);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

    s32 FavoriteNumber = 1364;

    s32 TestSteps = GetStepsToGoal(&Arena, 10, 7, 4);
    Assert(TestSteps == 11);
    s32 Steps = GetStepsToGoal(&Arena, FavoriteNumber, 31, 39);
    Assert(Steps == 86);

    s32 LocCount = CountLocationsWithin50Steps(&Arena, FavoriteNumber);
    Assert(LocCount == 127);

    return(0);
}
