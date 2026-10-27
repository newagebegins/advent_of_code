#include "lib.h"

internal s32
FindResultingFrequency(char *Input)
{
    s32 Result = 0;

    parser Parser;
    Parser.At = Input;

    while(Parser.At[0])
    {
        s32 Value = ParseS32(&Parser);
        Result += Value;
        SkipWhitespace(&Parser);
    }

    return(Result);
}

int
main(int ArgCount, char **Args)
{
    local_persist u8 ArenaMemory[4*1024*1024];
    memory_arena Arena;
    InitializeArena(&Arena, sizeof(ArenaMemory), ArenaMemory);

    char *InputFile = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(&Arena, InputFile);

    s32 Frequency = FindResultingFrequency(Input);
    printf("%d\n", Frequency);

    CheckArena(&Arena);

    return(0);
}
