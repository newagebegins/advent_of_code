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
GetWatermark(memory_arena *Arena, memory_index Alignment = 4)
{
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Alignment);
    Assert((Arena->Used + AlignmentOffset) <= Arena->Size);
    u8 *Result = Arena->Base + Arena->Used + AlignmentOffset;
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


inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == '\n') || (C == '\r') || (C == ' ') || (C == '\t'));
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
SkipLine(char **AtPtr)
{
    char *At = *AtPtr;
    while(*At && (*At != '\n'))
    {
        ++At;
    }
    if(*At)
    {
        ++At;
    }
    *AtPtr = At;
}

inline void
SkipString(char **AtPtr, char *String)
{
    char *At = *AtPtr;
    while(*At && *String && (*At == *String))
    {
        ++At;
        ++String;
    }
    Assert(*String == 0);
    *AtPtr = At;
}

inline void
SkipChar(char **AtPtr, char Char)
{
    char *At = *AtPtr;
    Assert(*At == Char);
    ++At;
    *AtPtr = At;
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

struct position
{
    u32 X;
    u32 Y;
};

struct map
{
    char *Map;
    u32 CountX;
    u32 CountY;
};

inline b32
PositionsAreEqual(position A, position B)
{
    b32 Result = ((A.X == B.X) && (A.Y == B.Y));
    return(Result);
}

inline u32
Convert2DTo1D(u32 X, u32 Y, map Map)
{
    u32 Result = Y*Map.CountX + X;
    return(Result);
}

inline u32
Convert2DTo1D(position P, map Map)
{
    u32 Result = Convert2DTo1D(P.X, P.Y, Map);
    return(Result);
}

inline position
Convert1DTo2D(u32 Index, map Map)
{
    position Result;
    Result.X = Index % Map.CountX;
    Result.Y = Index / Map.CountX;
    return(Result);
}

//
//
//

struct heap
{
    u32 *PIndices;
    u32 Capacity;
    u32 Count;
    u32 *FScore;
};

internal heap
AllocateHeap(memory_arena *Arena, u32 Capacity, u32 *FScore)
{
    heap Heap;
    Heap.PIndices = PushArray(Arena, Capacity, u32);
    Heap.Capacity = Capacity;
    // NOTE(slava): Indexing is 1-based for ease of calculation
    Heap.Count = 1;
    Heap.FScore = FScore;
    return(Heap);
}

inline b32
IsLess(heap *Heap, u32 IndexA, u32 IndexB)
{
    Assert(IndexA < Heap->Count);
    Assert(IndexB < Heap->Count);
    u32 ScoreA = Heap->FScore[Heap->PIndices[IndexA]];
    u32 ScoreB = Heap->FScore[Heap->PIndices[IndexB]];
    b32 Result = (ScoreA < ScoreB);
    return(Result);
}

inline void
Swap(heap *Heap, u32 IndexA, u32 IndexB)
{
    u32 Temp = Heap->PIndices[IndexA];
    Heap->PIndices[IndexA] = Heap->PIndices[IndexB];
    Heap->PIndices[IndexB] = Temp;
}

internal void
Insert(heap *Heap, u32 PIndex)
{
    u32 NewIndex = Heap->Count++;
    Assert(NewIndex < Heap->Capacity);
    Heap->PIndices[NewIndex] = PIndex;
    u32 ParentIndex;
    while((ParentIndex = NewIndex / 2) &&
          IsLess(Heap, NewIndex, ParentIndex))
    {
        Swap(Heap, NewIndex, ParentIndex);
        NewIndex = ParentIndex;
    }
}

inline u32
GetLesserChildIndex(heap *Heap, u32 ParentIndex)
{
    u32 LeftChildIndex = 2*ParentIndex;
    u32 RightChildIndex = 2*ParentIndex + 1;
    u32 Result = LeftChildIndex;
    if((RightChildIndex < Heap->Count) &&
       IsLess(Heap, RightChildIndex, LeftChildIndex))
    {
        Result = RightChildIndex;
    }
    return(Result);
}

inline b32
NotEmpty(heap *Heap)
{
    b32 Result = (Heap->Count > 1);
    return(Result);
}

inline b32
Contains(heap *Heap, u32 PIndex)
{
    b32 Result = false;
    for(u32 I = 1;
        I < Heap->Count;
        ++I)
    {
        if(Heap->PIndices[I] == PIndex)
        {
            Result = true;
            break;
        }
    }
    return(Result);
}

internal u32
ExtractMin(heap *Heap)
{
    Assert(NotEmpty(Heap));
    u32 Result = Heap->PIndices[1];
    Heap->PIndices[1] = Heap->PIndices[--Heap->Count];
    u32 ParentIndex = 1;
    u32 LesserChildIndex;
    while(((LesserChildIndex = GetLesserChildIndex(Heap, ParentIndex)) < Heap->Count) &&
          IsLess(Heap, LesserChildIndex, ParentIndex))
    {
        Swap(Heap, LesserChildIndex, ParentIndex);
        ParentIndex = LesserChildIndex;
    }
    return(Result);
}

//
//
//

struct path
{
    u32 *PIndices;
    u32 Count;
    u32 Capacity;
};

inline u32
Distance(position P, position Goal)
{
    u32 Result = 0;
    Result += ((Goal.X > P.X) ? (Goal.X - P.X) : (P.X - Goal.X));
    Result += ((Goal.Y > P.Y) ? (Goal.Y - P.Y) : (P.Y - Goal.Y));
    return(Result);
}

internal void
ReconstructPath(path *Path, u32 *CameFrom, u32 CurrentPIndex)
{
    Assert(Path->Count < Path->Capacity);
    Path->PIndices[Path->Count++] = CurrentPIndex;
    for(;;)
    {
        u32 CameFromPIndex = CameFrom[CurrentPIndex];
        if(CameFromPIndex == UINT32_MAX)
        {
            break;
        }
        CurrentPIndex = CameFromPIndex;
        Assert(Path->Count < Path->Capacity);
        Path->PIndices[Path->Count++] = CurrentPIndex;
    }
    u32 LeftIndex = 0;
    u32 RightIndex = Path->Count - 1;
    while(LeftIndex < RightIndex)
    {
        u32 Temp = Path->PIndices[LeftIndex];
        Path->PIndices[LeftIndex] = Path->PIndices[RightIndex];
        Path->PIndices[RightIndex] = Temp;
        ++LeftIndex;
        --RightIndex;
    }
}

internal path
FindPath(memory_arena *Arena, map Map, u32 Start1D, u32 Goal1D)
{
    u32 CellCount = Map.CountX*Map.CountY;
    position Start = Convert1DTo2D(Start1D, Map);
    position Goal = Convert1DTo2D(Goal1D, Map);

    path Path = {};
    Path.Capacity = CellCount;
    Path.PIndices = PushArray(Arena, Path.Capacity, u32);

    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    u32 *FScore = PushArray(Arena, CellCount, u32);
    for(u32 CellIndex = 0;
        CellIndex < CellCount;
        ++CellIndex)
    {
        FScore[CellIndex] = UINT32_MAX;
    }
    FScore[Start1D] = Distance(Start, Goal);

    heap OpenSet = AllocateHeap(Arena, CellCount, FScore);
    Insert(&OpenSet, Start1D);

    u32 *CameFrom = PushArray(Arena, CellCount, u32);
    for(u32 CellIndex = 0;
        CellIndex < CellCount;
        ++CellIndex)
    {
        CameFrom[CellIndex] = UINT32_MAX;
    }

    u32 *GScore = PushArray(Arena, CellCount, u32);
    for(u32 CellIndex = 0;
        CellIndex < CellCount;
        ++CellIndex)
    {
        GScore[CellIndex] = UINT32_MAX;
    }
    GScore[Start1D] = 0;

    while(NotEmpty(&OpenSet))
    {
        u32 Current1D = ExtractMin(&OpenSet);
        if(Current1D == Goal1D)
        {
            ReconstructPath(&Path, CameFrom, Current1D);
            break;
        }
        u32 NeighborPIndices[4];
        u32 NeighborCount = 0;
        position Current = Convert1DTo2D(Current1D, Map);
        // NOTE(slava): Left
        if(Current.X > 0)
        {
            position Left = {Current.X - 1, Current.Y};
            u32 LeftIndex = Convert2DTo1D(Left, Map);
            char Val = Map.Map[LeftIndex];
            if(Val != '#')
            {
                NeighborPIndices[NeighborCount++] = LeftIndex;
            }
        }
        // NOTE(slava): Right
        if((Current.X + 1) < Map.CountX)
        {
            position Right = {Current.X + 1, Current.Y};
            u32 RightIndex = Convert2DTo1D(Right, Map);
            char Val = Map.Map[RightIndex];
            if(Val != '#')
            {
                NeighborPIndices[NeighborCount++] = RightIndex;
            }
        }
        // NOTE(slava): Up
        if(Current.Y > 0)
        {
            position Up = {Current.X, Current.Y - 1};
            u32 UpIndex = Convert2DTo1D(Up, Map);
            char Val = Map.Map[UpIndex];
            if(Val != '#')
            {
                NeighborPIndices[NeighborCount++] = UpIndex;
            }
        }
        // NOTE(slava): Down
        if((Current.Y + 1) < Map.CountY)
        {
            position Down = {Current.X, Current.Y + 1};
            u32 DownIndex = Convert2DTo1D(Down, Map);
            char Val = Map.Map[DownIndex];
            if(Val != '#')
            {
                NeighborPIndices[NeighborCount++] = DownIndex;
            }
        }
        for(u32 NeighborI = 0;
            NeighborI < NeighborCount;
            ++NeighborI)
        {
            u32 NeighborIndex = NeighborPIndices[NeighborI];
            u32 TentativeGScore = GScore[Current1D] + 1;
            if(TentativeGScore < GScore[NeighborIndex])
            {
                CameFrom[NeighborIndex] = Current1D;
                GScore[NeighborIndex] = TentativeGScore;
                FScore[NeighborIndex] = (TentativeGScore +
                                         Distance(Convert1DTo2D(NeighborIndex, Map), Goal));
                if(!Contains(&OpenSet, NeighborIndex))
                {
                    Insert(&OpenSet, NeighborIndex);
                }
            }
        }
    }

    EndTemporaryMemory(TempMem);

    return(Path);
}

//
//
//

inline b32
IsNeighbor(u32 AIndex, u32 BIndex, map Map)
{
    position A = Convert1DTo2D(AIndex, Map);
    position B = Convert1DTo2D(BIndex, Map);
    b32 Result = (Distance(A, B) == 1);
    return(Result);
}

inline b32
IsNewLine(char C)
{
    b32 Result = ((C == '\n') || (C == '\r'));
    return(Result);
}

internal map
CreateMap(memory_arena *Arena, char *Input)
{
    map Result = {};
    for(char *At = Input;
        *At;
        ++At)
    {
        if(IsNewLine(*At))
        {
            Result.CountX = (u32)(At - Input);
            break;
        }
    }
    char *Row = Input;
    char *Col = Row;
    while(*Col)
    {
        if(IsNewLine(*Col))
        {
            ++Result.CountY;
            Assert((Col - Row) == Result.CountX);
            while(IsNewLine(*Col))
            {
                ++Col;
            }
            Row = Col;
        }
        else
        {
            ++Col;
        }
    }
    u32 CellCount = Result.CountX*Result.CountY;
    Result.Map = PushArray(Arena, CellCount, char);
    char *Source = Input;
    char *Dest = Result.Map;
    while(*Source)
    {
        if(IsNewLine(*Source))
        {
            ++Source;
        }
        else
        {
            *Dest++ = *Source++;
        }
    }
    Assert((Dest - Result.Map) == CellCount);
    return(Result);
}

#define PERMUTATION_CALLBACK(name) void name(u32 SetSize, u32 *Counters, void *ContextInit)
typedef PERMUTATION_CALLBACK(permutation_callback);

PERMUTATION_CALLBACK(PrintPermutation)
{
    for(u32 I = 0;
        I < SetSize;
        ++I)
    {
        printf("%u ", Counters[I]);
    }
    printf("\n");                
}

struct find_shortest_route_context
{
    u32 Distances[10][10];
    u32 MinTotalDistance;
    u32 MinRoute[10];
    b32 ComeBack;
};

PERMUTATION_CALLBACK(FindShortestRoute)
{
    find_shortest_route_context *Context = (find_shortest_route_context *)ContextInit;
    u32 TotalDistance = 0;
    u32 From = 0;
    for(u32 I = 0;
        I < SetSize;
        ++I)
    {
        u32 To = Counters[I] + 1;
        TotalDistance += Context->Distances[From][To];
        From = To;
    }
    if(Context->ComeBack)
    {
        TotalDistance += Context->Distances[From][0];
    }
    if(TotalDistance < Context->MinTotalDistance)
    {
        Context->MinTotalDistance = TotalDistance;
        Context->MinRoute[0] = 0;
        for(u32 I = 0;
            I < SetSize;
            ++I)
        {
            Context->MinRoute[I + 1] = Counters[I] + 1;
        }
    }
}

internal void
ForEachPermutation(u32 SetSize, permutation_callback *Callback, void *Context)
{
    u32 Counters[10] = {};
    Assert(SetSize <= ArrayCount(Counters));
    u32 CounterIndex = 0;
    b32 Forward = true;
    for(;;)
    {
        if(Forward)
        {
            Counters[CounterIndex] = 0;
            Forward = false;
        }
        while(Counters[CounterIndex] < SetSize)
        {
            b32 IsUnique = true;
            for(u32 PrevCounterIndex = 0;
                PrevCounterIndex < CounterIndex;
                ++PrevCounterIndex)
            {
                if(Counters[CounterIndex] == Counters[PrevCounterIndex])
                {
                    IsUnique = false;
                    break;
                }
            }
            if(IsUnique)
            {
                if(CounterIndex == (SetSize - 1))
                {
                    Callback(SetSize, Counters, Context);
                }
                else
                {
                    Forward = true;
                }
            }
            if(Forward)
            {
                ++CounterIndex;
                break;
            }
            else
            {
                ++Counters[CounterIndex];
            }
        }
        if(!Forward)
        {
            if(CounterIndex)
            {
                --CounterIndex;
                ++Counters[CounterIndex];
            }
            else
            {
                break;
            }
        }
    }
}

internal void
DoProblem(memory_arena *Arena, char *FileName, b32 ComeBack = false)
{
    temporary_memory ProblemTempMem = BeginTemporaryMemory(Arena);

    char *Input = ReadEntireFile(Arena, FileName);
    map Map = CreateMap(Arena, Input);

    position Positions[10] = {};
    u32 PositionCount = 0;

    for(u32 Y = 0;
        Y < Map.CountY;
        ++Y)
    {
        for(u32 X = 0;
            X < Map.CountX;
            ++X)
        {
            position P = {X, Y};
            u32 Index = Convert2DTo1D(P, Map);
            char Char = Map.Map[Index];
            if(IsDigit(Char))
            {
                u32 PositionIndex = Char - '0';
                Positions[PositionIndex] = P;
                ++PositionCount;
            }
        }
    }

    find_shortest_route_context Context = {};
    Context.MinTotalDistance = UINT_MAX;
    Context.ComeBack = ComeBack;

    for(u32 FromIndex = 0;
        FromIndex < PositionCount;
        ++FromIndex)
    {
        position From = Positions[FromIndex];
        u32 From1D = Convert2DTo1D(From, Map);
        for(u32 ToIndex = 0;
            ToIndex < PositionCount;
            ++ToIndex)
        {
            position To = Positions[ToIndex];
            u32 To1D = Convert2DTo1D(To, Map);

            temporary_memory TempMem = BeginTemporaryMemory(Arena);
            path Path = FindPath(Arena, Map, From1D, To1D);
            u32 Distance = Path.Count - 1;
            Context.Distances[FromIndex][ToIndex] = Distance;
            EndTemporaryMemory(TempMem);

        }
    }

    ForEachPermutation(PositionCount - 1, FindShortestRoute, &Context);

    printf("Shortest route is:\n");
    for(u32 Index = 0;
        Index < PositionCount;
        ++Index)
    {
        printf("%u ", Context.MinRoute[Index]);
    }
    printf("\n");
    printf("Its length is: %u\n", Context.MinTotalDistance);

    EndTemporaryMemory(ProblemTempMem);
}

int
main(void)
{
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(64);
    void *ArenaBase = malloc(ArenaSize);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

    DoProblem(&Arena, "test_input.txt");
    DoProblem(&Arena, "input.txt");
    DoProblem(&Arena, "input.txt", true);

    return(0);
}
