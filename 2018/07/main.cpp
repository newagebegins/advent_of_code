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

internal void
SolvePart2(step *Steps, u32 WorkerCount, u32 BaseStepSeconds)
{
    u32 Time = 0;
    worker *Workers = (worker *)calloc(WorkerCount, sizeof(worker));
    u32 FreeWorkerCount = WorkerCount;
    
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

    while((NextStepCount != 0) || (FreeWorkerCount != WorkerCount))
    {
        SortDesc(NextStepCount, NextSteps);
        while((NextStepCount != 0) && (FreeWorkerCount != 0))
        {
            u8 NextStepIndex = NextSteps[--NextStepCount];
            for(u32 WorkerIndex = 0;
                WorkerIndex < WorkerCount;
                ++WorkerIndex)
            {
                worker *Worker = Workers + WorkerIndex;
                if(Worker->SecondsLeft == 0)
                {
                    Worker->StepIndex = NextStepIndex;
                    Worker->SecondsLeft = BaseStepSeconds + NextStepIndex + 1;
                    --FreeWorkerCount;
                    break;
                }
            }
        }

        ++Time;

        for(u32 WorkerIndex = 0;
            WorkerIndex < WorkerCount;
            ++WorkerIndex)
        {
            worker *Worker = Workers + WorkerIndex;
            if(Worker->SecondsLeft != 0)
            {
                --Worker->SecondsLeft;
                if(Worker->SecondsLeft == 0)
                {
                    ++FreeWorkerCount;
                    //printf("%c", 'A' + Worker->StepIndex);
                    step *Step = Steps + Worker->StepIndex;

                    for(u8 OutIndex = 0;
                        OutIndex < Step->OutCount;
                        ++OutIndex)
                    {
                        step *OutStep = Steps + Step->Out[OutIndex];
                        Assert(OutStep->InCount != 0);
                        --OutStep->InCount;

                        if(OutStep->InCount == 0)
                        {
                            Assert(NextStepCount < ArrayCount(NextSteps));
                            NextSteps[NextStepCount++] = Step->Out[OutIndex];
                        }
                    }
                }
            }
        }
    }

    printf("%u\n", Time);

    free(Workers);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    u32 WorkerCount = ((ArgCount > 2) ? ParseU32(Args[2]) : 5);
    u32 BaseStepSeconds = ((ArgCount > 3) ? ParseU32(Args[3]) : 60);
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        {
            step *Steps = ParseSteps(Input);
            SolvePart1(Steps);
            free(Steps);
        }
        {
            step *Steps = ParseSteps(Input);
            SolvePart2(Steps, WorkerCount, BaseStepSeconds);
            free(Steps);
        }
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
