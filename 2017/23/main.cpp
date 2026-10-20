#include "main.h"

static char *
ReadEntireFileAndNullTerminate(char *FileName)
{
    char *Result = 0;
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        if(fseek(File, 0, SEEK_END) == 0)
        {
            long FileSize = ftell(File);
            if(FileSize >= 0)
            {
                if(fseek(File, 0, SEEK_SET) == 0)
                {
                    Result = (char *)malloc(FileSize + 1);
                    if(Result)
                    {
                        size_t ReadCount = fread(Result, 1, FileSize, File);
                        if(ReadCount == FileSize)
                        {
                            Result[FileSize] = 0;
                        }
                        else
                        {
                            free(Result);
                            Result = 0;
                        }
                    }
                }
            }
        }
        fclose(File);
    }
    return(Result);
}

inline b32
AddOverflows(register_value A, register_value B)
{
    b32 Result = (((B > 0) && (A > (MAX_REGISTER_VALUE - B))) ||
                  ((B < 0) && (A < (MIN_REGISTER_VALUE - B))));
    return(Result);
}

inline b32
SubOverflows(register_value A, register_value B)
{
    b32 Result = (((B > 0) && (A < (MIN_REGISTER_VALUE + B))) ||
                  ((B < 0) && (A > (MAX_REGISTER_VALUE + B))));
    return(Result);
}

inline b32
MulOverflows(register_value A, register_value B)
{
    b32 Result = false;
    if(A > 0)
    {
        if(B > 0)
        {
            if(A > (MAX_REGISTER_VALUE / B))
            {
                Result = true;
            }
        }
        else if(B < 0)
        {
            if(B < (MIN_REGISTER_VALUE / A))
            {
                Result = true;
            }
        }
    }
    else if (A < 0)
    {
        if(B > 0)
        {
            if(A < (MIN_REGISTER_VALUE / B))
            {
                Result = true;
            }
        }
        else if(B < 0)
        {
            if(A < (MAX_REGISTER_VALUE / B))
            {
                Result = true;
            }
        }
    }
    return(Result);
}

inline register_value
GetArgValue(program *Program, instruction_arg Arg)
{
    register_value Result = {};
    switch(Arg.Type)
    {
        case InstructionArg_Register:
        {
            Assert(Arg.RegisterIndex < ArrayCount(Program->Registers));
            Result = Program->Registers[Arg.RegisterIndex];
        } break;

        case InstructionArg_Constant:
        {
            Result = Arg.ConstantValue;
        } break;

        InvalidDefaultCase;
    }
    return(Result);
}

static void
RunInstruction(program *Program)
{
    if(Program->State == ProgramState_Running)
    {
        instruction Instruction = Program->Instructions[Program->InstructionIndex];
        b32 GoToNextInstruction = true;

        switch(Instruction.Type)
        {
            case Instruction_set:
            {
                Assert(Instruction.Args[0].Type == InstructionArg_Register);
                Assert(Instruction.Args[0].RegisterIndex < ArrayCount(Program->Registers));
                Program->Registers[Instruction.Args[0].RegisterIndex] = GetArgValue(Program, Instruction.Args[1]);
            } break;

            case Instruction_sub:
            {
                Assert(Instruction.Args[0].Type == InstructionArg_Register);
                Assert(Instruction.Args[0].RegisterIndex < ArrayCount(Program->Registers));
                register_value CurrentValue = Program->Registers[Instruction.Args[0].RegisterIndex];
                register_value SubtractedValue = GetArgValue(Program, Instruction.Args[1]);
                if(SubOverflows(CurrentValue, SubtractedValue))
                {
                    Program->State = ProgramState_Overflow;
                    GoToNextInstruction = false;
                }
                else
                {
                    Program->Registers[Instruction.Args[0].RegisterIndex] = CurrentValue - SubtractedValue;
                }
            } break;

            case Instruction_mul:
            {
                Assert(Instruction.Args[0].Type == InstructionArg_Register);
                Assert(Instruction.Args[0].RegisterIndex < ArrayCount(Program->Registers));
                register_value CurrentValue = Program->Registers[Instruction.Args[0].RegisterIndex];
                register_value Factor = GetArgValue(Program, Instruction.Args[1]);
                if(MulOverflows(CurrentValue, Factor))
                {
                    Program->State = ProgramState_Overflow;
                    GoToNextInstruction = false;
                }
                else
                {
                    Program->Registers[Instruction.Args[0].RegisterIndex] = CurrentValue * Factor;
                    ++Program->MulCount;
                }
            } break;

            case Instruction_jnz:
            {
                register_value X = GetArgValue(Program, Instruction.Args[0]);
                if(X)
                {
                    register_value Y = GetArgValue(Program, Instruction.Args[1]);
                    if(AddOverflows(Program->InstructionIndex, Y))
                    {
                        Program->State = ProgramState_Overflow;
                    }
                    else
                    {
                        Program->InstructionIndex += Y;
                    }
                    GoToNextInstruction = false;
                }
            } break;
        }

        if(GoToNextInstruction)
        {
            ++Program->InstructionIndex;
        }

        if((Program->InstructionIndex < 0) ||
           (Program->InstructionIndex >= Program->InstructionCount))
        {
            Program->State = ProgramState_InvalidInstructionIndex;
        }
    }
}

static b32
SolvePart1(parse_result ParseResult)
{
    b32 Success = true;

    program Program = {};
    Program.InstructionCount = ParseResult.InstructionCount;
    Program.Instructions = ParseResult.Instructions;

    while(Program.State == ProgramState_Running)
    {
        RunInstruction(&Program);
    }

    if(Program.State == ProgramState_Overflow)
    {
        fprintf(stderr, "Overflow\n");
        Success = false;
    }
    else
    {
        printf("%u\n", Program.MulCount);
    }

    return(Success);
}

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') || (C == '\t') || (C == '\r') || (C == '\n'));
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

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

static register_value
ParseRegisterValue(parser *Parser)
{
    register_value Result = 0;

    register_value Sign = 1;
    if(Parser->At[0] == '-')
    {
        Sign = -1;
        ++Parser->At;
    }

    while(IsDigit(Parser->At[0]))
    {
        Result = 10*Result + (Parser->At[0] - '0');
        ++Parser->At;
    }

    Result *= Sign;
    return(Result);
}

static str
ParseString(parser *Parser)
{
    str Result;
    Result.Chars = Parser->At;
    while(Parser->At[0] && !IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
    Result.Length = (u32)(Parser->At - Result.Chars);
    return(Result);
}

static token
GetToken(parser *Parser)
{
    token Result = {};
    SkipWhitespace(Parser);
    if(Parser->At[0])
    {
        if(IsDigit(Parser->At[0]) ||
           (Parser->At[0] == '-' && IsDigit(Parser->At[1])))
        {
            Result.Type = Token_Number;
            Result.Number = ParseRegisterValue(Parser);
        }
        else
        {
            Result.Type = Token_String;
            Result.String = ParseString(Parser);
        }
    }
    else
    {
        Result.Type = Token_EndOfFile;
    }
    return(Result);
}

static b32
StringsAreEqual(str A, char *B)
{
    u32 AIndex = 0;
    while((AIndex <= A.Length) && *B && (A.Chars[AIndex] == *B))
    {
        ++AIndex;
        ++B;
    }
    b32 Result = (AIndex == A.Length) && !*B;
    return(Result);
}

static instruction_arg
ParseInstructionArgument(parser *Parser, b32 RegisterOnly)
{
    instruction_arg Result = {};
    token Token = GetToken(Parser);
    switch(Token.Type)
    {
        case Token_String:
        {
            if(Token.String.Length == 1)
            {
                Result.Type = InstructionArg_Register;
                char C = Token.String.Chars[0];
                if((C >= 'a') && (C <= 'z'))
                {
                    Result.RegisterIndex = C - 'a';
                }
                else
                {
                    Parser->Error = true;
                }
            }
            else
            {
                Parser->Error = true;
            }
        } break;

        case Token_Number:
        {
            if(RegisterOnly)
            {
                Parser->Error = true;
            }
            else
            {
                Result.Type = InstructionArg_Constant;
                Result.ConstantValue = Token.Number;
            }
        } break;

        default:
        {
            Parser->Error = true;
        } break;
    }
    return(Result);
}

static instruction
ParseInstruction(parser *Parser, token Token)
{
    instruction Instruction = {};
    if((Token.Type == Token_String) && (Token.String.Length == 3))
    {
        if(StringsAreEqual(Token.String, "set"))
        {
            Instruction.Type = Instruction_set;
        }
        else if(StringsAreEqual(Token.String, "sub"))
        {
            Instruction.Type = Instruction_sub;
        }
        else if(StringsAreEqual(Token.String, "mul"))
        {
            Instruction.Type = Instruction_mul;
        }
        else if(StringsAreEqual(Token.String, "jnz"))
        {
            Instruction.Type = Instruction_jnz;
        }
        else
        {
            Parser->Error = true;
        }

        if(!Parser->Error)
        {
            instruction_blueprint Blueprint = InstructionBlueprints[Instruction.Type];
            for(u32 ArgIndex = 0;
                (ArgIndex < Blueprint.ArgCount) && !Parser->Error;
                ++ArgIndex)
            {
                Instruction.Args[ArgIndex] = ParseInstructionArgument(Parser, Blueprint.RegisterOnly[ArgIndex]);
            }
        }
    }
    else
    {
        Parser->Error = true;
    }
    return(Instruction);
}

static u32
CountNonEmptyLines(char *Input)
{
    u32 Result = 0;
    char *At = Input;
    while(*At)
    {
        if(IsWhitespace(*At))
        {
            ++At;
        }
        else
        {
            ++Result;
            while(*At && (*At != '\n'))
            {
                ++At;
            }
        }
    }
    return(Result);
}

static parse_result
ParseInstructions(char *Input)
{
    parse_result Result = {};
    u64 LineCount = CountNonEmptyLines(Input);
    Assert(sizeof(LineCount) >= sizeof(register_value));
    if(LineCount <= MAX_REGISTER_VALUE)
    {
        Result.Instructions = (instruction *)malloc(LineCount * sizeof(instruction));
        if(Result.Instructions)
        {
            parser Parser = {};
            Parser.At = Input;
            while(!Parser.Error)
            {
                token Token = GetToken(&Parser);

                if(Token.Type == Token_EndOfFile)
                {
                    break;
                }

                instruction Instruction = ParseInstruction(&Parser, Token);

                if(!Parser.Error)
                {
                    Assert((u32)Result.InstructionCount < LineCount);
                    Result.Instructions[Result.InstructionCount++] = Instruction;
                }
            }
            if(Parser.Error)
            {
                free(Result.Instructions);
                Result = {};
            }
        }
    }
    return(Result);
}

int
main(int ArgCount, char **Args)
{
    b32 Error = false;
    char *InputFileName = "input.txt";
    if(ArgCount > 1)
    {
        InputFileName = Args[1];
    }
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        parse_result ParseResult = ParseInstructions(Input);
        if(ParseResult.Instructions)
        {
            if(SolvePart1(ParseResult))
            {
                // NOTE(slava): Success!
            }
            else
            {
                Error = true;
            }
        }
        else
        {
            fprintf(stderr, "Parse error\n");
            Error = true;
        }
        free(ParseResult.Instructions);
        free(Input);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
        Error = true;
    }
    int Result = (Error ? 1 : 0);
    return(Result);
}
