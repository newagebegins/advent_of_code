#include "lib.h"
#include "main.h"

internal u32
CountClaimOverlaps(claimed_fabric Fabric)
{
    u32 Result = 0;
    u32 SquareCount = Fabric.Width * Fabric.Height;
    for(u32 SquareIndex = 0;
        SquareIndex < SquareCount;
        ++SquareIndex)
    {
        u8 ClaimCount = Fabric.ClaimCounts[SquareIndex];
        if(ClaimCount > 1)
        {
            ++Result;
        }
    }
    return(Result);
}

internal claim_rects
ParseClaimRects(char *Input)
{
    claim_rects Result = {};
    Result.Count = CountLines(Input);
    Result.Rects = (claim_rect *)malloc(Result.Count * sizeof(claim_rect));

    parser Parser = MakeParser(Input);
    u32 RectIndex = 0;

    while(StillParsing(&Parser))
    {
        claim_rect *Rect = Result.Rects + RectIndex++;

        SkipChar(&Parser, '#');
        u32 ID = ParseU32(&Parser);
        Assert(ID == RectIndex);
        SkipWhitespace(&Parser);

        SkipChar(&Parser, '@');
        SkipWhitespace(&Parser);

        Rect->X = ParseU32(&Parser);
        SkipChar(&Parser, ',');
        Rect->Y = ParseU32(&Parser);
        SkipChar(&Parser, ':');
        SkipWhitespace(&Parser);

        Rect->Width = ParseU32(&Parser);
        SkipChar(&Parser, 'x');
        Rect->Height = ParseU32(&Parser);

        SkipWhitespace(&Parser);
    }

    return(Result);
}

internal claimed_fabric
ClaimFabric(claim_rects Rects)
{
    claimed_fabric Result = {};

    for(u32 RectIndex = 0;
        RectIndex < Rects.Count;
        ++RectIndex)
    {
        claim_rect Rect = Rects.Rects[RectIndex];
        u32 MaxX = Rect.X + Rect.Width;
        u32 MaxY = Rect.Y + Rect.Height;
        if(Result.Width < MaxX)
        {
            Result.Width = MaxX;
        }
        if(Result.Height < MaxY)
        {
            Result.Height = MaxY;
        }
    }
    Result.ClaimCounts = (u8 *)calloc(Result.Width * Result.Height, sizeof(u8));

    for(u32 RectIndex = 0;
        RectIndex < Rects.Count;
        ++RectIndex)
    {
        claim_rect Rect = Rects.Rects[RectIndex];
        u32 MaxX = Rect.X + Rect.Width;
        u32 MaxY = Rect.Y + Rect.Height;

        for(u32 Y = Rect.Y;
            Y < MaxY;
            ++Y)
        {
            for(u32 X = Rect.X;
                X < MaxX;
                ++X)
            {
                ++Result.ClaimCounts[Y*Result.Width + X];
            }
        }
    }    

    return(Result);
}

internal b32
Overlaps(claimed_fabric Fabric, claim_rect Rect)
{
    b32 Result = false;
    for(u32 Y = Rect.Y;
        (Y < (Rect.Y + Rect.Height)) && !Result;
        ++Y)
    {
        for(u32 X = Rect.X;
            X < (Rect.X + Rect.Width);
            ++X)
        {
            if(Fabric.ClaimCounts[Y*Fabric.Width + X] > 1)
            {
                Result = true;
                break;
            }
        }
    }
    return(Result);
}

internal u32
FindNonOverlappingClaimID(claimed_fabric Fabric, claim_rects Rects)
{
    u32 Result = 0;
    for(u32 RectIndex = 0;
        RectIndex < Rects.Count;
        ++RectIndex)
    {
        claim_rect Rect = Rects.Rects[RectIndex];
        if(!Overlaps(Fabric, Rect))
        {
            Result = RectIndex + 1;
            break;
        }
    }
    return(Result);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        claim_rects Rects = ParseClaimRects(Input);
        claimed_fabric Fabric = ClaimFabric(Rects);
        u32 Overlaps = CountClaimOverlaps(Fabric);
        printf("%u\n", Overlaps);

        u32 ID = FindNonOverlappingClaimID(Fabric, Rects);
        printf("%u\n", ID);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
