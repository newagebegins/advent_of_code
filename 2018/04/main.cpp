#include "lib.h"
#include "main.h"

internal records
ParseRecords(char *Input)
{
    records Result;
    Result.Count = CountLines(Input);
    Result.Records = (record *)malloc(Result.Count * sizeof(record));

    parser Parser = MakeParser(Input);
    u32 RecordIndex = 0;

    while(StillParsing(&Parser))
    {
        Assert(RecordIndex < Result.Count);
        record *Record = Result.Records + RecordIndex++;

        Record->GuardID = 0;

        SkipChar(&Parser, '[');
        Record->Year = (u16)ParseU32(&Parser);
        SkipChar(&Parser, '-');
        Record->Month = (u8)ParseU32(&Parser);
        SkipChar(&Parser, '-');
        Record->Day = (u8)ParseU32(&Parser);
        SkipWhitespace(&Parser);
        Record->Hour = (u8)ParseU32(&Parser);
        SkipChar(&Parser, ':');
        Record->Minute = (u8)ParseU32(&Parser);
        SkipChar(&Parser, ']');
        SkipWhitespace(&Parser);

        str Word = ParseWord(&Parser);
        SkipWhitespace(&Parser);

        if(StringsAreEqual(Word, "Guard"))
        {
            Record->Event = Event_BeginsShift;
            SkipChar(&Parser, '#');
            Record->GuardID = (u16)ParseU32(&Parser);
            Assert(Record->GuardID < ONE_PAST_MAX_GUARD_ID);
        }
        else if(StringsAreEqual(Word, "falls"))
        {
            Record->Event = Event_FallsAsleep;
        }
        else if(StringsAreEqual(Word, "wakes"))
        {
            Record->Event = Event_WakesUp;
        }
        else
        {
            InvalidCodePath;
        }

        SkipRestOfLine(&Parser);
    }

    Assert(RecordIndex == Result.Count);

    return(Result);
}

#define MinutesPerHour 60
#define MinutesPerDay (24*MinutesPerHour)
#define MinutesPerMonth (31*MinutesPerDay)
#define MinutesPerYear (12*MinutesPerMonth)

internal void
SortRecords(records Records)
{
    for(u32 FirstUnsortedIndex = 0;
        FirstUnsortedIndex < Records.Count;
        ++FirstUnsortedIndex)
    {
        u32 MinSortKey = 0xFFFFFFFF;
        u32 MinIndex = 0;

        for(u32 Index = FirstUnsortedIndex;
            Index < Records.Count;
            ++Index)
        {
            record *Record = Records.Records + Index;
            u32 SortKey = (Record->Year*MinutesPerYear +
                           Record->Month*MinutesPerMonth +
                           Record->Day*MinutesPerDay +
                           Record->Hour*MinutesPerHour +
                           Record->Minute);
            if(SortKey < MinSortKey)
            {
                MinSortKey = SortKey;
                MinIndex = Index;
            }
        }

        record Swap = Records.Records[FirstUnsortedIndex];
        Records.Records[FirstUnsortedIndex] = Records.Records[MinIndex];
        Records.Records[MinIndex] = Swap;
    }
}

internal guard *
CollateRecords(records Records)
{
    guard *Guards = (guard *)calloc(ONE_PAST_MAX_GUARD_ID, sizeof(guard));

    guard *Guard = 0;
    u8 FallAsleepMinute = 0;

    for(u32 RecordIndex = 0;
        RecordIndex < Records.Count;
        ++RecordIndex)
    {
        record *Record = Records.Records + RecordIndex;

        switch(Record->Event)
        {
            case Event_BeginsShift:
            {
                Guard = Guards + Record->GuardID;
            } break;

            case Event_FallsAsleep:
            {
                FallAsleepMinute = Record->Minute;
            } break;

            case Event_WakesUp:
            {
                u32 MinutesAsleep = Record->Minute - FallAsleepMinute;
                Assert(MinutesAsleep > 0);
                Guard->TotalMinutesAsleep += MinutesAsleep;
                for(u8 Minute = FallAsleepMinute;
                    Minute < Record->Minute;
                    ++Minute)
                {
                    ++Guard->AsleepCountForMinute[Minute];
                }
            } break;

            InvalidDefaultCase;
        }
    }

    return(Guards);
}

struct strategy1_result
{
    u16 GuardID;
    u8 Minute;
};

internal strategy1_result
DoStrategy1(guard *Guards)
{
    u32 MaxTotalMinutesAsleep = 0;
    u16 ChosenID = 0;

    for(u16 GuardID = 0;
        GuardID < ONE_PAST_MAX_GUARD_ID;
        ++GuardID)
    {
        guard *Guard = Guards + GuardID;
        if(MaxTotalMinutesAsleep < Guard->TotalMinutesAsleep)
        {
            MaxTotalMinutesAsleep = Guard->TotalMinutesAsleep;
            ChosenID = GuardID;
        }
    }

    guard *ChosenGuard = Guards + ChosenID;
    u8 MaxAsleepCount = 0;
    u8 ChosenMinute = 0;

    for(u8 Minute = 0;
        Minute < 60;
        ++Minute)
    {
        u8 AsleepCount = ChosenGuard->AsleepCountForMinute[Minute];
        if(MaxAsleepCount < AsleepCount)
        {
            MaxAsleepCount = AsleepCount;
            ChosenMinute = Minute;
        }
    }

    strategy1_result Result;
    Result.GuardID = ChosenID;
    Result.Minute = ChosenMinute;

    return(Result);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        records Records = ParseRecords(Input);
        SortRecords(Records);
        guard *Guards = CollateRecords(Records);
        strategy1_result Result1 = DoStrategy1(Guards);
        printf("%u\n", Result1.GuardID * Result1.Minute);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
