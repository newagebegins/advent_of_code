#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#define internal static
#define local_persist static

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))

typedef uint32_t u32;
typedef int32_t s32;
typedef int32_t b32;

internal void
ReadEntireFileAndNullTerminate(char *FileName, char *Buffer, u32 BufferSize)
{
    char *Result = 0;
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    Assert(FileSize > 0);
    Assert(((u32)FileSize + 1) < BufferSize);
    fseek(File, 0, SEEK_SET);
    fread(Buffer, FileSize, 1, File);
    Buffer[FileSize] = 0;
    fclose(File);
}

struct str
{
    char *Chars;
    u32 Length;
};

internal b32
StringsAreEqual(str A, str B)
{
    b32 Result = false;
    if(A.Length == B.Length)
    {
        Result = true;
        for(u32 CharIndex = 0;
            CharIndex < A.Length;
            ++CharIndex)
        {
            if(A.Chars[CharIndex] != B.Chars[CharIndex])
            {
                Result = false;
                break;
            }
        }
    }
    return(Result);
}

internal b32
StringsAreEqual(str A, char *B)
{
    u32 AIndex = 0;
    while((AIndex < A.Length) && B[0] && (A.Chars[AIndex] == B[0]))
    {
        ++AIndex;
        ++B;
    }
    b32 Result = ((AIndex == A.Length) && !B[0]);
    return(Result);
}

struct parser
{
    char *At;
};

inline b32
IsWhitespace(char C)
{
    b32 Result;
    switch(C)
    {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        {
            Result = true;
        } break;

        default:
        {
            Result = false;
        } break;
    }
    return(Result);
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

inline b32
IsLowercaseLetter(char C)
{
    b32 Result = ((C >= 'a') && (C <= 'z'));
    return(Result);
}

internal str
GetName(parser *Parser)
{
    str Result;
    Result.Chars = Parser->At;
    while(IsLowercaseLetter(Parser->At[0]))
    {
        ++Parser->At;
    }
    Result.Length = (u32)(Parser->At - Result.Chars);
    Assert(Result.Length);
    return(Result);
}

internal void
SkipString(parser *Parser, char *String)
{
    for(char *At = String;
        *At;
        ++At)
    {
        Assert(Parser->At[0] == At[0]);
        ++Parser->At;
    }
}

internal u32
GetU32(parser *Parser)
{
    u32 Result = 0;
    Assert(Parser->At[0]);
    while(IsDigit(Parser->At[0]))
    {
        Result = 10*Result + (Parser->At[0] - '0');
        ++Parser->At;
    }
    return(Result);
}

internal void
SkipChar(parser *Parser, char C)
{
    Assert(Parser->At[0] == C);
    ++Parser->At;
}

internal void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

#define INVALID_PROGRAM_INDEX -1
#define INVALID_WEIGHT -1

struct program
{
    str Name;
    s32 Weight;
    s32 TotalWeight;
    s32 ParentIndex;
    s32 Children[8];
    u32 ChildCount;
};

struct program_storage
{
    program Programs[2048];
    u32 Count;
};

internal s32
FindOrCreateProgramWithName(str Name, program_storage *Storage)
{
    s32 ProgramIndex = INVALID_PROGRAM_INDEX;
    for(u32 Index = 0;
        Index < Storage->Count;
        ++Index)
    {
        if(StringsAreEqual(Name, Storage->Programs[Index].Name))
        {
            ProgramIndex = Index;
            break;
        }
    }
    if(ProgramIndex == INVALID_PROGRAM_INDEX)
    {
        ProgramIndex = Storage->Count++;
        Assert(ProgramIndex < ArrayCount(Storage->Programs));
        Storage->Programs[ProgramIndex].Name = Name;
        Storage->Programs[ProgramIndex].Weight = INVALID_WEIGHT;
        Storage->Programs[ProgramIndex].TotalWeight = INVALID_WEIGHT;
        Storage->Programs[ProgramIndex].ParentIndex = INVALID_PROGRAM_INDEX;
        Storage->Programs[ProgramIndex].ChildCount = 0;
    }
    return(ProgramIndex);
}

internal void
ParseInput(char *Input, program_storage *Storage)
{
    Storage->Count = 0;

    parser Parser;
    Parser.At = Input;

    while(Parser.At[0])
    {
        s32 ProgramIndex = FindOrCreateProgramWithName(GetName(&Parser), Storage);
        program *Program = Storage->Programs + ProgramIndex;
        SkipString(&Parser, " (");
        Program->Weight = GetU32(&Parser);
        SkipChar(&Parser, ')');
        SkipWhitespace(&Parser);
        if(Parser.At[0] == '-')
        {
            SkipString(&Parser, "-> ");
            for(;;)
            {
                s32 ChildIndex = FindOrCreateProgramWithName(GetName(&Parser), Storage);
                program *Child = Storage->Programs + ChildIndex;
                Child->ParentIndex = ProgramIndex;
                Assert(Program->ChildCount < ArrayCount(Program->Children));
                Program->Children[Program->ChildCount++] = ChildIndex;
                if(Parser.At[0] == ',')
                {
                    SkipString(&Parser, ", ");
                }
                else
                {
                    SkipWhitespace(&Parser);
                    break;
                }
            }
        }
    }
}

internal s32
FindBottomProgramIndex(program_storage *Storage)
{
    s32 Result = INVALID_PROGRAM_INDEX;
    for(u32 ProgramIndex = 0;
        ProgramIndex < Storage->Count;
        ++ProgramIndex)
    {
        if(Storage->Programs[ProgramIndex].ParentIndex == INVALID_PROGRAM_INDEX)
        {
            Result = ProgramIndex;
            break;
        }
    }
    return(Result);
}

internal void
FindTotalWeight(program_storage *Storage, s32 ParentIndex)
{
    program *Parent = Storage->Programs + ParentIndex;
    Assert(Parent->TotalWeight == INVALID_WEIGHT);
    Parent->TotalWeight = Parent->Weight;
    for(u32 Index = 0;
        Index < Parent->ChildCount;
        ++Index)
    {
        s32 ChildIndex = Parent->Children[Index];
        program *Child = Storage->Programs + ChildIndex;
        FindTotalWeight(Storage, ChildIndex);
        Assert(Child->TotalWeight != INVALID_WEIGHT);
        Assert(Parent->TotalWeight < INT_MAX - Child->TotalWeight);
        Parent->TotalWeight += Child->TotalWeight;
    }
}

struct unbalanced_program
{
    s32 Index;
    s32 BalancedWeight;
};

internal unbalanced_program
FindUnbalancedProgram(program_storage *Storage, s32 ParentIndex)
{
    unbalanced_program Result;
    Result.Index = INVALID_PROGRAM_INDEX;
    Result.BalancedWeight = INVALID_WEIGHT;
    program *Parent = Storage->Programs + ParentIndex;
    Assert(Parent->ChildCount > 2);
    for(u32 IndexA = 0;
        IndexA < Parent->ChildCount;
        ++IndexA)
    {
        u32 IndexB = (IndexA + 1) % Parent->ChildCount;
        u32 IndexC = (IndexA + 2) % Parent->ChildCount;
        s32 ChildAIndex = Parent->Children[IndexA];
        s32 ChildBIndex = Parent->Children[IndexB];
        s32 ChildCIndex = Parent->Children[IndexC];
        program *ChildA = Storage->Programs + ChildAIndex;
        program *ChildB = Storage->Programs + ChildBIndex;
        program *ChildC = Storage->Programs + ChildCIndex;
        if((ChildA->TotalWeight != ChildB->TotalWeight) &&
           (ChildA->TotalWeight != ChildC->TotalWeight))
        {
            Assert(ChildB->TotalWeight == ChildC->TotalWeight);
            unbalanced_program Unbalanced = FindUnbalancedProgram(Storage, ChildAIndex);
            if(Unbalanced.Index == INVALID_PROGRAM_INDEX)
            {
                s32 Delta = ChildB->TotalWeight - ChildA->TotalWeight;
                s32 BalancedWeight = ChildA->Weight + Delta;
                Assert(BalancedWeight > 0);
                Assert((ChildA->TotalWeight - ChildA->Weight + BalancedWeight) == ChildB->TotalWeight);
                Result.Index = ChildAIndex;
                Result.BalancedWeight = BalancedWeight;
            }
            else
            {
                Result = Unbalanced;
            }
        }
    }
    return(Result);
}

int
main(void)
{
    local_persist char PuzzleInput[32768];
    local_persist program_storage Storage;

    {
        ReadEntireFileAndNullTerminate("test_input.txt", PuzzleInput, ArrayCount(PuzzleInput));
        ParseInput(PuzzleInput, &Storage);
        s32 ProgramIndex = FindBottomProgramIndex(&Storage);
        Assert(StringsAreEqual(Storage.Programs[ProgramIndex].Name, "tknk"));

        FindTotalWeight(&Storage, ProgramIndex);
        unbalanced_program UnbalancedResult = FindUnbalancedProgram(&Storage, ProgramIndex);
        Assert(UnbalancedResult.Index != INVALID_PROGRAM_INDEX);
        program *Unbalanced = Storage.Programs + UnbalancedResult.Index;
        Assert(StringsAreEqual(Unbalanced->Name, "ugml"));
        Assert(UnbalancedResult.BalancedWeight == 60);
    }

    {
        ReadEntireFileAndNullTerminate("input.txt", PuzzleInput, ArrayCount(PuzzleInput));
        ParseInput(PuzzleInput, &Storage);
        s32 ProgramIndex = FindBottomProgramIndex(&Storage);
        Assert(StringsAreEqual(Storage.Programs[ProgramIndex].Name, "uownj"));

        FindTotalWeight(&Storage, ProgramIndex);
        unbalanced_program UnbalancedResult = FindUnbalancedProgram(&Storage, ProgramIndex);
        Assert(UnbalancedResult.Index != INVALID_PROGRAM_INDEX);
        program *Unbalanced = Storage.Programs + UnbalancedResult.Index;
        Assert(StringsAreEqual(Unbalanced->Name, "mfzpvpj"));
        Assert(UnbalancedResult.BalancedWeight == 596);
    }
}
