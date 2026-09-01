#include "lib.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>

inline b32
IsValidRegisterIndex(s32 Index)
{
    b32 Result = ((Index >= 0) && (Index < MAX_REGISTERS));
    return(Result);
}

inline s32
GetRegister(computer_state *State, s32 RegisterIndex)
{
    Assert(IsValidRegisterIndex(RegisterIndex));
    s32 Result = State->Registers[RegisterIndex];
    return(Result);
}

inline void
SetRegister(computer_state *State, s32 RegisterIndex, s32 Value)
{
    Assert(IsValidRegisterIndex(RegisterIndex));
    State->Registers[RegisterIndex] = Value;
}

inline void
IncrementRegister(computer_state *State, s32 RegisterIndex)
{
    Assert(IsValidRegisterIndex(RegisterIndex));
    ++State->Registers[RegisterIndex];
}

inline void
DecrementRegister(computer_state *State, s32 RegisterIndex)
{
    Assert(IsValidRegisterIndex(RegisterIndex));
    --State->Registers[RegisterIndex];
}

inline s32
GetValue(computer_state *State, argument Argument)
{
    s32 Result = 0;
    switch(Argument.Type)
    {
        case Argument_Constant:
        {
            Result = Argument.Value;
        } break;

        case Argument_Register:
        {
            Result = GetRegister(State, Argument.Value);
        } break;

        InvalidDefaultCase;
    }
    return(Result);
}

internal void
Toggle(instruction *Instruction)
{
    if(Instruction->Arguments[1].Type == Argument_None)
    {
        // NOTE(slava): One-argument instruction
        if(Instruction->Type == Instruction_inc)
        {
            Instruction->Type = Instruction_dec;
        }
        else
        {
            Instruction->Type = Instruction_inc;
        }
    }
    else
    {
        // NOTE(slava): Two-argument instruction
        Assert(Instruction->Arguments[0].Type != Argument_None);
        if(Instruction->Type == Instruction_jnz)
        {
            Instruction->Type = Instruction_cpy;
        }
        else
        {
            Instruction->Type = Instruction_jnz;
        }
    }
}

internal void
PrintInstructionsAndState(computer_state *State, u32 InstructionCount, instruction *Instructions)
{
    for(u32 RegisterIndex = 0;
        RegisterIndex < MAX_REGISTERS;
        ++RegisterIndex)
    {
        printf("%c: %d\n", 'a' + RegisterIndex, State->Registers[RegisterIndex]);
    }
    printf("\n");
    char *InstructionName[] = {"cpy", "inc", "dec", "jnz", "tgl"};
    for(u32 InstructionIndex = 0;
        InstructionIndex < InstructionCount;
        ++InstructionIndex)
    {
        instruction *Instruction = Instructions + InstructionIndex;
        if(InstructionIndex == State->InstructionIndex)
        {
            printf("> ");
        }
        else
        {
            printf("  ");
        }
        printf("%s", InstructionName[Instruction->Type]);
        for(u32 ArgumentIndex = 0;
            ArgumentIndex < ArrayCount(Instruction->Arguments);
            ++ArgumentIndex)
        {
            argument Arg = Instruction->Arguments[ArgumentIndex];
            switch(Arg.Type)
            {
                case Argument_Constant:
                {
                    printf(" %d", Arg.Value);
                } break;

                case Argument_Register:
                {
                    printf(" %c", 'a' + Arg.Value);
                } break;
            }
        }
        printf("\n");
    }
    printf("\n");
}

struct is_add_result
{
    b32 IsAdd;
    u32 SumR;
    u32 AddendR;
};

internal is_add_result
IsAdd(computer_state *State, u32 InstructionCount, instruction *Instructions)
{
    is_add_result Result = {};
    if((State->InstructionIndex + 2) < InstructionCount)
    {
        instruction I0 = Instructions[State->InstructionIndex];
        instruction I1 = Instructions[State->InstructionIndex + 1];
        instruction I2 = Instructions[State->InstructionIndex + 2];

        u32 SumR = MAX_REGISTERS;
        u32 AddendR = MAX_REGISTERS;

        if((I0.Type == Instruction_inc) && (I1.Type == Instruction_dec))
        {
            SumR = I0.Arguments[0].Value;
            AddendR = I1.Arguments[0].Value;
        }
        else if((I1.Type == Instruction_inc) && (I0.Type == Instruction_dec))
        {
            SumR = I1.Arguments[0].Value;
            AddendR = I0.Arguments[0].Value;
        }

        if((SumR < MAX_REGISTERS) &&
           (AddendR < MAX_REGISTERS) &&
           (SumR != AddendR) &&
           (I2.Type == Instruction_jnz) &&
           ((u32)I2.Arguments[0].Value == AddendR) &&
           (I2.Arguments[1].Value == -2))
        {
            Result.IsAdd = true;
            Result.SumR = SumR;
            Result.AddendR = AddendR;
        }
    }
    return(Result);
}

struct is_multiply_result
{
    b32 IsMultiply;
    u32 MultiplierR;
};

internal is_multiply_result
IsMultiply(computer_state *State, u32 InstructionCount, instruction *Instructions,
           u32 SumR, u32 AddendR)
{
    is_multiply_result Result = {};
    if((State->InstructionIndex + 4) < InstructionCount)
    {
        instruction I3 = Instructions[State->InstructionIndex + 3];
        instruction I4 = Instructions[State->InstructionIndex + 4];
        if((I3.Type == Instruction_dec) &&
           (I4.Type == Instruction_jnz))
        {
            u32 MultiplierR = I3.Arguments[0].Value;
            if((MultiplierR != SumR) &&
               (MultiplierR != AddendR) &&
               (I4.Arguments[0].Type == Argument_Register) &&
               ((u32)I4.Arguments[0].Value == MultiplierR) &&
               (I4.Arguments[1].Value == -5))
            {
                Result.IsMultiply = true;
                Result.MultiplierR = MultiplierR;
            }
        }
    }
    return(Result);
}

internal void
ExecuteProgram(computer_state *State, u32 InstructionCount, instruction *Instructions)
{
    while(State->InstructionIndex < InstructionCount)
    {
        //PrintInstructionsAndState(State, InstructionCount, Instructions);

        is_add_result IsAddResult = IsAdd(State, InstructionCount, Instructions);
        if(IsAddResult.IsAdd)
        {
            s32 DeltaSum = State->Registers[IsAddResult.AddendR];
            s32 DeltaInstructionIndex = 3;
            is_multiply_result IsMulRes = IsMultiply(State, InstructionCount, Instructions,
                                                     IsAddResult.SumR, IsAddResult.AddendR);
            State->Registers[IsAddResult.AddendR] = 0;

            if(IsMulRes.IsMultiply)
            {
                DeltaSum *= State->Registers[IsMulRes.MultiplierR];
                State->Registers[IsMulRes.MultiplierR] = 0;
                DeltaInstructionIndex = 5;
                //printf("Found Multiply\n");
            }
            else
            {
                //printf("Found Add\n");
            }

            State->Registers[IsAddResult.SumR] += DeltaSum;
            State->InstructionIndex += DeltaInstructionIndex;
        }
        else
        {
            b32 GoToNextInstruction = true;
            instruction *Instruction = Instructions + State->InstructionIndex;

            switch(Instruction->Type)
            {
                case Instruction_cpy:
                {
                    if(Instruction->Arguments[1].Type == Argument_Register)
                    {
                        s32 NewValue = GetValue(State, Instruction->Arguments[0]);
                        SetRegister(State,
                                    Instruction->Arguments[1].Value,
                                    NewValue);
                    }
                } break;

                case Instruction_inc:
                {
                    Assert(Instruction->Arguments[0].Type == Argument_Register);
                    Assert(Instruction->Arguments[1].Type == Argument_None);
                    IncrementRegister(State, Instruction->Arguments[0].Value);
                } break;

                case Instruction_dec:
                {
                    Assert(Instruction->Arguments[0].Type == Argument_Register);
                    Assert(Instruction->Arguments[1].Type == Argument_None);
                    DecrementRegister(State, Instruction->Arguments[0].Value);
                } break;

                case Instruction_jnz:
                {
                    s32 Condition = GetValue(State, Instruction->Arguments[0]);
                    if(Condition)
                    {
                        s32 Offset = GetValue(State, Instruction->Arguments[1]);
                        State->InstructionIndex += Offset;
                        GoToNextInstruction = false;
                    }
                } break;

                case Instruction_tgl:
                {
                    Assert(Instruction->Arguments[0].Type == Argument_Register);
                    Assert(Instruction->Arguments[1].Type == Argument_None);
                    s32 Offset = GetValue(State, Instruction->Arguments[0]);
                    u32 InstructionIndex = State->InstructionIndex + Offset;
                    if(InstructionIndex < InstructionCount)
                    {
                        Toggle(Instructions + InstructionIndex);
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

    //PrintInstructionsAndState(State, InstructionCount, Instructions);
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
    Assert(IsValidRegisterIndex(Result));
    return(Result);
}

internal argument
ParseArgument(parse_context *Context)
{
    argument Result = {};
    str Word = GetWord(Context);
    if(IsDigit(Word.Text[0]) || (Word.Text[0] == '-'))
    {
        Result.Type = Argument_Constant;
        Result.Value = ParseInt32(Word);
    }
    else
    {
        Result.Type = Argument_Register;
        Result.Value = ParseRegister(Word);
    }
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
    str TglStr = Str("tgl");

    while(*Context.At)
    {
        ++Result.InstructionCount;

        instruction *Inst = PushStruct(Arena, instruction);
        *Inst = {};

        str Command = GetWord(&Context);
        u32 ArgCount = ArrayCount(Inst->Arguments);

        if(StringsAreEqual(Command, CpyStr))
        {
            Inst->Type = Instruction_cpy;
            ArgCount = 2;
        }
        else if(StringsAreEqual(Command, IncStr))
        {
            Inst->Type = Instruction_inc;
            ArgCount = 1;
        }
        else if(StringsAreEqual(Command, DecStr))
        {
            Inst->Type = Instruction_dec;
            ArgCount = 1;
        }
        else if(StringsAreEqual(Command, JnzStr))
        {
            Inst->Type = Instruction_jnz;
            ArgCount = 2;
        }
        else if(StringsAreEqual(Command, TglStr))
        {
            Inst->Type = Instruction_tgl;
            ArgCount = 1;
        }
        else
        {
            InvalidCodePath;
        }

        Assert(ArgCount <= ArrayCount(Inst->Arguments));

        for(u32 ArgIndex = 0;
            ArgIndex < ArgCount;
            ++ArgIndex)
        {
            Inst->Arguments[ArgIndex] = ParseArgument(&Context);
        }

        SkipWhiteSpace(&Context);
    }

    return(Result);
}

internal void
Day12Tests(memory_arena *Arena)
{
    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    char *TestInput = R"(cpy 41 a
inc a
inc a
dec a
jnz a 2
dec a)";

    parsed_input ParsedInput = ParseInput(Arena, TestInput);
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
    ParsedInput = ParseInput(Arena, Input);

    State = {};
    ExecuteProgram(&State, ParsedInput.InstructionCount, ParsedInput.Instructions);
    Assert(State.Registers[0] == 318117);

    State = {};
    State.Registers[2] = 1;
    ExecuteProgram(&State, ParsedInput.InstructionCount, ParsedInput.Instructions);
    Assert(State.Registers[0] == 9227771);

    EndTemporaryMemory(TempMem);
}

int main(void)
{
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(1);
    void *ArenaBase = malloc(ArenaSize);
    Assert(ArenaBase);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

#if 1
    Day12Tests(&Arena);

    {
        char *Input = R"(cpy 2 a
tgl a
tgl a
tgl a
cpy 1 a
dec a
dec a)";

        parsed_input ParsedInput = ParseInput(&Arena, Input);
        computer_state State = {};
        ExecuteProgram(&State, ParsedInput.InstructionCount, ParsedInput.Instructions);
        Assert(State.Registers[0] == 3);
    }
#endif

    char *Input = R"(cpy a b
dec b
cpy a d
cpy 0 a
cpy b c
inc a
dec c
jnz c -2
dec d
jnz d -5
dec b
cpy b c
cpy c d
dec d
inc c
jnz d -2
tgl c
cpy -16 c
jnz 1 c
cpy 93 c
jnz 80 d
inc a
inc d
jnz d -2
inc c
jnz c -5)";

#if 1
    {
        parsed_input ParsedInput = ParseInput(&Arena, Input);
        computer_state State = {};
        State.Registers[0] = 7;
        ExecuteProgram(&State, ParsedInput.InstructionCount, ParsedInput.Instructions);
        Assert(State.Registers[0] == 12480);
    }
#endif

#if 1
    {
        parsed_input ParsedInput = ParseInput(&Arena, Input);
        computer_state State = {};
        State.Registers[0] = 12;
        ExecuteProgram(&State, ParsedInput.InstructionCount, ParsedInput.Instructions);
        Assert(State.Registers[0] == 479009040);
    }
#endif

    return(0);
}
