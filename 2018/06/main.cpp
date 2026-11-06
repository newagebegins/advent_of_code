#include "lib.h"
#include "main.h"

struct v2i
{
    s32 x, y;
};

struct parse_result
{
    u32 PointCount;
    v2i *Points;
};

internal parse_result
ParsePoints(char *Input)
{
    parse_result Result = {};
    Result.PointCount = CountLines(Input) + 1;
    Result.Points = (v2i *)malloc(Result.PointCount*sizeof(v2i));

    // NOTE(slava): A null point
    Result.Points[0] = {};
    u32 PointIndex = 1;

    parser Parser = MakeParser(Input);

    while(StillParsing(&Parser))
    {
        Assert(PointIndex < Result.PointCount);
        v2i *P = Result.Points + PointIndex++;
        P->x = ParseU32(&Parser);
        SkipChar(&Parser, ',');
        SkipWhitespace(&Parser);
        P->y = ParseU32(&Parser);
        SkipWhitespace(&Parser);
    }

    Assert(PointIndex == Result.PointCount);

    return(Result);
}

internal u32
FindLargestAreaThatIsntInfinite(u32 PointCount, v2i *Points)
{
    s32 MinX = INT_MAX;
    s32 MaxX = INT_MIN;
    s32 MinY = INT_MAX;
    s32 MaxY = INT_MIN;

    for(u32 PointIndex = 1;
        PointIndex < PointCount;
        ++PointIndex)
    {
        v2i P = Points[PointIndex];
        if(P.x < MinX)
        {
            MinX = P.x;
        }
        else if(P.x > MaxX)
        {
            MaxX = P.x;
        }
        if(P.y < MinY)
        {
            MinY = P.y;
        }
        else if(P.y > MaxY)
        {
            MaxY = P.y;
        }
    }

    u32 CountX = MaxX - MinX + 1;
    u32 CountY = MaxY - MinY + 1;
    u8 *Grid = (u8 *)malloc(CountX*CountY*sizeof(u8));

    for(u32 OffsetY = 0;
        OffsetY < CountY;
        ++OffsetY)
    {
        s32 Y = MinY + OffsetY;
        for(u32 OffsetX = 0;
            OffsetX < CountX;
            ++OffsetX)
        {
            s32 X = MinX + OffsetX;
            u32 MinDistance = UINT_MAX;
            u32 ClosestPointIndex = 0;
            for(u32 PointIndex = 1;
                PointIndex < PointCount;
                ++PointIndex)
            {
                v2i P = Points[PointIndex];
                u32 Distance = AbsoluteValue(P.x - X) + AbsoluteValue(P.y - Y);
                if(Distance < MinDistance)
                {
                    MinDistance = Distance;
                    ClosestPointIndex = PointIndex;
                }
                else if(Distance == MinDistance)
                {
                    ClosestPointIndex = 0;
                }
            }
            Grid[OffsetY*CountX + OffsetX] = (u8)ClosestPointIndex;
        }
    }

    u32 *Area = (u32 *)calloc(PointCount, sizeof(u32));
    b32 *InfiniteArea = (b32 *)calloc(PointCount, sizeof(b32));

    for(u32 OffsetY = 0;
        OffsetY < CountY;
        ++OffsetY)
    {
        for(u32 OffsetX = 0;
            OffsetX < CountX;
            ++OffsetX)
        {
            u8 PointIndex = Grid[OffsetY*CountX + OffsetX];
            ++Area[PointIndex];
            if((OffsetX == 0) ||
               (OffsetY == 0) ||
               (OffsetX == (CountX - 1)) ||
               (OffsetY == (CountY - 1)))
            {
                InfiniteArea[PointIndex] = true;
            }
        }
    }

    u32 MaxArea = 0;

    for(u32 PointIndex = 1;
        PointIndex < PointCount;
        ++PointIndex)
    {
        if(!InfiniteArea[PointIndex] && (MaxArea < Area[PointIndex]))
        {
            MaxArea = Area[PointIndex];
        }
    }

    free(InfiniteArea);
    free(Area);
    free(Grid);

    return(MaxArea);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        memory_index ArenaSize = Megabytes(16);
        void *ArenaBase = malloc(ArenaSize);
        memory_arena Arena;
        InitializeArena(&Arena, ArenaSize, ArenaBase);

        parse_result ParseResult = ParsePoints(Input);
        u32 LargestArea = FindLargestAreaThatIsntInfinite(ParseResult.PointCount, ParseResult.Points);
        printf("%u\n", LargestArea);

        CheckArena(&Arena);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
