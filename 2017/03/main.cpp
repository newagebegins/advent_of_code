#include "lib.h"

struct v2i
{
    s32 X, Y;
};

internal u32
CalculateDistanceTo1(u32 From)
{
    u32 Result = 0;
    s32 X = 0;
    s32 Y = 0;
    u32 Loc = 1;
    v2i Dirs[4] =
    {
        {1, 0},
        {0, 1},
        {-1, 0},
        {0, -1},
    };
    u32 DirIndex = 0;
    // Straight lengths: 1,1,2,2,3,3,4,4,5,5,...
    u32 StraightLength = 1;
    while(Loc != From)
    {
        for(u32 SameLengthIteration = 0;
            (SameLengthIteration < 2) && (Loc != From);
            ++SameLengthIteration)
        {
            v2i *Dir = Dirs + DirIndex;
            for(u32 Step = 0;
                (Step < StraightLength) && (Loc != From);
                ++Step)
            {
                X += Dir->X;
                Y += Dir->Y;
                ++Loc;
            }
            ++DirIndex;
            if(DirIndex == ArrayCount(Dirs))
            {
                DirIndex = 0;
            }
        }
        ++StraightLength;
    }
    Result += ((X < 0) ? -X : X);
    Result += ((Y < 0) ? -Y : Y);
    return(Result);
}

internal void
TestCalculateDistanceTo1(u32 From, u32 Expected)
{
    u32 Distance = CalculateDistanceTo1(From);
    Assert(Distance == Expected);
}

struct map
{
    u32 *Values;
    s32 SizeX;
    s32 SizeY;
};

inline map
CreateMap(s32 SizeX, s32 SizeY)
{
    Assert(SizeX > 0);
    Assert(SizeY > 0);
    map Map = {};
    Map.SizeX = SizeX;
    Map.SizeY = SizeY;
    Map.Values = (u32 *)calloc(SizeX*SizeY, sizeof(Map.Values[0]));
    Assert(Map.Values);
    return(Map);
}

inline void
FreeMap(map *Map)
{
    free(Map->Values);
    *Map = {};
}

inline u32 *
GetValuePtrAt(map *Map, s32 X, s32 Y)
{
    Assert(X >= 0);
    Assert(X < Map->SizeX);
    Assert(Y >= 0);
    Assert(Y < Map->SizeY);
    u32 *Result = &Map->Values[Y*Map->SizeX + X];
    return(Result);
}

inline u32
GetValueAt(map *Map, s32 X, s32 Y)
{
    u32 Result = *GetValuePtrAt(Map, X, Y);
    return(Result);
}

inline void
SetValueAt(map *Map, s32 X, s32 Y, u32 Value)
{
    *GetValuePtrAt(Map, X, Y) = Value;
}

internal u32
FindFirstValueLargerThan(u32 Input)
{
    u32 Result = 0;

    s32 SizeX = 64;
    s32 SizeY = 64;
    map Map = CreateMap(SizeX, SizeY);
    s32 X = SizeX/2;
    s32 Y = SizeY/2;
    SetValueAt(&Map, X, Y, 1);

    v2i Dirs[4] =
    {
        {1, 0},
        {0, 1},
        {-1, 0},
        {0, -1},
    };
    u32 DirIndex = 0;

    v2i NeighborOffsets[8] =
    {
        {1,0},
        {1,1},
        {0,1},
        {-1,1},
        {-1,0},
        {-1,-1},
        {0,-1},
        {1,-1},
    };

    // Straight lengths: 1,1,2,2,3,3,4,4,5,5,...
    u32 StraightLength = 1;

    while(Result <= Input)
    {
        for(u32 SameLengthIteration = 0;
            (SameLengthIteration < 2) && (Result <= Input);
            ++SameLengthIteration)
        {
            v2i *Dir = Dirs + DirIndex;
            for(u32 Step = 0;
                (Step < StraightLength) && (Result <= Input);
                ++Step)
            {
                X += Dir->X;
                Y += Dir->Y;
                u32 Sum = 0;
                for(u32 NeighborIndex = 0;
                    NeighborIndex < ArrayCount(NeighborOffsets);
                    ++NeighborIndex)
                {
                    v2i *NeighborOffset = NeighborOffsets + NeighborIndex;
                    s32 NeighborX = X + NeighborOffset->X;
                    s32 NeighborY = Y + NeighborOffset->Y;
                    Sum += GetValueAt(&Map, NeighborX, NeighborY);
                }
                SetValueAt(&Map, X, Y, Sum);
                Result = Sum;
            }
            ++DirIndex;
            if(DirIndex == ArrayCount(Dirs))
            {
                DirIndex = 0;
            }
        }
        ++StraightLength;
    }
    FreeMap(&Map);
    return(Result);
}

internal void
TestFindFirstValueLargerThan(u32 Input, u32 Expected)
{
    u32 Value = FindFirstValueLargerThan(Input);
    Assert(Value == Expected);
}

int
main(void)
{
    u32 PuzzleInput = 325489;

    // NOTE(slava): Part 1

    TestCalculateDistanceTo1(1, 0);
    TestCalculateDistanceTo1(12, 3);
    TestCalculateDistanceTo1(23, 2);
    TestCalculateDistanceTo1(1024, 31);
    TestCalculateDistanceTo1(PuzzleInput, 552);

    // NOTE(slava): Part 2

    TestFindFirstValueLargerThan(1, 2);
    TestFindFirstValueLargerThan(5, 10);
    TestFindFirstValueLargerThan(59, 122);
    TestFindFirstValueLargerThan(750, 806);
    TestFindFirstValueLargerThan(PuzzleInput, 330785);
}
