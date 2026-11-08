#include "lib.h"
#include "main.h"

internal u8
ParseStepIndex(parser *Parser)
{
    str Word = ParseWord(Parser);
    Assert(Word.Length == 1);
    u8 Result = Word.Chars[0] - 'A';
    return(Result);
}

internal step *
ParseSteps(char *Input)
{
    step *Result = (step *)calloc(MAX_STEP_COUNT, sizeof(step));
    parser Parser = MakeParser(Input);
    while(StillParsing(&Parser))
    {
        SkipString(&Parser, "Step ");
        u8 FromIndex = ParseStepIndex(&Parser);
        SkipString(&Parser, " must be finished before step ");
        u8 ToIndex = ParseStepIndex(&Parser);

        Result[FromIndex].Out[Result[FromIndex].OutCount++] = ToIndex;
        ++Result[ToIndex].InCount;

        SkipRestOfLine(&Parser);
    }
    return(Result);
}

internal void
SortDesc(u32 Count, u8 *Data)
{
    for(u32 OuterIndex = 0;
        OuterIndex < Count;
        ++OuterIndex)
    {
        b32 Sorted = true;
        for(u32 InnerIndex = 0;
            InnerIndex < (Count - 1);
            ++InnerIndex)
        {
            if(Data[InnerIndex] < Data[InnerIndex + 1])
            {
                u8 Temp = Data[InnerIndex];
                Data[InnerIndex] = Data[InnerIndex + 1];
                Data[InnerIndex + 1] = Temp;
                Sorted = false;
            }
        }
        if(Sorted)
        {
            break;
        }
    }
}

internal void
SolvePart1(step *Steps)
{
    u8 NextSteps[MAX_STEP_COUNT];
    u32 NextStepCount = 0;

    for(u8 StepIndex = 0;
        StepIndex < MAX_STEP_COUNT;
        ++StepIndex)
    {
        step *Step = Steps + StepIndex;
        if((Step->InCount == 0) && (Step->OutCount != 0))
        {
            Assert(NextStepCount < ArrayCount(NextSteps));
            NextSteps[NextStepCount++] = StepIndex;
        }
    }

    while(NextStepCount != 0)
    {
        SortDesc(NextStepCount, NextSteps);
        u8 NextStepIndex = NextSteps[--NextStepCount];
        printf("%c", 'A' + NextStepIndex);
        step *NextStep = Steps + NextStepIndex;

        for(u8 OutIndex = 0;
            OutIndex < NextStep->OutCount;
            ++OutIndex)
        {
            step *OutStep = Steps + NextStep->Out[OutIndex];
            Assert(OutStep->InCount != 0);
            --OutStep->InCount;

            if(OutStep->InCount == 0)
            {
                Assert(NextStepCount < ArrayCount(NextSteps));
                NextSteps[NextStepCount++] = NextStep->Out[OutIndex];
            }
        }
    }

    printf("\n");
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        step *Steps = ParseSteps(Input);
        SolvePart1(Steps);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
