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

enum TileType
{
    Tile_Viable,
    Tile_Unviable,
    Tile_Empty,
    Tile_Goal,
};

struct position
{
    u32 X;
    u32 Y;
};

struct map
{
    u8 *Map;
    u32 CountX;
    u32 CountY;
};

internal void
PrintMap(map Map)
{
    u32 OriginX = 0;
    u32 OriginY = 0;

    for(u32 Y = 0;
        Y < Map.CountY;
        ++Y)
    {
        for(u32 X = 0;
            X < Map.CountX;
            ++X)
        {
            u8 Val = Map.Map[Y*Map.CountX + X];

            if((Y == OriginY) && (X == OriginX))
            {
                printf("(");
            }
            else
            {
                printf(" ");
            }

            switch(Val)
            {
                case Tile_Viable:
                {
                    printf(".");
                } break;

                case Tile_Unviable:
                {
                    printf("#");
                } break;

                case Tile_Empty:
                {
                    printf("_");
                } break;

                case Tile_Goal:
                {
                    printf("G");
                } break;
            }

            if((Y == OriginY) && (X == OriginX))
            {
                printf(")");
            }
            else
            {
                printf(" ");
            }
        }

        printf("\n");
    }
}

inline b32
PositionsAreEqual(position A, position B)
{
    b32 Result = ((A.X == B.X) && (A.Y == B.Y));
    return(Result);
}

inline u32
PositionToIndex(position P, map Map)
{
    u32 Result = P.Y*Map.CountX + P.X;
    return(Result);
}

inline position
IndexToPosition(u32 Index, map Map)
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
    map Map;
    u32 *FScore;
};

internal heap
AllocateHeap(memory_arena *Arena, u32 Capacity, map Map, u32 *FScore)
{
    heap Heap;
    Heap.PIndices = PushArray(Arena, Capacity, u32);
    Heap.Capacity = Capacity;
    // NOTE(slava): Indexing is 1-based for ease of calculation
    Heap.Count = 1;
    Heap.Map = Map;
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
FindPath(memory_arena *Arena, map Map, u32 StartIndex, u32 GoalIndex)
{
    u32 CellCount = Map.CountX*Map.CountY;
    position Start = IndexToPosition(StartIndex, Map);
    position Goal = IndexToPosition(GoalIndex, Map);

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
    FScore[StartIndex] = Distance(Start, Goal);

    heap OpenSet = AllocateHeap(Arena, CellCount, Map, FScore);
    Insert(&OpenSet, StartIndex);

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
    GScore[StartIndex] = 0;

    while(NotEmpty(&OpenSet))
    {
        u32 CurrentIndex = ExtractMin(&OpenSet);
        if(CurrentIndex == GoalIndex)
        {
            ReconstructPath(&Path, CameFrom, CurrentIndex);
            break;
        }
        u32 NeighborPIndices[4];
        u32 NeighborCount = 0;
        position Current = IndexToPosition(CurrentIndex, Map);
        // NOTE(slava): Left
        if(Current.X > 0)
        {
            position Left = {Current.X - 1, Current.Y};
            u32 LeftIndex = PositionToIndex(Left, Map);
            u32 Val = Map.Map[LeftIndex];
            if(Val == Tile_Viable)
            {
                NeighborPIndices[NeighborCount++] = LeftIndex;
            }
        }
        // NOTE(slava): Right
        if((Current.X + 1) < Map.CountX)
        {
            position Right = {Current.X + 1, Current.Y};
            u32 RightIndex = PositionToIndex(Right, Map);
            u32 Val = Map.Map[RightIndex];
            if(Val == Tile_Viable)
            {
                NeighborPIndices[NeighborCount++] = RightIndex;
            }
        }
        // NOTE(slava): Up
        if(Current.Y > 0)
        {
            position Up = {Current.X, Current.Y - 1};
            u32 UpIndex = PositionToIndex(Up, Map);
            u32 Val = Map.Map[UpIndex];
            if(Val == Tile_Viable)
            {
                NeighborPIndices[NeighborCount++] = UpIndex;
            }
        }
        // NOTE(slava): Down
        if((Current.Y + 1) < Map.CountY)
        {
            position Down = {Current.X, Current.Y + 1};
            u32 DownIndex = PositionToIndex(Down, Map);
            u32 Val = Map.Map[DownIndex];
            if(Val == Tile_Viable)
            {
                NeighborPIndices[NeighborCount++] = DownIndex;
            }
        }
        for(u32 NeighborI = 0;
            NeighborI < NeighborCount;
            ++NeighborI)
        {
            u32 NeighborIndex = NeighborPIndices[NeighborI];
            u32 TentativeGScore = GScore[CurrentIndex] + 1;
            if(TentativeGScore < GScore[NeighborIndex])
            {
                CameFrom[NeighborIndex] = CurrentIndex;
                GScore[NeighborIndex] = TentativeGScore;
                FScore[NeighborIndex] = (TentativeGScore +
                                         Distance(IndexToPosition(NeighborIndex, Map), Goal));
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

struct node
{
    u32 X;
    u32 Y;
    u32 Used;
    u32 Avail;
};

struct nodes
{
    node *Nodes;
    u32 Count;
};

struct node_pair
{
    node A;
    node B;
};

struct node_pairs
{
    node_pair *Pairs;
    u32 Count;
};

internal node_pairs
FindViablePairs(memory_arena *Arena, nodes Nodes)
{
    node_pairs Result = {};
    Result.Pairs = (node_pair *)GetWatermark(Arena);
    for(u32 AIndex = 0;
        AIndex < Nodes.Count;
        ++AIndex)
    {
        node A = Nodes.Nodes[AIndex];
        for(u32 BIndex = 0;
            BIndex < Nodes.Count;
            ++BIndex)
        {
            node B = Nodes.Nodes[BIndex];
            if(A.Used && (AIndex != BIndex) && (A.Used <= B.Avail))
            {
                PushStruct(Arena, node_pair);
                Result.Pairs[Result.Count].A = A;
                Result.Pairs[Result.Count].B = B;
                ++Result.Count;
            }
        }
    }
    return(Result);
}

internal nodes
ParseNodes(memory_arena *Arena, char *Input)
{
    nodes Result = {};
    Result.Nodes = (node *)GetWatermark(Arena);
    char *At = Input;
    SkipWhitespace(&At);
    SkipLine(&At);
    SkipLine(&At);
    while(*At)
    {
        // Filesystem              Size  Used  Avail  Use%
        // /dev/grid/node-x0-y0     94T   67T    27T   71%
        SkipString(&At, "/dev/grid/node-x");
        u32 X = ParseU32(&At);
        SkipString(&At, "-y");
        u32 Y = ParseU32(&At);
        SkipWhitespace(&At);
        u32 Size = ParseU32(&At);
        SkipChar(&At, 'T');
        SkipWhitespace(&At);
        u32 Used = ParseU32(&At);
        SkipChar(&At, 'T');
        SkipWhitespace(&At);
        u32 Avail = ParseU32(&At);
        SkipChar(&At, 'T');
        SkipWhitespace(&At);
        u32 UsePercent = ParseU32(&At);
        SkipChar(&At, '%');
        SkipWhitespace(&At);

        node *Node = PushStruct(Arena, node);
        Node->X = X;
        Node->Y = Y;
        Node->Used = Used;
        Node->Avail = Avail;

        ++Result.Count;
    }
    return(Result);
}

struct swap
{
    u32 A;
    u32 B;
};

struct swaps
{
    swap *Swaps;
    u32 Count;
};

inline b32
IsNeighbor(u32 AIndex, u32 BIndex, map Map)
{
    position A = IndexToPosition(AIndex, Map);
    position B = IndexToPosition(BIndex, Map);
    b32 Result = (Distance(A, B) == 1);
    return(Result);
}

internal swaps
FindSwapsToAccessGoalData(memory_arena *Arena, map Map, u32 EmptyP)
{
    swaps Result = {};
    u32 MaxSwapCount = Map.CountX*Map.CountY;
    Result.Swaps = PushArray(Arena, MaxSwapCount, swap);
    u32 GoalP = Map.CountX - 1;
    u32 OriginP = 0;
    path GoalToOrigin = FindPath(Arena, Map, GoalP, OriginP);
    for(u32 I = 1;
        I < GoalToOrigin.Count;
        ++I)
    {
        u32 P = GoalToOrigin.PIndices[I];
        temporary_memory TempMem = BeginTemporaryMemory(Arena);
        path EmptyPToP = FindPath(Arena, Map, EmptyP, P);
        for(u32 J = 1;
            J < EmptyPToP.Count;
            ++J)
        {
            Assert(Result.Count < MaxSwapCount);
            Result.Swaps[Result.Count].A = EmptyPToP.PIndices[J - 1];
            Result.Swaps[Result.Count].B = EmptyPToP.PIndices[J];
            ++Result.Count;

            u8 TempVal = Map.Map[EmptyPToP.PIndices[J]];
            Map.Map[EmptyPToP.PIndices[J]] = Map.Map[EmptyPToP.PIndices[J - 1]];
            Map.Map[EmptyPToP.PIndices[J - 1]] = TempVal;
#if 0
            printf("I=%u/%u, J=%u/%u\n", I, GoalToOrigin.Count, J, EmptyPToP.Count);
            PrintMap(Map);
#endif
        }
        EmptyP = EmptyPToP.PIndices[EmptyPToP.Count - 1];

        Assert(Result.Count < MaxSwapCount);
        Result.Swaps[Result.Count].A = EmptyP;
        Result.Swaps[Result.Count].B = GoalP;
        ++Result.Count;

        Assert(IsNeighbor(EmptyP, GoalP, Map));
        Map.Map[EmptyP] = Tile_Goal;
        Map.Map[GoalP] = Tile_Empty;
#if 0
        printf("I=%u/%u\n", I, GoalToOrigin.Count);
        PrintMap(Map);
#endif

        u32 TempP = EmptyP;
        EmptyP = GoalP;
        GoalP = TempP;
        Assert(GoalP == P);

        EndTemporaryMemory(TempMem);
    }
    return(Result);
}

struct create_map_result
{
    map Map;
    u32 EmptyP;
};

internal create_map_result
CreateMap(memory_arena *Arena, nodes Nodes, node_pairs Pairs)
{
    create_map_result Result = {};
    Result.EmptyP = UINT32_MAX;

    u32 MaxX = 0;
    u32 MaxY = 0;

    for(u32 NodeIndex = 0;
        NodeIndex < Nodes.Count;
        ++NodeIndex)
    {
        node Node = Nodes.Nodes[NodeIndex];
        if(MaxX < Node.X)
        {
            MaxX = Node.X;
        }
        if(MaxY < Node.Y)
        {
            MaxY = Node.Y;
        }
    }

    Result.Map.CountX = MaxX + 1;
    Result.Map.CountY = MaxY + 1;

    Result.Map.Map = PushArray(Arena, Result.Map.CountX*Result.Map.CountY, u8);

    for(u32 Y = 0;
        Y < Result.Map.CountY;
        ++Y)
    {
        for(u32 X = 0;
            X < Result.Map.CountX;
            ++X)
        {
            Result.Map.Map[Y*Result.Map.CountX + X] = Tile_Unviable;
        }
    }

    for(u32 PairIndex = 0;
        PairIndex < Pairs.Count;
        ++PairIndex)
    {
        node_pair Pair = Pairs.Pairs[PairIndex];
        Result.Map.Map[Pair.A.Y*Result.Map.CountX + Pair.A.X] = Tile_Viable;
        u32 EmptyP = Pair.B.Y*Result.Map.CountX + Pair.B.X;
        if(Result.EmptyP == UINT32_MAX)
        {
            Result.EmptyP = EmptyP;
            Result.Map.Map[Result.EmptyP] = Tile_Empty;
        }
        else
        {
            // NOTE(slava): We support only one empty position on the map
            Assert(Result.EmptyP == EmptyP);
        }
    }

    Result.Map.Map[Result.Map.CountX - 1] = Tile_Goal;

    return(Result);
}

int main(void)
{
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(64);
    void *ArenaBase = malloc(ArenaSize);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

    {
        char *Input = ReadEntireFile(&Arena, "test_input.txt");
        nodes Nodes = ParseNodes(&Arena, Input);
        node_pairs Pairs = FindViablePairs(&Arena, Nodes);
        Assert(Pairs.Count == 7);
        create_map_result CreateMapResult = CreateMap(&Arena, Nodes, Pairs);
#if 0
        PrintMap(CreateMapResult.Map);
#endif
        swaps Swaps = FindSwapsToAccessGoalData(&Arena, CreateMapResult.Map, CreateMapResult.EmptyP);
        Assert(Swaps.Count == 7);
    }

    {
        char *Input = ReadEntireFile(&Arena, "input.txt");
        nodes Nodes = ParseNodes(&Arena, Input);
        node_pairs Pairs = FindViablePairs(&Arena, Nodes);
        Assert(Pairs.Count == 864);
        create_map_result CreateMapResult = CreateMap(&Arena, Nodes, Pairs);
#if 0
        PrintMap(CreateMapResult.Map);
#endif
        swaps Swaps = FindSwapsToAccessGoalData(&Arena, CreateMapResult.Map, CreateMapResult.EmptyP);
        Assert(Swaps.Count == 244);
    }
}
