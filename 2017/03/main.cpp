#include "lib.h"

struct direction
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
    direction Dirs[4] =
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
            direction *Dir = Dirs + DirIndex;
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

int
main(void)
{
    TestCalculateDistanceTo1(1, 0);
    TestCalculateDistanceTo1(12, 3);
    TestCalculateDistanceTo1(23, 2);
    TestCalculateDistanceTo1(1024, 31);
    TestCalculateDistanceTo1(325489, 552);
}
