#include "lib.h"
#include "main.h"

internal v2i
ParseV2i(parser *Parser)
{
    v2i Result;
    SkipChar(Parser, '<');
    SkipWhitespace(Parser);
    Result.x = ParseS32(Parser);
    SkipChar(Parser, ',');
    SkipWhitespace(Parser);
    Result.y = ParseS32(Parser);
    SkipChar(Parser, '>');
    return(Result);
}

internal points
ParsePoints(char *Input)
{
    points Result = {};
    Result.Count = CountLines(Input);
    Result.Points = (point *)malloc(Result.Count * sizeof(point));

    parser Parser = MakeParser(Input);
    u32 PointIndex = 0;

    while(StillParsing(&Parser))
    {
        point *Point = Result.Points + PointIndex++;
        SkipString(&Parser, "position=");
        Point->P = ParseV2i(&Parser);
        SkipWhitespace(&Parser);
        SkipString(&Parser, "velocity=");
        Point->V = ParseV2i(&Parser);
        SkipWhitespace(&Parser);
    }

    Assert(PointIndex == Result.Count);

    return(Result);
}

inline u32
CalculateDistance(v2i A, v2i B)
{
    u32 Result = AbsoluteValue(A.x - B.x) + AbsoluteValue(A.y - B.y);
    return(Result);
}

internal u32
FindDistanceToClosestPoint(points Points, u32 FromIndex)
{
    u32 Result = UINT_MAX;
    point From = Points.Points[FromIndex];
    for(u32 PointIndex = 0;
        PointIndex < Points.Count;
        ++PointIndex)
    {
        if(PointIndex != FromIndex)
        {
            point To = Points.Points[PointIndex];
            u32 Dist = CalculateDistance(From.P, To.P);
            if(Dist < Result)
            {
                Result = Dist;
            }
        }
    }
    return(Result);
}

internal b32
AssembledIntoMessage(points Points)
{
    b32 Result = true;
    for(u32 Index = 0;
        Index < Points.Count;
        ++Index)
    {
        u32 Dist = FindDistanceToClosestPoint(Points, Index);
        if(Dist > 2)
        {
            Result = false;
            break;
        }
    }
    return(Result);
}

inline void
SimulateOneSecond(points Points)
{
    for(u32 PointIndex = 0;
        PointIndex < Points.Count;
        ++PointIndex)
    {
        point *Point = Points.Points + PointIndex;
        Point->P.x += Point->V.x;
        Point->P.y += Point->V.y;
    }
}

internal void
PrintPoints(points Points)
{
    s32 MinX = INT_MAX;
    s32 MaxX = INT_MIN;
    s32 MinY = INT_MAX;
    s32 MaxY = INT_MIN;

    for(u32 PointIndex = 0;
        PointIndex < Points.Count;
        ++PointIndex)
    {
        point Point = Points.Points[PointIndex];
        if(Point.P.x < MinX)
        {
            MinX = Point.P.x;
        }
        if(MaxX < Point.P.x)
        {
            MaxX = Point.P.x;
        }
        if(Point.P.y < MinY)
        {
            MinY = Point.P.y;
        }
        if(MaxY < Point.P.y)
        {
            MaxY = Point.P.y;
        }
    }

    u32 CountX = MaxX - MinX + 1;
    u32 CountY = MaxY - MinY + 1;
    u8 *Grid = (u8 *)calloc(CountX*CountY, sizeof(u8));

    for(u32 PointIndex = 0;
        PointIndex < Points.Count;
        ++PointIndex)
    {
        point Point = Points.Points[PointIndex];
        u32 X = Point.P.x - MinX;
        u32 Y = Point.P.y - MinY;
        Grid[Y*CountX + X] = 1;
    }

    for(u32 Y = 0;
        Y < CountY;
        ++Y)
    {
        for(u32 X = 0;
            X < CountX;
            ++X)
        {
            if(Grid[Y*CountX + X])
            {
                printf("#");
            }
            else
            {
                printf(".");
            }
        }
        printf("\n");
    }

    free(Grid);
}

internal void
FindMessage(points Points)
{
    while(!AssembledIntoMessage(Points))
    {
        SimulateOneSecond(Points);
    }
    PrintPoints(Points);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        points Points = ParsePoints(Input);
        FindMessage(Points);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
