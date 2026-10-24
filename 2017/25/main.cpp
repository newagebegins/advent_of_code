#include "main.h"

internal void
ReadEntireFileAndNullTerminate(char *FileName, u32 BufferSize, char *Buffer)
{
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    size_t ReadCount = fread(Buffer, 1, BufferSize - 1, File);
    Assert(ReadCount < (BufferSize - 1));
    Buffer[ReadCount] = 0;
    fclose(File);
}

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

inline void
SkipString(parser *Parser, char *String)
{
    for(char *At = String;
        *At;
        ++At)
    {
        SkipChar(Parser, *At);
    }
}

inline u32
ParseU32(parser *Parser)
{
    u32 Result = 0;
    while(IsDigit(Parser->At[0]))
    {
        Result = 10*Result + (Parser->At[0] - '0');
        ++Parser->At;
    }
    return(Result);
}

inline s32
ParseMoveDir(parser *Parser)
{
    s32 Result = 0;
    switch(Parser->At[0])
    {
        case 'l':
        {
            Result = -1;
            SkipString(Parser, "left");
        } break;

        case 'r':
        {
            Result = 1;
            SkipString(Parser, "right");
        } break;

        InvalidDefaultCase;
    }
    return(Result);
}

inline u32
ParseStateIndex(parser *Parser)
{
    char StateLetter = Parser->At[0];
    ++Parser->At;
    Assert(('A' <= StateLetter) && (StateLetter <= 'Z'));
    u32 Result = StateLetter - 'A';
    Assert(Result < ArrayCount(((blueprint *)0)->States));
    return(Result);
}

internal void
ParseStateAction(parser *Parser, state *State)
{
    SkipString(Parser, "If the current value is ");
    u32 CurrentValue = ParseU32(Parser);
    Assert(CurrentValue < ArrayCount(State->Actions));
    state_action *Action = State->Actions + CurrentValue;
    SkipChar(Parser, ':');
    SkipWhitespace(Parser);

    SkipString(Parser, "- Write the value ");
    u32 ValueToWrite = ParseU32(Parser);
    Assert(ValueToWrite <= 1);
    Action->ValueToWrite = (u8)ValueToWrite;
    SkipChar(Parser, '.');
    SkipWhitespace(Parser);

    SkipString(Parser, "- Move one slot to the ");
    Action->MoveDir = ParseMoveDir(Parser);
    SkipChar(Parser, '.');
    SkipWhitespace(Parser);

    SkipString(Parser, "- Continue with state ");
    Action->NextStateIndex = ParseStateIndex(Parser);
    SkipChar(Parser, '.');
    SkipWhitespace(Parser);
}

internal void
ParseInput(char *Input, blueprint *Blueprint)
{
    parser Parser;
    Parser.At = Input;

    SkipWhitespace(&Parser);
    SkipString(&Parser, "Begin in state ");
    Blueprint->StartStateIndex = ParseStateIndex(&Parser);
    SkipChar(&Parser, '.');

    SkipWhitespace(&Parser);
    SkipString(&Parser, "Perform a diagnostic checksum after ");
    Blueprint->DiagnosticStepCount = ParseU32(&Parser);
    SkipString(&Parser, " steps.");

    SkipWhitespace(&Parser);

    while(Parser.At[0])
    {
        SkipString(&Parser, "In state ");
        u32 StateIndex = ParseStateIndex(&Parser);
        Assert(StateIndex == Blueprint->StateCount);
        SkipChar(&Parser, ':');
        SkipWhitespace(&Parser);

        Assert(Blueprint->StateCount < ArrayCount(Blueprint->States));
        state *State = Blueprint->States + Blueprint->StateCount++;

        for(u32 ActionIndex = 0;
            ActionIndex < ArrayCount(State->Actions);
            ++ActionIndex)
        {
            ParseStateAction(&Parser, State);
        }
    }
}

internal void
Step(machine *Machine, blueprint *Blueprint)
{
    state State = Blueprint->States[Machine->CurrentStateIndex];
    u8 TapeValue = Machine->Tape[Machine->TapeAt];
    state_action Action = State.Actions[TapeValue];
    Machine->Tape[Machine->TapeAt] = Action.ValueToWrite;
    Machine->TapeAt += Action.MoveDir;
    if(Machine->TapeAt < 0)
    {
        Machine->TapeAt += ArrayCount(Machine->Tape);
    }
    else if(Machine->TapeAt >= ArrayCount(Machine->Tape))
    {
        Machine->TapeAt -= ArrayCount(Machine->Tape);
    }
    Machine->CurrentStateIndex = Action.NextStateIndex;
}

internal u32
GetDiagnosticChecksum(machine *Machine, blueprint *Blueprint)
{
    Machine->CurrentStateIndex = Blueprint->StartStateIndex;
    for(u32 StepIndex = 0;
        StepIndex < Blueprint->DiagnosticStepCount;
        ++StepIndex)
    {
        Step(Machine, Blueprint);
    }
    u32 Result = 0;
    for(u32 TapeIndex = 0;
        TapeIndex < ArrayCount(Machine->Tape);
        ++TapeIndex)
    {
        Result += Machine->Tape[TapeIndex];
    }
    return(Result);
}

int
main(int ArgCount, char **Args)
{
    local_persist char Input[4*1024];

    char *InputFile = ((ArgCount > 1) ? Args[1] : "input.txt");
    ReadEntireFileAndNullTerminate(InputFile, sizeof(Input), Input);

    local_persist blueprint Blueprint;
    local_persist machine Machine;

    ParseInput(Input, &Blueprint);
    u32 Checksum = GetDiagnosticChecksum(&Machine, &Blueprint);
    printf("%u\n", Checksum);

    return(0);
}
