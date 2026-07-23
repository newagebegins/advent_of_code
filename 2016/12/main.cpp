#include "lib.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>

internal void
ExecuteProgram(computer_state *State, u32 InstructionCount, instruction *Instructions)
{
    while(State->InstructionIndex < InstructionCount)
    {
        b32 GoToNextInstruction = true;
        instruction *Instruction = Instructions + State->InstructionIndex;

        switch(Instruction->Type)
        {
            case Instruction_cpy_const:
            {
                State->Registers[Instruction->Y] = Instruction->X;
            } break;

            case Instruction_cpy_reg:
            {
                State->Registers[Instruction->Y] = State->Registers[Instruction->X];
            } break;

            case Instruction_inc:
            {
                ++State->Registers[Instruction->X];
            } break;

            case Instruction_dec:
            {
                --State->Registers[Instruction->X];
            } break;

            case Instruction_jnz_const:
            {
                if(Instruction->X)
                {
                    State->InstructionIndex += Instruction->Y;
                    GoToNextInstruction = false;
                }
            } break;

            case Instruction_jnz_reg:
            {
                if(State->Registers[Instruction->X])
                {
                    State->InstructionIndex += Instruction->Y;
                    GoToNextInstruction = false;
                }
            } break;

            InvalidDefaultCase;
        }

        if(GoToNextInstruction)
        {
            ++State->InstructionIndex;
        }
    }
}

struct parsed_input
{
    u32 InstructionCount;
    instruction *Instructions;
};

struct parse_context
{
    char *At;
};

inline b32
IsWhiteSpace(char C)
{
    b32 Result = ((C == ' ') ||
                  (C == '\n') ||
                  (C == '\t'));
    return(Result);
}

inline void
SkipWhiteSpace(parse_context *Ctx)
{
    while(IsWhiteSpace(Ctx->At[0]))
    {
        ++Ctx->At;
    }    
}

inline str
GetWord(parse_context *Ctx)
{
    SkipWhiteSpace(Ctx);

    str Result;
    Result.Text = Ctx->At;
    while(Ctx->At[0] && !IsWhiteSpace(Ctx->At[0]))
    {
        ++Ctx->At;
    }
    Result.Length = (u32)(Ctx->At - Result.Text);

    return(Result);
}

internal b32
StringsAreEqual(str A, str B)
{
    b32 Result = true;
    if(A.Length == B.Length)
    {
        for(u32 Index = 0;
            Index < A.Length;
            ++Index)
        {
            if(A.Text[Index] != B.Text[Index])
            {
                Result = false;
                break;
            }
        }
    }
    else
    {
        Result = false;
    }
    return(Result);
}

inline u32
StringLength(char *Text)
{
    u32 Result = 0;
    while(*Text++)
    {
        ++Result;
    }
    return(Result);
}

inline str
Str(char *Text)
{
    str Result;
    Result.Text = Text;
    Result.Length = StringLength(Text);
    return(Result);
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

internal s32
ParseInt32(str S)
{
    Assert(S.Length > 0);
    s32 Result = 0;
    s32 Sign = 1;
    u32 At = 0;

    if(S.Text[0] == '-')
    {
        Sign = -1;
        ++At;
    }

    for(;
        At < S.Length;
        ++At)
    {
        char C = S.Text[At];
        Assert(IsDigit(C));
        Result = Result*10 + (C - '0');
    }

    Result *= Sign;

    return(Result);
}

internal s32
ParseRegister(str S)
{
    Assert(S.Length == 1);
    s32 Result = S.Text[0] - 'a';
    Assert(Result < MAX_REGISTERS);
    return(Result);
}

internal parsed_input
ParseInput(memory_arena *Arena, char *Input)
{
    parsed_input Result = {};
    Result.Instructions = (instruction *)((u8 *)Arena->Base + Arena->Used);

    parse_context Context = {};
    Context.At = Input;

    str CpyStr = Str("cpy");
    str IncStr = Str("inc");
    str DecStr = Str("dec");
    str JnzStr = Str("jnz");

    while(*Context.At)
    {
        ++Result.InstructionCount;
        instruction *Inst = PushStruct(Arena, instruction);
        str Word = GetWord(&Context);

        if(StringsAreEqual(Word, CpyStr))
        {
            Word = GetWord(&Context);
            if(IsDigit(Word.Text[0]))
            {
                Inst->Type = Instruction_cpy_const;
                Inst->X = ParseInt32(Word);
            }
            else
            {
                Inst->Type = Instruction_cpy_reg;
                Inst->X = ParseRegister(Word);
            }
            Word = GetWord(&Context);
            Inst->Y = ParseRegister(Word);
        }
        else if(StringsAreEqual(Word, IncStr))
        {
            Inst->Type = Instruction_inc;

            Word = GetWord(&Context);
            Inst->X = ParseRegister(Word);
        }
        else if(StringsAreEqual(Word, DecStr))
        {
            Inst->Type = Instruction_dec;

            Word = GetWord(&Context);
            Inst->X = ParseRegister(Word);
        }
        else if(StringsAreEqual(Word, JnzStr))
        {
            Word = GetWord(&Context);
            if(IsDigit(Word.Text[0]))
            {
                Inst->Type = Instruction_jnz_const;
                Inst->X = ParseInt32(Word);
            }
            else
            {
                Inst->Type = Instruction_jnz_reg;
                Inst->X = ParseRegister(Word);
            }

            Word = GetWord(&Context);
            Inst->Y = ParseInt32(Word);
        }
        else
        {
            InvalidCodePath;
        }

        SkipWhiteSpace(&Context);
    }

    return(Result);
}

int main(void)
{
    char *TestInput = R"(cpy 41 a
inc a
inc a
dec a
jnz a 2
dec a)";
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(1);
    void *ArenaBase = malloc(ArenaSize);
    Assert(ArenaBase);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

    parsed_input ParsedInput = ParseInput(&Arena, TestInput);
    computer_state State = {};
    ExecuteProgram(&State, ParsedInput.InstructionCount, ParsedInput.Instructions);
    Assert(State.Registers[0] == 42);

    char *Input = R"(cpy 1 a
cpy 1 b
cpy 26 d
jnz c 2
jnz 1 5
cpy 7 c
inc d
dec c
jnz c -2
cpy a c
inc a
dec b
jnz b -2
cpy c b
dec d
jnz d -6
cpy 17 c
cpy 18 d
inc a
dec d
jnz d -2
dec c
jnz c -5)";
    ParsedInput = ParseInput(&Arena, Input);

    State = {};
    ExecuteProgram(&State, ParsedInput.InstructionCount, ParsedInput.Instructions);
    Assert(State.Registers[0] == 318117);

    State = {};
    State.Registers[2] = 1;
    ExecuteProgram(&State, ParsedInput.InstructionCount, ParsedInput.Instructions);
    Assert(State.Registers[0] == 9227771);

    return(0);
}
