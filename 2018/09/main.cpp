#include "lib.h"
#include "main.h"

struct parsed_input
{
    u32 PlayerCount;
    u32 LastMarbleNumber;
};

internal parsed_input
ParseInput(char *Input)
{
    parsed_input Result;
    parser Parser = MakeParser(Input);
    Result.PlayerCount = ParseU32(&Parser);
    SkipString(&Parser, " players; last marble is worth ");
    Result.LastMarbleNumber = ParseU32(&Parser);
    return(Result);
}

struct marble
{
    u32 Number;
    u32 Prev;
    u32 Next;
};

internal u32
PlayMarbles(u32 PlayerCount, u32 MarbleCount)
{
    marble *Marbles = (marble *)malloc(MarbleCount*sizeof(marble));
    Marbles[0] = {};
    u32 NextFreeMarbleIndex = 1;
    u32 CurrentMarbleIndex = 0;
    u32 FirstMarbleIndex = 0;

    u32 *Scores = (u32 *)calloc(PlayerCount, sizeof(u32));
    u32 CurrentPlayerIndex = 0;

    for(u32 NextMarbleNumberToPlace = 1;
        NextMarbleNumberToPlace < MarbleCount;
        ++NextMarbleNumberToPlace)
    {
        if((NextMarbleNumberToPlace % 23) == 0)
        {
            Scores[CurrentPlayerIndex] += NextMarbleNumberToPlace;

            u32 RemovedMarbleIndex = CurrentMarbleIndex;
            for(u32 JumpIndex = 0;
                JumpIndex < 7;
                ++JumpIndex)
            {
                RemovedMarbleIndex = Marbles[RemovedMarbleIndex].Prev;
            }

            marble *Removed = Marbles + RemovedMarbleIndex;
            Marbles[Removed->Prev].Next = Removed->Next;
            Marbles[Removed->Next].Prev = Removed->Prev;

            CurrentMarbleIndex = Removed->Next;
            Scores[CurrentPlayerIndex] += Removed->Number;
            if(Removed->Number == 0)
            {
                FirstMarbleIndex = CurrentMarbleIndex;
            }

#if 0
            for(u32 PlayerIndex = 0;
                PlayerIndex < PlayerCount;
                ++PlayerIndex)
            {
                printf("[%u] = %u\n", PlayerIndex + 1, Scores[PlayerIndex]);
            }
#endif
        }
        else
        {
            CurrentMarbleIndex = Marbles[CurrentMarbleIndex].Next;
            marble *Current = Marbles + CurrentMarbleIndex;
            marble *Inserted = Marbles + NextFreeMarbleIndex;
            Inserted->Number = NextMarbleNumberToPlace;
            Inserted->Next = Current->Next;
            Inserted->Prev = CurrentMarbleIndex;
            Current->Next = NextFreeMarbleIndex;
            Marbles[Inserted->Next].Prev = NextFreeMarbleIndex;

            CurrentMarbleIndex = NextFreeMarbleIndex;
            ++NextFreeMarbleIndex;
        }

#if 0
        printf("[%u]", CurrentPlayerIndex + 1);
        u32 MarbleIndexToPrint = FirstMarbleIndex;
        for(;;)
        {
            printf(" ");
            if(MarbleIndexToPrint == CurrentMarbleIndex)
            {
                printf("(");
            }
            printf("%u", Marbles[MarbleIndexToPrint].Number);
            if(MarbleIndexToPrint == CurrentMarbleIndex)
            {
                printf(")");
            }
            MarbleIndexToPrint = Marbles[MarbleIndexToPrint].Next;
            if(MarbleIndexToPrint == FirstMarbleIndex)
            {
                break;
            }
        }
        printf("\n");
#endif

        ++CurrentPlayerIndex;
        if(CurrentPlayerIndex == PlayerCount)
        {
            CurrentPlayerIndex = 0;
        }
    }

    u32 MaxScore = 0;
    for(u32 PlayerIndex = 0;
        PlayerIndex < PlayerCount;
        ++PlayerIndex)
    {
        if(MaxScore < Scores[PlayerIndex])
        {
            MaxScore = Scores[PlayerIndex];
        }
    }

    free(Scores);
    free(Marbles);

    return(MaxScore);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        parsed_input ParsedInput = ParseInput(Input);
        u32 MaxScore = PlayMarbles(ParsedInput.PlayerCount, ParsedInput.LastMarbleNumber + 1);
        printf("%u\n", MaxScore);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
