#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t b32;

#define internal static
#define local_persist static

#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))
#define Assert(C) if(!(C)) { *(int *)0 = 0; }
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); } break

#define MAX_PROGRAM_COUNT 16

struct programs
{
    char Buffers[2][MAX_PROGRAM_COUNT + 1];
    u32 Count;
    u32 BufferIndex;
};

inline char *
GetBuffer(programs *Programs)
{
    char *Buffer = Programs->Buffers[Programs->BufferIndex];
    return(Buffer);
}

internal void
InitializePrograms(programs *Programs, u32 Count)
{
    Assert(Count <= MAX_PROGRAM_COUNT);
    Programs->Count = Count;
    Programs->BufferIndex = 0;
    char *Buffer = GetBuffer(Programs);
    for(u8 ProgramIndex = 0;
        ProgramIndex < Programs->Count;
        ++ProgramIndex)
    {
        Buffer[ProgramIndex] = 'a' + ProgramIndex;
    }
    Buffer[Programs->Count] = 0;
}

internal b32
InOrder(programs *Programs, char *ExpectedOrder)
{
    b32 Result = true;
    char *Buffer = GetBuffer(Programs);
    for(u8 ProgramIndex = 0;
        ProgramIndex < Programs->Count;
        ++ProgramIndex)
    {
        if(Buffer[ProgramIndex] != ExpectedOrder[ProgramIndex])
        {
            Result = false;
            break;
        }
    }
    return(Result);
}

internal b32
InOrder(char *Buffer, char *ExpectedOrder)
{
    b32 Result = true;
    for(u8 ProgramIndex = 0;
        ProgramIndex < 16;
        ++ProgramIndex)
    {
        if(Buffer[ProgramIndex] != ExpectedOrder[ProgramIndex])
        {
            Result = false;
            break;
        }
    }
    return(Result);
}

internal void
Spin(programs *Programs, u32 SpinSize)
{
    Assert(Programs->BufferIndex <= ArrayCount(Programs->Buffers));
    Assert(SpinSize <= Programs->Count);
    u32 SourceBufferIndex = Programs->BufferIndex;
    u32 DestBufferIndex = !SourceBufferIndex;
    Programs->BufferIndex = DestBufferIndex;

    char *Source = Programs->Buffers[SourceBufferIndex];
    char *Dest = Programs->Buffers[DestBufferIndex];

    char *SourceAt = Source + Programs->Count - SpinSize;
    char *DestAt = Dest;

    for(u32 Index = 0;
        Index < SpinSize;
        ++Index)
    {
        *DestAt++ = *SourceAt++;
    }

    SourceAt = Source;

    for(u32 Index = 0;
        Index < (Programs->Count - SpinSize);
        ++Index)
    {
        *DestAt++ = *SourceAt++;
    }
}

internal void
Exchange(programs *Programs, u32 AIndex, u32 BIndex)
{
    char *Buffer = GetBuffer(Programs);
    char Temp = Buffer[AIndex];
    Buffer[AIndex] = Buffer[BIndex];
    Buffer[BIndex] = Temp;
}

internal u32
FindIndex(programs *Programs, char Program)
{
    u32 Result = 0;
    char *Buffer = GetBuffer(Programs);
    for(u32 Index = 0;
        Index < Programs->Count;
        ++Index)
    {
        if(Buffer[Index] == Program)
        {
            Result = Index;
            break;
        }
    }
    return(Result);
}

internal void
Partner(programs *Programs, char A, char B)
{
    u32 AIndex = FindIndex(Programs, A);
    u32 BIndex = FindIndex(Programs, B);
    Exchange(Programs, AIndex, BIndex);
}

internal void
ReadEntireFileAndNullTerminate(char *FileName, char *Buffer, u32 BufferSize)
{
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    Assert(FileSize > 0);
    Assert(((u32)FileSize + 1) <= BufferSize);
    fseek(File, 0, SEEK_SET);
    fread(Buffer, FileSize, 1, File);
    Buffer[FileSize] = 0;
    fclose(File);
}

enum token_type
{
    Token_Unknown,
    Token_Spin,
    Token_Exchange,
    Token_Partner,
    Token_Comma,
    Token_EndOfFile,
};

struct token
{
    token_type Type;
    u8 Args[2];
};

struct tokenizer
{
    char *At;
};

inline void
InitializeTokenizer(tokenizer *Tokenizer, char *Input)
{
    Tokenizer->At = Input;
}

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') || (C == '\t') || (C == '\r') || (C == '\n'));
    return(Result);
}

inline void
SkipWhitespace(tokenizer *Tokenizer)
{
    while(IsWhitespace(Tokenizer->At[0]))
    {
        ++Tokenizer->At;
    }
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

internal u8
GetU8(tokenizer *Tokenizer)
{
    u8 Result = 0;
    while(IsDigit(Tokenizer->At[0]))
    {
        Result = 10*Result + (Tokenizer->At[0] - '0');
        ++Tokenizer->At;
    }
    return(Result);
}

internal token
GetToken(tokenizer *Tokenizer)
{
    token Result = {};
    SkipWhitespace(Tokenizer);
    switch(Tokenizer->At[0])
    {
        case 0:
        {
            Result.Type = Token_EndOfFile;
        } break;

        case ',':
        {
            Result.Type = Token_Comma;
            ++Tokenizer->At;
        } break;

        case 's':
        {
            Result.Type = Token_Spin;
            ++Tokenizer->At;
            Result.Args[0] = GetU8(Tokenizer);
        } break;

        case 'x':
        {
            Result.Type = Token_Exchange;
            ++Tokenizer->At;
            Result.Args[0] = GetU8(Tokenizer);
            Assert(Tokenizer->At[0] == '/');
            ++Tokenizer->At;
            Result.Args[1] = GetU8(Tokenizer);
        } break;

        case 'p':
        {
            Result.Type = Token_Partner;
            ++Tokenizer->At;
            Result.Args[0] = Tokenizer->At[0];
            ++Tokenizer->At;
            Assert(Tokenizer->At[0] == '/');
            ++Tokenizer->At;
            Result.Args[1] = Tokenizer->At[0];
            ++Tokenizer->At;
        } break;

        InvalidDefaultCase;
    }
    return(Result);
}

internal void
Dance(programs *Programs, char *Input)
{
    tokenizer Tokenizer;
    InitializeTokenizer(&Tokenizer, Input);
    b32 Finished = false;
    while(!Finished)
    {
        token Token = GetToken(&Tokenizer);
        switch(Token.Type)
        {
            case Token_Spin:
            {
                Spin(Programs, Token.Args[0]);
            } break;

            case Token_Exchange:
            {
                Exchange(Programs, Token.Args[0], Token.Args[1]);
            } break;

            case Token_Partner:
            {
                Partner(Programs, Token.Args[0], Token.Args[1]);
            } break;

            case Token_Comma:
            {
                // NOTE(slava): Do nothing
            } break;

            case Token_EndOfFile:
            {
                Finished = true;
            } break;

            InvalidDefaultCase;
        }
    }
}

struct compressed_dance
{
    u8 Shuffle[16];
    char Rename[16];
};

internal compressed_dance
CompressDance(char *Input)
{
    compressed_dance Result;

    u8 *Shuffle = Result.Shuffle;
    char *Rename = Result.Rename;

    for(u8 Index = 0;
        Index < 16;
        ++Index)
    {
        Shuffle[Index] = Index;
    }

    for(u8 Index = 0;
        Index < 16;
        ++Index)
    {
        Rename[Index] = 'a' + Index;
    }

    tokenizer Tokenizer;
    InitializeTokenizer(&Tokenizer, Input);

    b32 Finished = false;

    while(!Finished)
    {
        token Token = GetToken(&Tokenizer);

        switch(Token.Type)
        {
            case Token_Spin:
            {
                u8 Temp[16];
                for(u32 Index = 0;
                    Index < 16;
                    ++Index)
                {
                    Temp[Index] = Shuffle[Index];
                }
                u8 SpinSize = Token.Args[0];
                for(u32 Index = 0;
                    Index < SpinSize;
                    ++Index)
                {
                    Shuffle[Index] = Temp[16 - SpinSize + Index];
                }
                for(u32 Index = SpinSize;
                    Index < 16;
                    ++Index)
                {
                    Shuffle[Index] = Temp[Index - SpinSize];
                }
            } break;

            case Token_Exchange:
            {
                u8 Temp = Shuffle[Token.Args[0]];
                Shuffle[Token.Args[0]] = Shuffle[Token.Args[1]];
                Shuffle[Token.Args[1]] = Temp;
            } break;

            case Token_Partner:
            {
                for(u32 Index = 0;
                    Index < 16;
                    ++Index)
                {
                    if(Rename[Index] == Token.Args[0])
                    {
                        Rename[Index] = Token.Args[1];
                    }
                    else if(Rename[Index] == Token.Args[1])
                    {
                        Rename[Index] = Token.Args[0];
                    }
                }
            } break;

            case Token_Comma:
            {
                // NOTE(slava): Do nothing
            } break;

            case Token_EndOfFile:
            {
                Finished = true;
            } break;

            InvalidDefaultCase;
        }
    }

    return(Result);
}

internal void
PerformDance(compressed_dance *CompressedDance, char *Programs)
{
    char Copy[16];

    for(u32 Index = 0;
        Index < 16;
        ++Index)
    {
        Copy[Index] = Programs[Index];
    }

    for(u32 Index = 0;
        Index < 16;
        ++Index)
    {
        Programs[Index] = Copy[CompressedDance->Shuffle[Index]];
    }

    for(u32 Index = 0;
        Index < 16;
        ++Index)
    {
        Programs[Index] = CompressedDance->Rename[Programs[Index] - 'a'];
    }
}

int
main(void)
{
    {
        programs Programs;
        InitializePrograms(&Programs, 5);
        Assert(InOrder(&Programs, "abcde"));
        Spin(&Programs, 1);
        Assert(InOrder(&Programs, "eabcd"));
        Exchange(&Programs, 3, 4);
        Assert(InOrder(&Programs, "eabdc"));
        Partner(&Programs, 'e', 'b');
        Assert(InOrder(&Programs, "baedc"));
    }

    {
        local_persist char Input[64 * 1024];
        ReadEntireFileAndNullTerminate("input.txt", Input, sizeof(Input));

        programs Programs;
        InitializePrograms(&Programs, 16);

        Dance(&Programs, Input);
        Assert(InOrder(&Programs, "eojfmbpkldghncia"));

        // NOTE(slava): Part 2

        char Progs[] = "abcdefghijklmnop";
        compressed_dance CompressedDance = CompressDance(Input);

        for(u32 DanceIndex = 0;
            DanceIndex < 1000000000;
            ++DanceIndex)
        {
            PerformDance(&CompressedDance, Progs);
        }

        Assert(InOrder(Progs, "iecopnahgdflmkjb"));
    }

    return(0);
}
