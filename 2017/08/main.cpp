#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define internal static

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t s32;
typedef int32_t b32;

#define Assert(C) if(!(C)) {*(int *)0 = 0;}
#define InvalidCodePath Assert(!"Invalid code path")

struct memory_arena
{
    u32 Used;
    u32 Size;
    void *Base;
};

inline void *
PushSize_(memory_arena *Arena, u32 Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = (u8 *)Arena->Base + Arena->Used;
    Arena->Used += Size;
    return(Result);
}

#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))

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

struct register_storage
{
    u32 MaxCount;
    // NOTE(slava): Assume 0 is an invalid register index
    u32 Count;
    str *Names;
    s32 *Values;
};

internal u32
GetRegisterIndexFromName(register_storage *Storage, str Name)
{
    u32 Result = 0;
    for(u32 RegisterIndex = 1;
        RegisterIndex < Storage->Count;
        ++RegisterIndex)
    {
        if(StringsAreEqual(Name, Storage->Names[RegisterIndex]))
        {
            Result = RegisterIndex;
            break;
        }
    }
    if(!Result)
    {
        Result = Storage->Count++;
        Assert(Result < Storage->MaxCount);
        Storage->Names[Result] = Name;
        Storage->Values[Result] = 0;
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
    b32 Result = ((C == ' ') ||
                  (C == '\t') ||
                  (C == '\r') ||
                  (C == '\n'));
    return(Result);
}

internal void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

internal str
GetWord(parser *Parser)
{
    SkipWhitespace(Parser);
    str Result = {};
    Result.Chars = Parser->At;
    while(Parser->At[0] && !IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
    Result.Length = (u32)(Parser->At - Result.Chars);
    return(Result);
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

internal s32
GetS32(parser *Parser)
{
    SkipWhitespace(Parser);
    s32 Result = 0;
    s32 Sign = 1;
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

internal void
SkipString(parser *Parser, char *String)
{
    SkipWhitespace(Parser);
    while(*String)
    {
        Assert(Parser->At[0] == *String);
        ++Parser->At;
        ++String;
    }
}

internal s32
RunInstructionsFromInput(register_storage *Storage, char *Input)
{
    s32 LargestValueSeen = INT_MIN;

    parser Parser;
    Parser.At = Input;

    while(Parser.At[0])
    {
        str RegisterNameToModify = GetWord(&Parser);
        u32 RegisterIndexToModify = GetRegisterIndexFromName(Storage, RegisterNameToModify);
        str OperationName = GetWord(&Parser);
        b32 Decrease = StringsAreEqual(OperationName, "dec");
        s32 Delta = GetS32(&Parser);
        if(Decrease)
        {
            Delta = -Delta;
        }
        SkipString(&Parser, "if");
        str RegisterNameToCompare = GetWord(&Parser);
        u32 RegisterIndexToCompare = GetRegisterIndexFromName(Storage, RegisterNameToCompare);
        str ComparisonOperationName = GetWord(&Parser);
        s32 ComparisonValue = GetS32(&Parser);
        s32 RegisterValueToCompare = Storage->Values[RegisterIndexToCompare];

        b32 ComparisonSuccess = false;
        if(StringsAreEqual(ComparisonOperationName, ">"))
        {
            ComparisonSuccess = (RegisterValueToCompare > ComparisonValue);
        }
        else if(StringsAreEqual(ComparisonOperationName, "<"))
        {
            ComparisonSuccess = (RegisterValueToCompare < ComparisonValue);
        }
        else if(StringsAreEqual(ComparisonOperationName, ">="))
        {
            ComparisonSuccess = (RegisterValueToCompare >= ComparisonValue);
        }
        else if(StringsAreEqual(ComparisonOperationName, "<="))
        {
            ComparisonSuccess = (RegisterValueToCompare <= ComparisonValue);
        }
        else if(StringsAreEqual(ComparisonOperationName, "=="))
        {
            ComparisonSuccess = (RegisterValueToCompare == ComparisonValue);
        }
        else if(StringsAreEqual(ComparisonOperationName, "!="))
        {
            ComparisonSuccess = (RegisterValueToCompare != ComparisonValue);
        }
        else
        {
            InvalidCodePath;
        }

        if(ComparisonSuccess)
        {
            Storage->Values[RegisterIndexToModify] += Delta;
            if(LargestValueSeen < Storage->Values[RegisterIndexToModify])
            {
                LargestValueSeen = Storage->Values[RegisterIndexToModify];
            }
        }

        SkipWhitespace(&Parser);
    }

    return(LargestValueSeen);
}

internal char *
ReadEntireFileAndNullTerminate(memory_arena *Arena, char *FileName)
{
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);
    char *Result = PushArray(Arena, FileSize + 1, char);
    fread(Result, FileSize, 1, File);
    Result[FileSize] = 0;
    fclose(File);
    return(Result);
}

internal s32
GetLargestRegisterValue(register_storage *Storage)
{
    s32 LargestValue = INT_MIN;
    for(u32 RegisterIndex = 1;
        RegisterIndex < Storage->Count;
        ++RegisterIndex)
    {
        if(LargestValue < Storage->Values[RegisterIndex])
        {
            LargestValue = Storage->Values[RegisterIndex];
        }
    }
    return(LargestValue);
}

int
main(void)
{
    memory_arena Arena = {};
    Arena.Size = 1*1024*1024;
    Arena.Base = malloc(Arena.Size);

    register_storage Storage;
    Storage.MaxCount = 128;
    Storage.Count = 1;
    Storage.Names = PushArray(&Arena, Storage.MaxCount, str);
    Storage.Values = PushArray(&Arena, Storage.MaxCount, s32);

    char *Input = ReadEntireFileAndNullTerminate(&Arena, "test_input.txt");
    s32 LargestValueSeen = RunInstructionsFromInput(&Storage, Input);
    s32 LargestValue = GetLargestRegisterValue(&Storage);
    Assert(LargestValue == 1);
    Assert(LargestValueSeen == 10);

    Input = ReadEntireFileAndNullTerminate(&Arena, "input.txt");
    Storage.Count = 1;
    LargestValueSeen = RunInstructionsFromInput(&Storage, Input);
    LargestValue = GetLargestRegisterValue(&Storage);
    Assert(LargestValue == 8022);
    Assert(LargestValueSeen == 9819);
}
