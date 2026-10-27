#include "lib.h"

struct number_list
{
    u32 Count;
    s32 *Numbers;
};

internal number_list
ParseInput(memory_arena *Arena, char *Input)
{
    number_list Result;
    Result.Count = CountLines(Input);
    Result.Numbers = PushArray(Arena, Result.Count, s32);

    parser Parser;
    Parser.At = Input;

    u32 Index = 0;

    while(Parser.At[0])
    {
        Assert(Index < Result.Count);
        Result.Numbers[Index++] = ParseS32(&Parser);
        SkipWhitespace(&Parser);
    }

    Assert(Index == Result.Count);

    return(Result);
}

internal s32
SumNumbers(number_list List)
{
    s32 Result = 0;
    for(u32 Index = 0;
        Index < List.Count;
        ++Index)
    {
        Result += List.Numbers[Index];
    }
    return(Result);
}

struct frequency
{
    s32 Value;
    frequency *NextInHash;
};

struct frequency_hash
{
    frequency *Hash[4096];
};

struct frequency_result
{
    b32 Created;
    frequency *Frequency;
};

internal frequency_result
GetOrCreateFrequency(memory_arena *Arena, frequency_hash *Hash, s32 Value)
{
    frequency_result Result = {};
    s32 HashSlot = ModN(Value, ArrayCount(Hash->Hash));

    for(frequency *Frequency = Hash->Hash[HashSlot];
        Frequency;
        Frequency = Frequency->NextInHash)
    {
        if(Frequency->Value == Value)
        {
            Result.Frequency = Frequency;
            break;
        }
    }

    if(!Result.Frequency)
    {
        Result.Created = true;
        Result.Frequency = PushStruct(Arena, frequency);
        Result.Frequency->Value = Value;
        Result.Frequency->NextInHash = Hash->Hash[HashSlot];
        Hash->Hash[HashSlot] = Result.Frequency;
    }

    return(Result);
}

internal s32
FindFirstRepeatingFrequency(memory_arena *Arena, number_list List)
{
    temporary_memory TempMem = BeginTemporaryMemory(Arena);
    frequency_hash *Hash = PushStruct(Arena, frequency_hash); 
    s32 CurrentFrequency = 0;
    u32 NumberIndex = 0;
    for(;;)
    {
        frequency_result FrequencyResult = GetOrCreateFrequency(Arena, Hash, CurrentFrequency);
        if(!FrequencyResult.Created)
        {
            break;
        }

        CurrentFrequency += List.Numbers[NumberIndex];

        ++NumberIndex;
        if(NumberIndex == List.Count)
        {
            NumberIndex = 0;
        }
    }
    EndTemporaryMemory(TempMem);
    return(CurrentFrequency);
}

int
main(int ArgCount, char **Args)
{
    local_persist u8 ArenaMemory[4*1024*1024];
    memory_arena Arena;
    InitializeArena(&Arena, sizeof(ArenaMemory), ArenaMemory);

    char *InputFile = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(&Arena, InputFile);

    number_list NumberList = ParseInput(&Arena, Input);
    s32 Sum = SumNumbers(NumberList);
    printf("%d\n", Sum);

    s32 Frequency = FindFirstRepeatingFrequency(&Arena, NumberList);
    printf("%d\n", Frequency);

    CheckArena(&Arena);

    return(0);
}
