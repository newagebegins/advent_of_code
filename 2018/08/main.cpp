#include "lib.h"
#include "main.h"

struct numbers
{
    u32 Count;
    u32 *Numbers;
};

internal numbers
ParseNumbers(char *Input)
{
    numbers Result = {};
    Result.Count = CountSpaces(Input) + 1;
    Result.Numbers = (u32 *)malloc(Result.Count*sizeof(u32));

    parser Parser = MakeParser(Input);
    u32 NumberIndex = 0;

    while(StillParsing(&Parser))
    {
        Result.Numbers[NumberIndex++] = ParseU32(&Parser);
        SkipWhitespace(&Parser);
    }

    Assert(NumberIndex == Result.Count);

    return(Result);
}

struct sum_result
{
    u32 Sum;
    u32 *Node;
};

internal sum_result
SumMetadataEntries(u32 *Node)
{
    sum_result Result = {};
    Result.Node = Node;
    u32 ChildCount = *Result.Node++;
    u32 MetaCount = *Result.Node++;
    for(u32 ChildIndex = 0;
        ChildIndex < ChildCount;
        ++ChildIndex)
    {
        sum_result ChildResult = SumMetadataEntries(Result.Node);
        Result.Sum += ChildResult.Sum;
        Result.Node = ChildResult.Node;
    }
    for(u32 MetaIndex = 0;
        MetaIndex < MetaCount;
        ++MetaIndex)
    {
        Result.Sum += *Result.Node++;
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
        numbers Numbers = ParseNumbers(Input);
        sum_result SumResult = SumMetadataEntries(Numbers.Numbers);
        printf("%u\n", SumResult.Sum);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
