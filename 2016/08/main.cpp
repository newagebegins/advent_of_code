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

struct screen
{
    u32 DimX;
    u32 DimY;
    u8 *Pixels;
};

enum operation_type
{
    Operation_Rect,
    Operation_RotateRow,
    Operation_RotateColumn,
};

struct operation
{
    operation_type Type;
    union
    {
        struct
        {
            u32 DimX, DimY;
        };
        struct
        {
            u32 RowY, Amount;
        };
        struct
        {
            u32 ColX, Amount;
        };
    };
};

internal screen *
AllocateScreen(memory_arena *Arena, u32 DimX, u32 DimY)
{
    screen *Result = PushStruct(Arena, screen);
    Result->DimX = DimX;
    Result->DimY = DimY;
    u32 Size = DimX*DimY*sizeof(u8);
    Result->Pixels = (u8 *)PushSize(Arena, Size);
    ZeroSize(Result->Pixels, Size);
    return(Result);
}


internal u32
GetLitPixelCount(screen *Screen)
{
    u32 Result = 0;
    u8 *Row = Screen->Pixels;
    for(u32 Y = 0;
        Y < Screen->DimY;
        ++Y)
    {
        u8 *Pixels = Row;
        for(u32 X = 0;
            X < Screen->DimX;
            ++X)
        {
            u8 Pixel = *Pixels++;
            if(Pixel)
            {
                ++Result;
            }
        }
        Row += Screen->DimX;
    }
    return(Result);
}


internal void
Print(screen *Screen)
{
    u8 *Row = Screen->Pixels;
    for(u32 Y = 0;
        Y < Screen->DimY;
        ++Y)
    {
        u8 *Pixels = Row;
        for(u32 X = 0;
            X < Screen->DimX;
            ++X)
        {
            u8 Pixel = *Pixels++;
            char C = Pixel ? '#' : '.';
            printf("%c", C);
        }
        printf("\n");
        Row += Screen->DimX;
    }
    printf("\n");
}

internal void
Rect(screen *Screen, u32 DimX, u32 DimY)
{
    u8 *Row = Screen->Pixels;
    for(u32 Y = 0;
        Y < DimY;
        ++Y)
    {
        u8 *Pixels = Row;
        for(u32 X = 0;
            X < DimX;
            ++X)
        {
            *Pixels++ = 1;
        }
        Row += Screen->DimX;
    }
}

internal void
RotateRow(memory_arena *Arena, screen *Screen, u32 RowY, u32 Amount)
{
    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    u8 *Row = Screen->Pixels + RowY*Screen->DimX;
    u8 *NewRow = PushArray(Arena, Screen->DimX, u8);

    for(u32 X = 0;
        X < Screen->DimX;
        ++X)
    {
        u32 NewX = (X + Amount) % Screen->DimX;
        NewRow[NewX] = Row[X];
    }

    for(u32 X = 0;
        X < Screen->DimX;
        ++X)
    {
        Row[X] = NewRow[X];
    }

    EndTemporaryMemory(TempMem);
}

internal void
RotateColumn(memory_arena *Arena, screen *Screen, u32 ColX, u32 Amount)
{
    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    u8 *Col = Screen->Pixels + ColX;
    u8 *NewCol = PushArray(Arena, Screen->DimY, u8);

    u8 *ColAt = Col;
    for(u32 Y = 0;
        Y < Screen->DimY;
        ++Y)
    {
        u32 NewY = (Y + Amount) % Screen->DimY;
        NewCol[NewY] = *ColAt;
        ColAt += Screen->DimX;
    }

    ColAt = Col;
    for(u32 Y = 0;
        Y < Screen->DimY;
        ++Y)
    {
        *ColAt = NewCol[Y];
        ColAt += Screen->DimX;
    }

    EndTemporaryMemory(TempMem);
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

int main(void)
{
    u32 MemorySize = Megabytes(16);
    void *Memory = malloc(MemorySize);
    Assert(Memory);

    memory_arena MainArena = MakeArena(Memory, MemorySize);

#if 0
    screen *Screen = AllocateScreen(&MainArena, 7, 3);
    Print(Screen);
    Rect(Screen, 3, 2);
    Print(Screen);
    RotateColumn(&MainArena, Screen, 1, 1);
    Print(Screen);
    RotateRow(&MainArena, Screen, 0, 4);
    Print(Screen);
    RotateColumn(&MainArena, Screen, 1, 1);
    Print(Screen);
#endif

    entire_file File = ReadEntireFile("input.txt");

    parser Parser_ = {};
    parser *Parser = &Parser_;
    Parser->Text = (char *)File.Contents;
    Parser->Size = File.ContentsSize;
    Parser->At = 0;

    operation *Operations = (operation *)((u8 *)MainArena.Memory + MainArena.Used);
    u32 OperationCount = 0;

    str RectStr = Str("rect");
    str RotateStr = Str("rotate");
    str RowStr = Str("row");
    str ColumnStr = Str("column");

    while(NotDone(Parser))
    {
        operation *Operation = PushStruct(&MainArena, operation);
        ++OperationCount;

        str OpType = GetWord(Parser);
        if(Equal(OpType, RectStr))
        {
            Operation->Type = Operation_Rect;
            Operation->DimX = GetU32(Parser);
            SkipChar(Parser, 'x');
            Operation->DimY = GetU32(Parser);
        }
        else if(Equal(OpType, RotateStr))
        {
            str RowOrColumn = GetWord(Parser);
            if(Equal(RowOrColumn, RowStr))
            {
                Operation->Type = Operation_RotateRow;
                SkipString(Parser, "y=");
                Operation->RowY = GetU32(Parser);
                SkipString(Parser, "by");
                Operation->Amount = GetU32(Parser);
            }
            else if(Equal(RowOrColumn, ColumnStr))
            {
                Operation->Type = Operation_RotateColumn;
                SkipString(Parser, "x=");
                Operation->ColX = GetU32(Parser);
                SkipString(Parser, "by");
                Operation->Amount = GetU32(Parser);
            }
            else
            {
                Assert(!"Unknown word");
            }
        }
        else
        {
            Assert(!"Unknown operation");
        }

        SkipWhitespace(Parser);
    }

    screen *Screen = AllocateScreen(&MainArena, 50, 6);

    for(u32 OperationIndex = 0;
        OperationIndex < OperationCount;
        ++OperationIndex)
    {
        operation *Operation = Operations + OperationIndex;

        switch(Operation->Type)
        {
            case Operation_Rect:
            {
                Rect(Screen, Operation->DimX, Operation->DimY);
            } break;

            case Operation_RotateRow:
            {
                RotateRow(&MainArena, Screen, Operation->RowY, Operation->Amount);
            } break;

            case Operation_RotateColumn:
            {
                RotateColumn(&MainArena, Screen, Operation->ColX, Operation->Amount);
            } break;

            InvalidDefaultCase;
        }
    }

    Print(Screen);

    u32 LitPixelCount = GetLitPixelCount(Screen);
    printf("%u\n", LitPixelCount);

    CheckArena(&MainArena);

    return(0);
}
