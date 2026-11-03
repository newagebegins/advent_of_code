#include "lib.h"
#include "main.h"

internal polymer
ParsePolymer(memory_arena *Arena, char *Input)
{
    polymer Result = {};
    parser Parser = MakeParser(Input);
    polymer_unit *Prev = 0;

    while(StillParsing(&Parser))
    {
        char C = Parser.At[0];
        Assert((('a' <= C) && (C <= 'z')) ||
               (('A' <= C) && (C <= 'Z')));
        ++Parser.At;
        SkipWhitespace(&Parser);

        polymer_unit *Unit = PushStruct(Arena, polymer_unit);
        Unit->Type = (C | 0x20);
        Unit->Polarity = ((C & 0x20) == 0);
        Unit->Prev = Prev;
        Unit->Next = 0;

        if(Prev)
        {
            Prev->Next = Unit;
        }
        else
        {
            Result.First = Unit;
        }

        Prev = Unit;
    }

    return(Result);
}

internal void
DoReactions(polymer *Polymer)
{
    for(polymer_unit *Unit1 = Polymer->First;
        Unit1 && Unit1->Next;
        )
    {
        polymer_unit *Unit2 = Unit1->Next;

        if((Unit1->Type == Unit2->Type) &&
           (Unit1->Polarity != Unit2->Polarity))
        {
            // 0 -> [1] -> 2 -> 3
            polymer_unit *Unit0 = Unit1->Prev;
            polymer_unit *Unit3 = Unit2->Next;

            if(Unit0)
            {
                Unit0->Next = Unit3;
            }
            if(Unit3)
            {
                Unit3->Prev = Unit0;
            }

            *Unit1 = *Unit2 = {};

            if(Unit0)
            {
                Unit1 = Unit0;
            }
            else
            {
                Unit1 = Unit3;
                Polymer->First = Unit3;
            }
        }
        else
        {
            Unit1 = Unit1->Next;
        }
    }
}

internal void
RemoveUnitType(polymer *Polymer, char UnitTypeToRemove)
{
    for(polymer_unit *Unit1 = Polymer->First;
        Unit1;
        )
    {
        if(Unit1->Type == UnitTypeToRemove)
        {
            // 0 -> [1] -> 2
            polymer_unit *Unit2 = Unit1->Next;
            polymer_unit *Unit0 = Unit1->Prev;

            if(Unit0)
            {
                Unit0->Next = Unit2;
            }
            if(Unit2)
            {
                Unit2->Prev = Unit0;
            }

            *Unit1 = {};

            if(Unit2)
            {
                Unit1 = Unit2;
            }
            if(!Unit0)
            {
                Polymer->First = Unit2;
            }
        }
        else
        {
            Unit1 = Unit1->Next;
        }
    }
}

internal u32
CountUnits(polymer Polymer)
{
    u32 Result = 0;
    for(polymer_unit *Unit = Polymer.First;
        Unit;
        Unit = Unit->Next)
    {
        ++Result;
    }
    return(Result);
}

internal void
SolvePart1(memory_arena *Arena, char *Input)
{
    temporary_memory TempMem = BeginTemporaryMemory(Arena);
    polymer Polymer = ParsePolymer(Arena, Input);
    DoReactions(&Polymer);
    u32 Count = CountUnits(Polymer);
    printf("%u\n", Count);
    EndTemporaryMemory(TempMem);
}

internal void
SolvePart2(memory_arena *Arena, char *Input)
{
    u32 MinCount = 0xFFFFFFFF;
    for(char UnitTypeToRemove = 'a';
        UnitTypeToRemove <= 'z';
        ++UnitTypeToRemove)
    {
        temporary_memory TempMem = BeginTemporaryMemory(Arena);
        polymer Polymer = ParsePolymer(Arena, Input);
        RemoveUnitType(&Polymer, UnitTypeToRemove);
        DoReactions(&Polymer);
        u32 Count = CountUnits(Polymer);
        if(Count < MinCount)
        {
            MinCount = Count;
        }
        EndTemporaryMemory(TempMem);
    }
    printf("%u\n", MinCount);
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

        SolvePart1(&Arena, Input);
        SolvePart2(&Arena, Input);

        CheckArena(&Arena);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
