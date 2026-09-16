#include <stdint.h>
#include <stdio.h>

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
    s32 ParentIndex;
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
        Storage->Programs[ProgramIndex].ParentIndex = INVALID_PROGRAM_INDEX;
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
        str Name = GetName(&Parser);
        s32 ProgramIndex = FindOrCreateProgramWithName(Name, Storage);
        SkipString(&Parser, " (");
        u32 Weight = GetU32(&Parser);
        Storage->Programs[ProgramIndex].Weight = Weight;
        SkipChar(&Parser, ')');
        SkipWhitespace(&Parser);
        if(Parser.At[0] == '-')
        {
            SkipString(&Parser, "-> ");
            for(;;)
            {
                str ChildName = GetName(&Parser);
                s32 ChildIndex = FindOrCreateProgramWithName(ChildName, Storage);
                Storage->Programs[ChildIndex].ParentIndex = ProgramIndex;
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
    }

    {
        ReadEntireFileAndNullTerminate("input.txt", PuzzleInput, ArrayCount(PuzzleInput));
        ParseInput(PuzzleInput, &Storage);
        s32 ProgramIndex = FindBottomProgramIndex(&Storage);
        Assert(StringsAreEqual(Storage.Programs[ProgramIndex].Name, "uownj"));
    }
}
