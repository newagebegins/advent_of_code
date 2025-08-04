#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef int8_t i8;
typedef uint8_t u8;

typedef int16_t i16;
typedef uint16_t u16;

typedef int32_t i32;
typedef uint32_t u32;

typedef int64_t i64;
typedef uint64_t u64;

typedef i32 b32;

#define Assert(X) if (!(X)) { *(char*)0 = 0; }

struct arena
{
    u8* Base;
    u64 Size;
    u64 Used;
};

u8* PushSize(arena* Arena, u64 Size)
{
    Assert(Arena->Used + Size <= Arena->Size);
    u8* Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    return Result;
}

#define PushArray(Arena, Type, Count) (Type*) PushSize((Arena), sizeof(Type) * (Count))

struct str
{
    const char* P;
    i32 Len;
};

enum inst_type
{
    It_hlf,
    It_tpl,
    It_inc,
    It_jmp,
    It_jie,
    It_jio,
};

i32 GetCStrLen(const char* S)
{
    i32 Result = 0;
    while (*S)
    {
        ++Result;
        ++S;
    }
    return Result;
}

str MakeStr(const char* S)
{
    str Result;
    Result.P = S;
    Result.Len = GetCStrLen(S);
    return Result;
}

str hlfStr = MakeStr("hlf");
str tplStr = MakeStr("tpl");
str incStr = MakeStr("inc");
str jmpStr = MakeStr("jmp");
str jieStr = MakeStr("jie");
str jioStr = MakeStr("jio");

enum reg
{
    Reg_a,
    Reg_b,
};

str aStr = MakeStr("a");
str bStr = MakeStr("b");

struct inst
{
    inst_type Type;
    reg R;
    int32_t Offset;
};

struct state
{
    union
    {
        struct
        {
            u32 A;
            u32 B;
        };
        u32 Regs[2];
    };
    i32 InstIdx;
    i32 I;
};

b32 IsWhitespace(char C)
{
    b32 Result;
    switch (C)
    {
        case ' ':
        case '\t':
        case '\n':
        Result = true;
        break;
        default:
        Result = false;
        break;
    }
    return Result;
}

str SkipWhitespace(str Str)
{
    while (Str.Len > 0 && IsWhitespace(*Str.P))
    {
        Str.P++;
        Str.Len--;
    }
    return Str;
}

struct get_next_word_result
{
    b32 Success;
    str Word;
    str Remainder;
};

get_next_word_result GetNextWord(str Str)
{
    get_next_word_result Result = {};
    Str = SkipWhitespace(Str);
    if (Str.Len > 0)
    {
        Result.Success = true;
        Result.Word.P = Str.P;
        while (Str.Len > 0 && !IsWhitespace(*Str.P))
        {
            Str.P++;
            Str.Len--;
            Result.Word.Len++;
        }
        Result.Remainder = Str;
    }
    return Result;
}

struct parse_context
{
    b32 Success;
    i32 LineIdx;
    i32 ErrorLine;
    i32 ErrorCol;
    char* ErrorText;
    i32 MaxErrorTextSize;
    str Line;
    arena* Arena;
};

struct parse_reg_result
{
    reg R;
    b32 Success;
    str Remainder;
};

void Error(parse_context* Ctx, str ParsedStr, const char* format, ...)
{
    Ctx->Success = false;
    Ctx->ErrorLine = Ctx->LineIdx;
    Ctx->ErrorCol = ParsedStr.P - Ctx->Line.P;
    
    va_list vlist;
    va_start(vlist, format);
    vsnprintf(Ctx->ErrorText, Ctx->MaxErrorTextSize, format, vlist);
    va_end(vlist);
}

b32 StrEqual(str A, str B)
{
    b32 Result = true;;
    if (A.Len == B.Len)
    {
        for (i32 I = 0; I < A.Len; ++I)
        {
            if (A.P[I] != B.P[I])
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
    return Result;
}

char* ToCStr(arena* Arena, str Str)
{
    char* Result = PushArray(Arena, char, Str.Len + 1);
    for (i32 I = 0; I < Str.Len; ++I)
    {
        Result[I] = Str.P[I];
    }
    Result[Str.Len] = 0;
    return Result;
}

parse_reg_result ParseReg(parse_context* Ctx, str Str)
{
    parse_reg_result Result = {};
    Str = SkipWhitespace(Str);
    if (Str.Len > 0)
    {
        switch (Str.P[0])
        {
            case 'a':
            {
                Result.R = Reg_a;
                Result.Success = true;
            } break;
            case 'b':
            {
                Result.R = Reg_b;
                Result.Success = true;
            } break;
            default:
            {
                Error(Ctx, Str, "Unknown register: %c", Str.P[0]);
            } break;
        }
    }
    else
    {
        Error(Ctx, Str, "Expected a register, found nothing");
    }
    if (Result.Success)
    {
        Result.Remainder = Str;
        Result.Remainder.P++;
        Result.Remainder.Len--;
    }
    
    return Result;
}

struct parse_sign_result
{
    b32 Success;
    i32 Sign;
    str Remainder;
};

parse_sign_result ParseSign(parse_context* Ctx, str Str)
{
    parse_sign_result Result = {};
    Assert(Str.Len > 0);
    switch (Str.P[0])
    {
        case '+':
        {
            Result.Success = true;
            Result.Sign = 1;
        } break;
        case '-':
        {
            Result.Success = true;
            Result.Sign = -1;
        } break;
        default:
        {
            Error(Ctx, Str, "Expected sign, found: %s", ToCStr(Ctx->Arena, Str));
        } break;
    }
    if (Result.Success)
    {
        Result.Remainder = Str;
        Result.Remainder.P++;
        Result.Remainder.Len--;
    }
    return Result;
}

b32 IsInteger(str Str)
{
    b32 Result = true;
    for (i32 I = 0; I < Str.Len; ++I)
    {
        if ('0' <= Str.P[I] && Str.P[I] <= '9')
        {
            // OK
        }
        else
        {
            Result = false;
            break;
        }
    }
    return Result;
}

struct parse_integer_result
{
    b32 Success;
    i32 Integer;
    str Remainder;
};

parse_integer_result ParseInteger(parse_context* Ctx, str Str)
{
    parse_integer_result Result = {};
    get_next_word_result WordResult = GetNextWord(Str);
    if (WordResult.Success)
    {
        if (IsInteger(WordResult.Word))
        {
            i32 Integer = 0;
            for (i32 I = 0; I < WordResult.Word.Len; ++I)
            {
                Integer = Integer * 10 + (WordResult.Word.P[I] - '0');
            }
            Result.Success = true;
            Result.Integer = Integer;
            Result.Remainder = WordResult.Remainder;
        }
        else
        {
            Error(Ctx, Str, "Expected an integer, found: %s", ToCStr(Ctx->Arena, WordResult.Word));
        }
    }
    else
    {
        Error(Ctx, Str, "Missing integer");
    }
    return Result;
}

struct parse_offset_result
{
    b32 Success;
    i32 Offset;
};

parse_offset_result ParseOffset(parse_context* Ctx, str Str)
{
    parse_offset_result Result = {};
    get_next_word_result OffsetWordResult = GetNextWord(Str);
    if (OffsetWordResult.Success)
    {
        parse_sign_result SignResult = ParseSign(Ctx, OffsetWordResult.Word);
        if (SignResult.Success)
        {
            parse_integer_result IntegerResult = ParseInteger(Ctx, SignResult.Remainder);
            if (IntegerResult.Success)
            {
                Result.Success = true;
                Result.Offset = SignResult.Sign * IntegerResult.Integer;
            }
        }
    }
    else
    {
        Error(Ctx, Str, "Expected an offset, found nothing");
    }
    return Result;
}

struct skip_comma_result
{
    b32 Success;
    str Remainder;
};

skip_comma_result SkipComma(parse_context* Ctx, str Str)
{
    skip_comma_result Result = {};
    Str = SkipWhitespace(Str);
    if (Str.Len > 0 && Str.P[0] == ',')
    {
        Result.Success = true;
        Result.Remainder.P = Str.P + 1;
        Result.Remainder.Len = Str.Len - 1;
    }
    else
    {
        Error(Ctx, Str, "Missing comma");
    }
    return Result;
}

struct program
{
    inst* Instructions;
    i32 InstCount;
};

i32 CountNonEmptyLines(str Str)
{
    i32 Result = 0;
    b32 IsEmpty = true;
    for (i32 I = 0; I < Str.Len; ++I)
    {
        if (IsWhitespace(Str.P[I]))
        {
            if (Str.P[I] == '\n')
            {
                if (!IsEmpty)
                {
                    ++Result;
                }
                IsEmpty = true;
            }
        }
        else
        {
            IsEmpty = false;
        }
    }
    return Result;
}

struct get_next_line_result
{
    str Line;
    str Remainder;
};

get_next_line_result GetNextLine(str Str)
{
    get_next_line_result Result = {};
    Result.Line.P = Str.P;
    i32 I;
    for (I = 0; I < Str.Len && Str.P[I] != '\n'; ++I)
    {
    }
    Result.Line.Len = I;
    
    Result.Remainder.P = Str.P + I;
    Result.Remainder.Len = Str.Len - I;
    if (I < Str.Len)
    {
        // Skip the newline char
        Result.Remainder.P++;
        Result.Remainder.Len--;
    }
    
    return Result;
}

struct parse_result
{
    program Program;
    
    b32 Success;
    
    i32 ErrorLine;
    i32 ErrorCol;
    const char* ErrorText;
};

parse_result Parse(arena* Arena, str Input)
{
    parse_result Result = {};
    parse_context Ctx = {};
    Ctx.Arena = Arena;
    i32 InstCount = CountNonEmptyLines(Input);
    inst* Instructions = PushArray(Arena, inst, InstCount);
    Ctx.MaxErrorTextSize = 128;
    Ctx.ErrorText = PushArray(Arena, char, Ctx.MaxErrorTextSize);
    Ctx.LineIdx = 0;
    Ctx.Success = true;
    i32 InstIdx = 0;
    while (Ctx.Success && Input.Len > 0)
    {
        get_next_line_result GetNextLineResult = GetNextLine(Input);
        Ctx.Line = GetNextLineResult.Line;
        Input = GetNextLineResult.Remainder;
        
        get_next_word_result InstWordResult = GetNextWord(Ctx.Line);
        if (InstWordResult.Success)
        {
            inst Inst = {};
            if (StrEqual(InstWordResult.Word, hlfStr))
            {
                Inst.Type = It_hlf;
                parse_reg_result RegResult = ParseReg(&Ctx, InstWordResult.Remainder);
                if (RegResult.Success)
                {
                    Inst.R = RegResult.R;
                }
            }
            else if (StrEqual(InstWordResult.Word, tplStr))
            {
                Inst.Type = It_tpl;
                parse_reg_result RegResult = ParseReg(&Ctx, InstWordResult.Remainder);
                if (RegResult.Success)
                {
                    Inst.R = RegResult.R;
                }
            }
            else if (StrEqual(InstWordResult.Word, incStr))
            {
                Inst.Type = It_inc;
                parse_reg_result RegResult = ParseReg(&Ctx, InstWordResult.Remainder);
                if (RegResult.Success)
                {
                    Inst.R = RegResult.R;
                }
            }
            else if (StrEqual(InstWordResult.Word, jmpStr))
            {
                Inst.Type = It_jmp;
                parse_offset_result OffsetResult = ParseOffset(&Ctx, InstWordResult.Remainder);
                if (OffsetResult.Success)
                {
                    Inst.Offset = OffsetResult.Offset;
                }
            }
            else if (StrEqual(InstWordResult.Word, jieStr))
            {
                Inst.Type = It_jie;
                parse_reg_result RegResult = ParseReg(&Ctx, InstWordResult.Remainder);
                if (RegResult.Success)
                {
                    Inst.R = RegResult.R;
                    skip_comma_result CommaResult = SkipComma(&Ctx, RegResult.Remainder);
                    if (CommaResult.Success)
                    {
                        parse_offset_result OffsetResult = ParseOffset(&Ctx, CommaResult.Remainder);
                        if (OffsetResult.Success)
                        {
                            Inst.Offset = OffsetResult.Offset;
                        }
                    }
                }
            }
            else if (StrEqual(InstWordResult.Word, jioStr))
            {
                Inst.Type = It_jio;
                parse_reg_result RegResult = ParseReg(&Ctx, InstWordResult.Remainder);
                if (RegResult.Success)
                {
                    Inst.R = RegResult.R;
                    skip_comma_result CommaResult = SkipComma(&Ctx, RegResult.Remainder);
                    if (CommaResult.Success)
                    {
                        parse_offset_result OffsetResult = ParseOffset(&Ctx, CommaResult.Remainder);
                        if (OffsetResult.Success)
                        {
                            Inst.Offset = OffsetResult.Offset;
                        }
                    }
                }
            }
            else
            {
                Error(&Ctx, Ctx.Line, "Unknown instruction: %s\n", ToCStr(Ctx.Arena, InstWordResult.Word));
            }
            
            if (Ctx.Success)
            {
                Instructions[InstIdx++] = Inst;
            }
        }
        
        ++Ctx.LineIdx;
    }
    
    Assert(InstIdx == InstCount);
    
    Result.Success = Ctx.Success;
    
    if (Result.Success)
    {
        Result.Program.Instructions = Instructions;
        Result.Program.InstCount = InstCount;
    }
    else
    {
        Result.ErrorText = Ctx.ErrorText;
    }
    
    return Result;
}

void ExecuteInst(state* State, inst Inst)
{
    switch (Inst.Type)
    {
        case It_hlf:
        {
            u32* R = &State->Regs[Inst.R];
            *R /= 2;
            ++State->I;
        } break;
        case It_tpl:
        {
            u32* R = &State->Regs[Inst.R];
            *R *= 3;
            ++State->I;
        } break;
        case It_inc:
        {
            u32* R = &State->Regs[Inst.R];
            *R += 1;
            ++State->I;
        } break;
        case It_jmp:
        {
            State->I += Inst.Offset;
        } break;
        case It_jie:
        {
            u32 R = State->Regs[Inst.R];
            if ((R%2) == 0)
            {
                State->I += Inst.Offset;
            }
            else
            {
                State->I += 1;
            }
        } break;
        case It_jio:
        {
            u32 R = State->Regs[Inst.R];
            if (R == 1)
            {
                State->I += Inst.Offset;
            }
            else
            {
                State->I += 1;
            }
        } break;
        default:
        {
            Assert(!"Unknown instruction type");
        }
        break;
    }
}

void ExecuteProgram(state* State, program* Prog)
{
    while (0 <= State->I && State->I < Prog->InstCount)
    {
        ExecuteInst(State, Prog->Instructions[State->I]);
    }
}

void Run(state* State, const char* Prog)
{
    arena Arena = {};
    Arena.Size = 4*1024*1024;
    Arena.Base = (u8*)malloc(Arena.Size);
    Assert(Arena.Base);
    
    str ProgStr = MakeStr(Prog);
    
    parse_result ParseResult = Parse(&Arena, ProgStr);
    if (ParseResult.Success)
    {
        ExecuteProgram(State, &ParseResult.Program);
    }
    else
    {
        fprintf(stderr, "Parse error at Line %d, Col %d: %s\n", ParseResult.ErrorLine, ParseResult.ErrorCol, ParseResult.ErrorText);
    }
    
    free(Arena.Base);
}

int main()
{
    const char* Prog1 = R"(
        inc a
        jio a, +2
        tpl a
        inc a
        )";
    state State1 = {};
    Run(&State1, Prog1);
    Assert(State1.A == 2);
    
    const char* Prog2 = R"(
        jio a, +19
inc a
tpl a
inc a
tpl a
inc a
tpl a
tpl a
inc a
inc a
tpl a
tpl a
inc a
inc a
tpl a
inc a
inc a
tpl a
jmp +23
tpl a
tpl a
inc a
inc a
tpl a
inc a
inc a
tpl a
inc a
tpl a
inc a
tpl a
inc a
tpl a
inc a
inc a
tpl a
inc a
inc a
tpl a
tpl a
inc a
jio a, +8
inc b
jie a, +4
tpl a
inc a
jmp +2
hlf a
jmp -7
        )";
    state State2 = {};
    Run(&State2, Prog2);
    Assert(State2.B == 184);
    
    state State3 = {};
    State3.A = 1;
    Run(&State3, Prog2);
    printf("%d\n", State3.B);
    
    return 0;
}
