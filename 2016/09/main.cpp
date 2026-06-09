#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define Kilobytes(X) (1024*(X))
#define Megabytes(X) (1024*Kilobytes(X))

#define Assert(Cond) if(!(Cond)) { *(int *)0 = 0; }
#define InvalidCodePath Assert(!"Invalid code path")
#define InvalidDefaultCase default: { InvalidCodePath; } break;

#define internal static

typedef int32_t s32;

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint32_t b32;

struct memory_arena
{
    void *Memory;
    u32 Size;
    u32 Used;
    s32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    u32 Used;
};

inline memory_arena
MakeArena(void *Memory, u32 Size)
{
    memory_arena Result = {};
    Result.Memory = Memory;
    Result.Size = Size;
    return(Result);
}

inline void *
PushSize_(memory_arena *Arena, u32 Size)
{
    Assert(Arena->Used + Size <= Arena->Size);
    void *Result = (u8 *)Arena->Memory + Arena->Used;
    Arena->Used += Size;
    return(Result);
}

#define PushSize(Arena, Size) PushSize_(Arena, Size)
#define PushStruct(Arena, Type) (Type *)PushSize_(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type *)PushSize_(Arena, Count*sizeof(Type))

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    Result.Arena = Arena;
    Result.Used = Arena->Used;
    ++Arena->TempCount;
    return(Result);
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    Arena->Used = TempMem.Used;
    --Arena->TempCount;
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

inline void
ZeroSize(void *Memory, u32 Size)
{
    u8 *M = (u8 *)Memory;
    for(u32 Index = 0;
        Index < Size;
        ++Index)
    {
        M[Index] = 0;
    }
}

struct entire_file
{
    void *Contents;
    u32 ContentsSize;
};

internal entire_file
ReadEntireFile(char *FileName)
{
    entire_file Result = {};
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    Result.ContentsSize = ftell(File);
    Assert(Result.ContentsSize > 0);
    fseek(File, 0, SEEK_SET);
    Result.Contents = malloc(Result.ContentsSize);
    Assert(Result.Contents);
    size_t ReadCount = fread(Result.Contents, Result.ContentsSize, 1, File);
    Assert(ReadCount == 1);
    fclose(File);
    return(Result);
}

struct parser
{
    char *Text;
    u32 Size;
    u32 At;
};

struct str
{
    char *String;
    u32 Length;
};

inline b32
NotDone(parser *Parser)
{
    b32 Result = Parser->At < Parser->Size;
    return(Result);
}

inline b32
IsWhitespace(char C)
{
    b32 Result = (C == ' ' || C == '\n' || C == '\t');
    return(Result);
}

internal void
SkipWhitespace(parser *Parser)
{
    while(Parser->At < Parser->Size)
    {
        char C = Parser->Text[Parser->At];
        if(IsWhitespace(C))
        {
            ++Parser->At;
        }
        else
        {
            break;
        }
    }
}

internal str
GetWord(parser *Parser)
{
    SkipWhitespace(Parser);
    Assert(Parser->At < Parser->Size);
    u32 StartAt = Parser->At;
    str Result = { Parser->Text + Parser->At };
    while((Parser->At < Parser->Size) && !IsWhitespace(Parser->Text[Parser->At]))
    {
        ++Parser->At;
    }
    Result.Length = Parser->At - StartAt;
    return(Result);
}

internal char
GetChar(parser *Parser)
{
    SkipWhitespace(Parser);
    Assert(Parser->At < Parser->Size);
    char Result = Parser->Text[Parser->At++];
    return(Result);
}

internal u32
GetU32(parser *Parser)
{
    SkipWhitespace(Parser);
    Assert(Parser->At < Parser->Size);
    u32 Result = 0;
    while(Parser->At < Parser->Size)
    {
        char C = Parser->Text[Parser->At];
        if(C >= '0' && C <= '9')
        {
            Result = Result*10 + (C - '0');
            ++Parser->At;
        }
        else
        {
            break;
        }
    }
    return(Result);
}

internal void
SkipChar(parser *Parser, char C)
{
    SkipWhitespace(Parser);
    Assert(Parser->At < Parser->Size);
    Assert(Parser->Text[Parser->At] == C);
    ++Parser->At;
}

internal void
SkipString(parser *Parser, char *String)
{
    SkipWhitespace(Parser);
    for(;;)
    {
        char C = *String;
        if(C == 0)
        {
            break;
        }
        Assert(Parser->At < Parser->Size);
        Assert(Parser->Text[Parser->At] == C);
        ++Parser->At;
        ++String;
    }
}

internal b32
Equal(str A, str B)
{
    b32 Result;
    if(A.Length == B.Length)
    {
        Result = true;
        for(u32 At = 0;
            At < A.Length;
            ++At)
        {
            if(A.String[At] != B.String[At])
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
Length(char *String)
{
    char *Start = String;
    while(*String)
    {
        ++String;
    }
    u32 Result = (u32)(String - Start);
    return(Result);
}

inline str
Str(char *S)
{
    str Result = {S, Length(S)};
    return(Result);
}

internal b32
StringsAreEqual(char *A, char *B)
{
    b32 Result = false;
    for(;;)
    {
        if(*A == *B)
        {
            if(*A == 0)
            {
                Result = true;
                break;
            }
            else
            {
                ++A;
                ++B;
            }
        }
        else
        {
            break;
        }
    }
    return(Result);
}

inline void
PushStr(memory_arena *Arena, char *Source, u32 Length)
{
    char *Dest = PushArray(Arena, Length, char);
    for(u32 Index = 0;
        Index < Length;
        ++Index)
    {
        Dest[Index] = Source[Index];
    }
}

inline void
PushChar(memory_arena *Arena, char C)
{
    char *Dest = (char *)PushSize(Arena, sizeof(char));
    *Dest = C;
}

internal str
Decompress(memory_arena *Arena, str Input)
{
    str Result = {};
    Result.String = (char *)Arena->Memory + Arena->Used;

    parser Parser_ = {};
    parser *Parser = &Parser_;
    Parser->Text = Input.String;
    Parser->Size = Input.Length;
    Parser->At = 0;

    u32 OldUsed = Arena->Used;

    while(NotDone(Parser))
    {
        char C = GetChar(Parser);
        if(C == '(')
        {
            u32 SequenceLength = GetU32(Parser);
            SkipChar(Parser, 'x');
            u32 RepeatCount = GetU32(Parser);
            SkipChar(Parser, ')');
            char *Sequence = Parser->Text + Parser->At;
            for(u32 Iteration = 0;
                Iteration < RepeatCount;
                ++Iteration)
            {
                PushStr(Arena, Sequence, SequenceLength);
            }
            Parser->At += SequenceLength;
        }
        else
        {
            PushChar(Arena, C);
        }
        SkipWhitespace(Parser);
    }

    Result.Length = Arena->Used - OldUsed;

    return(Result);
}

internal u64
DecompressV2(str Input)
{
    u64 Result = 0;

    parser Parser_ = {};
    parser *Parser = &Parser_;
    Parser->Text = Input.String;
    Parser->Size = Input.Length;
    Parser->At = 0;

    while(NotDone(Parser))
    {
        char C = GetChar(Parser);
        if(C == '(')
        {
            u32 SequenceLength = GetU32(Parser);
            SkipChar(Parser, 'x');
            u32 RepeatCount = GetU32(Parser);
            SkipChar(Parser, ')');
            str Sequence = {Parser->Text + Parser->At, SequenceLength};
            Result += DecompressV2(Sequence)*RepeatCount;
            Parser->At += SequenceLength;
        }
        else
        {
            ++Result;
        }
        SkipWhitespace(Parser);
    }

    return(Result);
}

inline void
Test(memory_arena *Arena, char *Input, char *Expected)
{
    temporary_memory TempMem = BeginTemporaryMemory(Arena);
    str Decompressed = Decompress(Arena, Str(Input));
    Assert(Equal(Decompressed, Str(Expected)));
    EndTemporaryMemory(TempMem);
}

inline void
TestV2(char *Input, u64 ExpectedLength)
{
    u64 Length = DecompressV2(Str(Input));
    Assert(Length == ExpectedLength);
}

int main(void)
{
    u32 MemorySize = Megabytes(16);
    void *Memory = malloc(MemorySize);
    Assert(Memory);

    memory_arena MainArena = MakeArena(Memory, MemorySize);

    Test(&MainArena, "ADVENT", "ADVENT");
    Test(&MainArena, "A(1x5)BC", "ABBBBBC");
    Test(&MainArena, "(3x3)XYZ", "XYZXYZXYZ");
    Test(&MainArena, "A(2x2)BCD(2x2)EFG", "ABCBCDEFEFG");
    Test(&MainArena, "(6x1)(1x3)A", "(1x3)A");
    Test(&MainArena, "X(8x2)(3x3)ABCY", "X(3x3)ABC(3x3)ABCY");

    TestV2("(3x3)XYZ", 9);
    TestV2("X(8x2)(3x3)ABCY", 20);
    TestV2("(27x12)(20x12)(13x14)(7x10)(1x12)A", 241920);
    TestV2("(25x3)(3x3)ABC(2x3)XY(5x2)PQRSTX(18x9)(3x2)TWO(5x7)SEVEN", 445);

    entire_file File = ReadEntireFile("input.txt");

    str Input = {(char *)File.Contents, File.ContentsSize};
    str Decompressed = Decompress(&MainArena, Input);
    printf("%u\n", Decompressed.Length);
    printf("%I64u\n", DecompressV2(Input));

    CheckArena(&MainArena);

    return(0);
}
