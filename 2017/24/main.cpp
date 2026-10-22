#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t b32;
typedef size_t memory_index;

#define internal static
#define local_persist static

#define Assert(C) if(!(C)) { *(int *)0 = 0; }

struct memory_arena
{
    memory_index Used;
    memory_index Size;
    void *Base;
    u32 TempCount;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Used = 0;
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->TempCount = 0;
}

struct temporary_memory
{
    memory_arena *Arena;
    memory_index OldUsed;
};

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    Result.Arena = Arena;
    Result.OldUsed = Arena->Used;
    ++Arena->TempCount;
    return(Result);
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
    TempMem.Arena->Used = TempMem.OldUsed;
    --TempMem.Arena->TempCount;
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, Count*sizeof(type))

inline void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = (u8 *)Arena->Base + Arena->Used;
    Arena->Used += Size;
    return(Result);
}

internal char *
ReadEntireFileAndNullTerminate(memory_arena *Arena, char *InputFile)
{
    char *Result = 0;
    FILE *File = fopen(InputFile, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);
    Result = PushArray(Arena, FileSize + 1, char);
    size_t ReadItems = fread(Result, FileSize, 1, File);
    Assert(ReadItems == 1);
    Result[FileSize] = 0;
    return(Result);
}

struct component
{
    u8 Ports[2];
};

inline u32
GetComponentStrength(component Comp)
{
    u32 Result = Comp.Ports[0] + Comp.Ports[1];
    return(Result);
}

inline b32
CanConnect(component Comp, u8 Port)
{
    b32 Result = ((Comp.Ports[0] == Port) || (Comp.Ports[1] == Port));
    return(Result);
}

inline u8
GetEndPort(component Comp, u8 StartPort)
{
    u8 Result = ((Comp.Ports[0] == StartPort) ? Comp.Ports[1] : Comp.Ports[0]);
    return(Result);
}

internal void
CopyComponentsWithout(u32 SourceComponentCount, component *SourceComponents,
                      component *DestComponents, u32 ExcludedComponentIndex)
{
    component *Dest = DestComponents;
    for(u32 ComponentIndex = 0;
        ComponentIndex < ExcludedComponentIndex;
        ++ComponentIndex)
    {
        *Dest++ = SourceComponents[ComponentIndex];
    }
    for(u32 ComponentIndex = ExcludedComponentIndex + 1;
        ComponentIndex < SourceComponentCount;
        ++ComponentIndex)
    {
        *Dest++ = SourceComponents[ComponentIndex];
    }
}

internal u32
FindMaxBridgeStrength(memory_arena *Arena, u32 ComponentCount, component *Components,
                      u8 StartPort)
{
    u32 Result = 0;
    for(u32 ComponentIndex = 0;
        ComponentIndex < ComponentCount;
        ++ComponentIndex)
    {
        component Component = Components[ComponentIndex];
        if(CanConnect(Component, StartPort))
        {
            temporary_memory TempMem = BeginTemporaryMemory(Arena);
            u8 EndPort = GetEndPort(Component, StartPort);
            u32 RemainingComponentCount = ComponentCount - 1;
            component *RemainingComponents = PushArray(Arena, RemainingComponentCount, component);
            CopyComponentsWithout(ComponentCount, Components, RemainingComponents, ComponentIndex);
            u32 Strength = GetComponentStrength(Component);
            Strength += FindMaxBridgeStrength(Arena, RemainingComponentCount, RemainingComponents,
                                              EndPort);
            if(Result < Strength)
            {
                Result = Strength;
            }
            EndTemporaryMemory(TempMem);
        }
    }
    return(Result);
}

struct parser
{
    char *At;
};

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') || (C == '\t') || (C == '\r') || (C == '\n'));
    return(Result);
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

inline void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

inline void
SkipChar(parser *Parser, char Char)
{
    Assert(Parser->At[0] == Char);
    ++Parser->At;
}

inline u8
ParseU8(parser *Parser)
{
    u8 Result = 0;
    while(IsDigit(Parser->At[0]))
    {
        Result = 10*Result + (Parser->At[0] - '0');
        ++Parser->At;
    }
    return(Result);
}

struct parsed_input
{
    u32 ComponentCount;
    component *Components;
};

inline u32
CountLines(char *Text)
{
    u32 Result = 0;
    for(char *At = Text;
        *At;
        ++At)
    {
        if(*At == '\n')
        {
            ++Result;
        }
    }
    return(Result);
}

internal parsed_input
ParseInput(memory_arena *Arena, char *Input)
{
    parsed_input Result;
    Result.ComponentCount = CountLines(Input);
    Result.Components = PushArray(Arena, Result.ComponentCount, component);

    u32 ComponentCount = 0;

    parser Parser;
    Parser.At = Input;

    SkipWhitespace(&Parser);

    while(Parser.At[0])
    {
        component *Component = Result.Components + ComponentCount++;
        Component->Ports[0] = ParseU8(&Parser);
        SkipChar(&Parser, '/');
        Component->Ports[1] = ParseU8(&Parser);
        SkipWhitespace(&Parser);
    }

    Assert(ComponentCount == Result.ComponentCount);

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

    parsed_input ParsedInput = ParseInput(&Arena, Input);

    u32 MaxStrength = FindMaxBridgeStrength(&Arena, ParsedInput.ComponentCount, ParsedInput.Components, 0);
    printf("%u\n", MaxStrength);

    CheckArena(&Arena);

    return(0);
}
