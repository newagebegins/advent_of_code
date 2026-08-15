#include <stdint.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#define global_variable static
#define internal static
#define local_persist static

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef int32_t b32;

typedef float r32;
typedef double r64;

typedef size_t memory_index;

#define Real32Max FLT_MAX
#define Real64Max DBL_MAX

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobytes(Value) ((Value)*1024ULL)
#define Megabytes(Value) (Kilobytes(Value)*1024ULL)
#define Gigabytes(Value) (Megabytes(Value)*1024ULL)
#define Terabytes(Value) (Gigabytes(Value)*1024ULL)

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: { InvalidCodePath; } break;

struct memory_arena
{
    u8 *Base;
    memory_index Size;
    memory_index Used;
    s32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    memory_index Used;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Base = (u8 *)Base;
    Arena->Size = Size;
    Arena->Used = 0;
    Arena->TempCount = 0;
}

inline memory_index
GetAlignmentOffset(memory_arena *Arena, memory_index Alignment)
{
    memory_index AlignmentMask = Alignment - 1;
    memory_index Pointer = (memory_index)Arena->Base + Arena->Used;
    memory_index AlignmentError = Pointer & AlignmentMask;
    memory_index AlignmentOffset = 0;
    if(AlignmentError)
    {
        AlignmentOffset = Alignment - AlignmentError;
    }
    return(AlignmentOffset);
}

inline memory_index
GetRemainingSize(memory_arena *Arena, memory_index Alignment = 4)
{
    memory_index Result = Arena->Size - Arena->Used - GetAlignmentOffset(Arena, Alignment);
    return(Result);
}

inline u8 *
GetWatermark(memory_arena *Arena)
{
    u8 *Result = Arena->Base + Arena->Used;
    return(Result);
}

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    Result.Arena = Arena;
    Result.Used = Arena->Used;
    ++Arena->TempCount;
    return Result;
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    Assert(TempMem.Used <= Arena->Used);
    Arena->Used = TempMem.Used;
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, __VA_ARGS__)
#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(Arena, Count*sizeof(type), __VA_ARGS__)
#define PushCopy(Arena, Size, Source, ...) Copy(Size, Source, PushSize_(Arena, Size, __VA_ARGS__))

inline void *
PushSize_(memory_arena *Arena, memory_index Size, memory_index Alignment = 4)
{
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Alignment);
    Size += AlignmentOffset;

    Assert((Arena->Used + Size) <= Arena->Size);

    void *Result = Arena->Base + Arena->Used + AlignmentOffset;
    Arena->Used += Size;

    return(Result);
}

inline void *
Copy(memory_index Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--)
    {
        *Dest++ = *Source++;
    }
    return(DestInit);
}

inline void
SubArena(memory_arena *Sub, memory_arena *Main, memory_index Size, memory_index Alignment = 16)
{
    Sub->Base = (u8 *)PushSize(Main, Size, Alignment);
    Sub->Size = Size;
    Sub->Used = 0;
    Sub->TempCount = 0;
}

#define ZeroStruct(Instance) ZeroSize(&Instance, sizeof(Instance))
#define ZeroArray(Pointer, Count) ZeroSize((Pointer), (Count)*sizeof((Pointer)[0]))
inline void
ZeroSize(void *VoidPtr, memory_index Size)
{
    u8 *Ptr = (u8 *)VoidPtr;
    while(Size--)
    {
        *Ptr++ = 0;
    }
}

//
//
//

struct str
{
    char *Str;
    u32 Length;
};

internal b32
StringsAreEqual(str A, char *B)
{
    char *AEnd = A.Str + A.Length;
    while((A.Str != AEnd) && *B && (*A.Str == *B))
    {
        ++A.Str;
        ++B;
    }
    b32 Result = ((A.Str == AEnd) && (*B == 0));
    return(Result);
}

inline b32
StringsAreEqual(char *A, char *B)
{
    b32 Result = (A == B);
    if(A && B)
    {
        while(*A && *B && (*A == *B))
        {
            ++A;
            ++B;
        }
        Result = ((*A == 0) && (*B == 0));
    }
    return(Result);
}

//
//
//

enum operation_type
{
    OperationType_SwapPositions,
    OperationType_SwapLetters,
    OperationType_RotateLeft,
    OperationType_RotateRight,
    OperationType_RotatePos,
    OperationType_Reverse,
    OperationType_Move,
};

struct operation
{
    operation_type Type;
    u32 X;
    u32 Y;
};

internal void
RotateRight(char *Source, u32 PasswordLength, char *Dest, u32 Count)
{
    for(u32 LetterIndex = 0;
        LetterIndex < PasswordLength;
        ++LetterIndex)
    {
        u32 DestIndex = ((LetterIndex + Count) % PasswordLength);
        Dest[DestIndex] = Source[LetterIndex];
    }
}

internal void
RotateLeft(char *Source, u32 PasswordLength, char *Dest, u32 Count)
{
    for(u32 LetterIndex = 0;
        LetterIndex < PasswordLength;
        ++LetterIndex)
    {
        u32 DestIndex;
        if(LetterIndex < Count)
        {
            Assert((PasswordLength + LetterIndex) >= Count);
            DestIndex = PasswordLength + LetterIndex - Count;
        }
        else
        {
            DestIndex = LetterIndex - Count;
        }
        Assert(DestIndex < PasswordLength);
        Dest[DestIndex] = Source[LetterIndex];
    }
}

inline void
Swap(char **A, char **B)
{
    char *Temp = *A;
    *A = *B;
    *B = Temp;
}

internal void
RotatePos(char *Source, u32 PasswordLength, char *Dest, char X)
{
    u32 XIndex = PasswordLength;
    for(u32 LetterIndex = 0;
        LetterIndex < PasswordLength;
        ++LetterIndex)
    {
        if(Source[LetterIndex] == X)
        {
            XIndex = LetterIndex;
            break;
        }
    }
    Assert(XIndex < PasswordLength);
    u32 RotateCount = 1 + XIndex + ((XIndex >= 4) ? 1 : 0);
    RotateRight(Source, PasswordLength, Dest, RotateCount);
}

internal char *
Scramble(memory_arena *Arena, char *Password, u32 PasswordLength,
         operation *Operations, u32 OperationCount, b32 Reverse = false)
{
    char *Source = (char *)PushCopy(Arena, PasswordLength + 1, Password);
    char *Dest = PushArray(Arena, PasswordLength + 1, char);
    Dest[PasswordLength] = 0;
    char *Dest2 = PushArray(Arena, PasswordLength + 1, char);
    Dest2[PasswordLength] = 0;

    for(u32 OperationOffset = 0;
        OperationOffset < OperationCount;
        ++OperationOffset)
    {
        u32 OperationIndex = (Reverse ? (OperationCount - 1 - OperationOffset) : OperationOffset);
        operation Operation = Operations[OperationIndex];
        switch(Operation.Type)
        {
            case OperationType_SwapPositions:
            {
                Assert(Operation.X < PasswordLength);
                Assert(Operation.Y < PasswordLength);
                char Temp = Source[Operation.X];
                Source[Operation.X] = Source[Operation.Y];
                Source[Operation.Y] = Temp;
            } break;

            case OperationType_SwapLetters:
            {
                char X = (char)Operation.X;
                char Y = (char)Operation.Y;
                for(u32 LetterIndex = 0;
                    LetterIndex < PasswordLength;
                    ++LetterIndex)
                {
                    if(Source[LetterIndex] == X)
                    {
                        Source[LetterIndex] = Y;
                    }
                    else if(Source[LetterIndex] == Y)
                    {
                        Source[LetterIndex] = X;
                    }
                }
            } break;

            case OperationType_RotateLeft:
            {
                if(Reverse)
                {
                    RotateRight(Source, PasswordLength, Dest, Operation.X);
                }
                else
                {
                    RotateLeft(Source, PasswordLength, Dest, Operation.X);
                }
                Swap(&Source, &Dest);
            } break;

            case OperationType_RotateRight:
            {
                if(Reverse)
                {
                    RotateLeft(Source, PasswordLength, Dest, Operation.X);
                }
                else
                {
                    RotateRight(Source, PasswordLength, Dest, Operation.X);
                }
                Swap(&Source, &Dest);
            } break;

            case OperationType_RotatePos:
            {
                char X = (char)Operation.X;
                if(Reverse)
                {
                    for(u32 RotateCount = 0;
                        RotateCount < PasswordLength;
                        ++RotateCount)
                    {
                        RotateLeft(Source, PasswordLength, Dest, RotateCount);
                        RotatePos(Dest, PasswordLength, Dest2, X);
                        if(StringsAreEqual(Dest2, Source))
                        {
                            break;
                        }
                    }
                }
                else
                {
                    RotatePos(Source, PasswordLength, Dest, X);
                }
                Swap(&Source, &Dest);
            } break;

            case OperationType_Reverse:
            {
                Assert(Operation.X < Operation.Y);
                Assert(Operation.Y < PasswordLength);
                u32 LetterCount = Operation.Y - Operation.X + 1;
                char *Left = Source + Operation.X;
                char *Right = Source + Operation.Y;
                while(Left < Right)
                {
                    char Temp = *Left;
                    *Left = *Right;
                    *Right = Temp;
                    ++Left;
                    --Right;
                }
            } break;

            case OperationType_Move:
            {
                Assert(Operation.X != Operation.Y);
                u32 X;
                u32 Y;
                if(Reverse)
                {
                    X = Operation.Y;
                    Y = Operation.X;
                }
                else
                {
                    X = Operation.X;
                    Y = Operation.Y;
                }
                char *DestAt = Dest;
                if(X < Y)
                {
                    for(u32 SourceIndex = 0;
                        SourceIndex < X;
                        ++SourceIndex)
                    {
                        *DestAt++ = Source[SourceIndex];
                    }
                    for(u32 SourceIndex = X + 1;
                        SourceIndex <= Y;
                        ++SourceIndex)
                    {
                        *DestAt++ = Source[SourceIndex];
                    }
                    *DestAt++ = Source[X];
                    for(u32 SourceIndex = Y + 1;
                        SourceIndex < PasswordLength;
                        ++SourceIndex)
                    {
                        *DestAt++ = Source[SourceIndex];
                    }
                }
                else
                {
                    for(u32 SourceIndex = 0;
                        SourceIndex < Y;
                        ++SourceIndex)
                    {
                        *DestAt++ = Source[SourceIndex];
                    }
                    *DestAt++ = Source[X];
                    for(u32 SourceIndex = Y;
                        SourceIndex < X;
                        ++SourceIndex)
                    {
                        *DestAt++ = Source[SourceIndex];
                    }
                    for(u32 SourceIndex = X + 1;
                        SourceIndex < PasswordLength;
                        ++SourceIndex)
                    {
                        *DestAt++ = Source[SourceIndex];
                    }
                }
                Assert(DestAt == (Dest + PasswordLength));
                Swap(&Source, &Dest);
            } break;

            InvalidDefaultCase;
        }
    }

    return(Source);
}

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == '\n') || (C == ' ') || (C == '\t'));
    return(Result);
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

inline void
SkipWhitespace(char **AtPtr)
{
    char *At = *AtPtr;
    while(IsWhitespace(*At))
    {
        ++At;
    }
    *AtPtr = At;
}

internal u32
FindNonEmptyLineCount(char *Input)
{
    u32 Result = 0;
    char *At = Input;
    while(*At)
    {
        SkipWhitespace(&At);
        if(*At)
        {
            ++Result;
        }
        while(*At && (*At != '\n'))
        {
            ++At;
        }
    }
    return(Result);
}

internal u32
ParseU32(char **AtPtr)
{
    SkipWhitespace(AtPtr);
    u32 Result = 0;
    char *At = *AtPtr;
    Assert(IsDigit(*At));
    while(IsDigit(*At))
    {
        Result = 10*Result + (*At - '0');
        ++At;
    }
    *AtPtr = At;
    return(Result);
}

internal char
ParseChar(char **At)
{
    SkipWhitespace(At);
    Assert(**At);
    char Result = **At;
    ++*At;
    return(Result);
}

internal str
GetWord(char **At)
{
    str Result = {};
    SkipWhitespace(At);
    Assert(**At);
    Result.Str = *At;
    while(**At && !IsWhitespace(**At))
    {
        ++*At;
        ++Result.Length;
    }
    return(Result);
}

internal void
SkipWord(char **At)
{
    SkipWhitespace(At);
    while(**At && !IsWhitespace(**At))
    {
        ++*At;
    }
}

inline u32
StringLength(char *Str)
{
    char *At = Str;
    while(*At)
    {
        ++At;
    }
    u32 Count = (u32)(At - Str);
    return(Count);
}

struct operation_list
{
    operation *Operations;
    u32 Count;
};

internal operation_list
ParseOperations(memory_arena *Arena, char *Input)
{
    operation_list Result;
    Result.Count = FindNonEmptyLineCount(Input);
    Result.Operations = PushArray(Arena, Result.Count, operation);

    char *At = Input;
    SkipWhitespace(&At);

    u32 OperationIndex = 0;

    while(*At)
    {
        operation *Operation = Result.Operations + OperationIndex++;
        str Command = GetWord(&At);
        if(StringsAreEqual(Command, "swap"))
        {
            str SwapType = GetWord(&At);
            if(StringsAreEqual(SwapType, "position"))
            {
                Operation->Type = OperationType_SwapPositions;
                Operation->X = ParseU32(&At);
                SkipWord(&At);
                SkipWord(&At);
                Operation->Y = ParseU32(&At);
            }
            else if(StringsAreEqual(SwapType, "letter"))
            {
                Operation->Type = OperationType_SwapLetters;
                Operation->X = ParseChar(&At);
                SkipWord(&At);
                SkipWord(&At);
                Operation->Y = ParseChar(&At);
            }
            else
            {
                InvalidCodePath;
            }
        }
        else if(StringsAreEqual(Command, "reverse"))
        {
            Operation->Type = OperationType_Reverse;
            SkipWord(&At);
            Operation->X = ParseU32(&At);
            SkipWord(&At);
            Operation->Y = ParseU32(&At);
        }
        else if(StringsAreEqual(Command, "rotate"))
        {
            str RotateDir = GetWord(&At);
            if(StringsAreEqual(RotateDir, "left"))
            {
                Operation->Type = OperationType_RotateLeft;
                Operation->X = ParseU32(&At);
                SkipWord(&At);
            }
            else if(StringsAreEqual(RotateDir, "right"))
            {
                Operation->Type = OperationType_RotateRight;
                Operation->X = ParseU32(&At);
                SkipWord(&At);
            }
            else if(StringsAreEqual(RotateDir, "based"))
            {
                Operation->Type = OperationType_RotatePos;
                SkipWord(&At);
                SkipWord(&At);
                SkipWord(&At);
                SkipWord(&At);
                Operation->X = ParseChar(&At);
            }
            else
            {
                InvalidCodePath;
            }
        }
        else if(StringsAreEqual(Command, "move"))
        {
            Operation->Type = OperationType_Move;
            SkipWord(&At);
            Operation->X = ParseU32(&At);
            SkipWord(&At);
            SkipWord(&At);
            Operation->Y = ParseU32(&At);
        }
        else
        {
            InvalidCodePath;
        }
        SkipWhitespace(&At);
    }

    Assert(OperationIndex == Result.Count);

    return(Result);
}

internal char *
ReadEntireFile(memory_arena *Arena, char *FileName)
{
    char *Result = 0;
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    u32 ContentsSize = ftell(File);
    Assert(ContentsSize > 0);
    fseek(File, 0, SEEK_SET);
    Result = PushArray(Arena, ContentsSize + 1, char);
    size_t ReadCount = fread(Result, ContentsSize, 1, File);
    Assert(ReadCount == 1);
    fclose(File);
    Result[ContentsSize] = 0;
    return(Result);
}

int main(void)
{
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(64);
    void *ArenaBase = malloc(ArenaSize);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

    {
        char *TestOperationsText = R"(swap position 4 with position 0
swap letter d with letter b
reverse positions 0 through 4
rotate left 1 step
move position 1 to position 4
move position 3 to position 0
rotate based on position of letter b
rotate based on position of letter d)";
        char *TestPassword = "abcde";
        printf("%s\n", TestPassword);
        operation_list Operations = ParseOperations(&Arena, TestOperationsText);
        char *Result = Scramble(&Arena, TestPassword, StringLength(TestPassword),
                                Operations.Operations, Operations.Count);
        printf("%s\n", Result);
        char *UnscrambledResult = Scramble(&Arena, Result, StringLength(Result),
                                           Operations.Operations, Operations.Count, true);
        printf("%s\n", UnscrambledResult);
    }

    printf("---\n");

    char *FileContents = ReadEntireFile(&Arena, "input.txt");
    char *Password = "abcdefgh";
    printf("%s\n", Password);
    operation_list Operations = ParseOperations(&Arena, FileContents);
    char *Result = Scramble(&Arena, Password, StringLength(Password),
                            Operations.Operations, Operations.Count);
    printf("%s\n", Result);
    char *UnscrambledResult = Scramble(&Arena, Result, StringLength(Result),
                                       Operations.Operations, Operations.Count, true);
    printf("%s\n", UnscrambledResult);

    printf("---\n");

    char *Password2 = "fbgdceah";
    printf("%s\n", Password2);
    char *UnscrambledResult2 = Scramble(&Arena, Password2, StringLength(Password2),
                                       Operations.Operations, Operations.Count, true);
    printf("%s\n", UnscrambledResult2);
    char *Result2 = Scramble(&Arena, UnscrambledResult2, StringLength(UnscrambledResult2),
                             Operations.Operations, Operations.Count);
    printf("%s\n", Result2);

    return(0);
}
