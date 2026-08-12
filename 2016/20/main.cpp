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

inline u32
SafeTruncateU64(u64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return(Result);
}

#define AlignN(Value, N) (((Value) + (N-1)) & ~(N-1))
#define Align4(Value) (((Value) + 3) & ~3)
#define Align16(Value) (((Value) + 15) & ~15)

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

struct random_series
{
    u32 A;
};

inline random_series
RandomSeries(u32 Seed)
{
    random_series Result = {Seed};
    return(Result);
}

inline u32
NextRandomNumber(random_series *Series)
{
    u32 X = Series->A;
    X ^= X << 13;
    X ^= X >> 17;
    X ^= X << 5;
    Series->A = X;
    return(X);
}

inline u32
RandomChoice(random_series *Series, u32 Count)
{
    u32 Result = NextRandomNumber(Series) % Count;
    return(Result);
}

#define RandomPick(Series, Array) (Array[RandomChoice(Series, ArrayCount(Array))])

inline s32
RandomBetween(random_series *Series, s32 Min, s32 OnePastMax)
{
    s32 Result = Min + RandomChoice(Series, OnePastMax - Min);
    return(Result);
}

//
//
//

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

//
//
//

struct ip_range
{
    u32 Begin;
    u32 End;
};

struct ip_range_list
{
    ip_range *Ranges;
    u32 Count;
};

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

inline void
SkipChar(char **AtPtr, char C)
{
    Assert(**AtPtr == C);
    ++*AtPtr;
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

internal ip_range_list
ParseIPRanges(memory_arena *Arena, char *Input)
{
    ip_range_list Result = {};
    Result.Count = FindNonEmptyLineCount(Input);
    Result.Ranges = PushArray(Arena, Result.Count, ip_range);

    u32 RangeIndex = 0;
    char *At = Input;
    SkipWhitespace(&At);

    while(*At)
    {
        Result.Ranges[RangeIndex].Begin = ParseU32(&At);
        SkipChar(&At, '-');
        Result.Ranges[RangeIndex].End = ParseU32(&At);
        ++RangeIndex;
        SkipWhitespace(&At);
    }

    Assert(RangeIndex == Result.Count);

    return(Result);
}

internal u32
FindLowestNonBlockedIP(ip_range_list SortedRanges)
{
    u32 Result = 0;
    for(u32 RangeIndex = 0;
        RangeIndex < SortedRanges.Count;
        ++RangeIndex)
    {
        ip_range *Range = SortedRanges.Ranges + RangeIndex;
        if(Result < Range->Begin)
        {
            break;
        }
        if(Result <= Range->End)
        {
            Assert(Range->End != 0xFFFFFFFF);
            Result = Range->End + 1;
        }
    }
    return(Result);
}

internal u32
FindAllowedIPCount(ip_range_list SortedRanges, u32 MaxIP)
{
    u32 Count = 0;
    u32 IP = 0;
    b32 Overflow = false;
    for(u32 RangeIndex = 0;
        RangeIndex < SortedRanges.Count;
        ++RangeIndex)
    {
        ip_range *Range = SortedRanges.Ranges + RangeIndex;
        if(IP < Range->Begin)
        {
            u32 AllowedRange = Range->Begin - IP;
            Count += AllowedRange;
        }
        if(IP <= Range->End)
        {
            IP = Range->End + 1;
            if(Range->End == 0xFFFFFFFF)
            {
                Overflow = true;
                break;
            }
        }
    }
    if(!Overflow)
    {
        Count += (MaxIP - IP + 1);
    }
    return(Count);
}

struct sort_job
{
    u32 FirstIndex;
    u32 OnePastLastIndex;
};

inline void
Swap(ip_range *Ranges, u32 AIndex, u32 BIndex)
{
    ip_range Temp = Ranges[AIndex];
    Ranges[AIndex] = Ranges[BIndex];
    Ranges[BIndex] = Temp;
}

internal void
SortRanges(ip_range_list Ranges, random_series *Series)
{
    sort_job Jobs[128];
    Jobs[0] = {0, Ranges.Count};
    u32 JobCount = 1;

    while(JobCount)
    {
        sort_job Job = Jobs[--JobCount];
        u32 RandomIndex = RandomBetween(Series, Job.FirstIndex, Job.OnePastLastIndex);
        Swap(Ranges.Ranges, Job.FirstIndex, RandomIndex);
        u32 Pivot = Ranges.Ranges[Job.FirstIndex].Begin;
        u32 I = Job.FirstIndex + 1;
        for(u32 J = Job.FirstIndex + 1;
            J < Job.OnePastLastIndex;
            ++J)
        {
            if(Ranges.Ranges[J].Begin < Pivot)
            {
                Swap(Ranges.Ranges, I++, J);
            }
        }
        u32 PivotIndex = I - 1;
        Swap(Ranges.Ranges, Job.FirstIndex, PivotIndex);
        if(PivotIndex > (Job.FirstIndex + 1))
        {
            Assert(JobCount < ArrayCount(Jobs));
            Jobs[JobCount++] = {Job.FirstIndex, PivotIndex};
        }
        if(Job.OnePastLastIndex > (1 + (PivotIndex + 1)))
        {
            Assert(JobCount < ArrayCount(Jobs));
            Jobs[JobCount++] = {PivotIndex + 1, Job.OnePastLastIndex};
        }
    }
}

int main(void)
{
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(128);
    void *ArenaBase = malloc(ArenaSize);
    Assert(ArenaBase);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

    random_series Series = RandomSeries(123);

    {
        char *TestInput = R"(5-8
0-2
4-7)";
        ip_range_list Ranges = ParseIPRanges(&Arena, TestInput);
        SortRanges(Ranges, &Series);
        u32 IP = FindLowestNonBlockedIP(Ranges);
        Assert(IP == 3);

        u32 Count = FindAllowedIPCount(Ranges, 9);
        Assert(Count == 2);
    }

    char *FileContents = ReadEntireFile(&Arena, "input.txt");
    ip_range_list Ranges = ParseIPRanges(&Arena, FileContents);
    SortRanges(Ranges, &Series);
    u32 IP = FindLowestNonBlockedIP(Ranges);
    Assert(IP == 31053880);

    u32 Count = FindAllowedIPCount(Ranges, 0xFFFFFFFF);
    Assert(Count == 117);

    return(0);
}
