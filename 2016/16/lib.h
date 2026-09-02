#if !defined(LIB_H)
#define LIB_H

#include <stdint.h>
#include <float.h>

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

inline char *
PushString(memory_arena *Arena, char *Source)
{
    s32 Size = 1;
    for(char *At = Source;
        *At;
        ++At)
    {
        ++Size;
    }
    char *Dest = PushArray(Arena, Size, char);
    for(s32 CharIndex = 0;
        CharIndex < Size;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }
    return(Dest);
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

inline u32
StringLength(char *Str)
{
    u32 Count = 0;
    while(*Str++)
    {
        ++Count;
    }
    return(Count);
}

inline u8
CharToHexDigit(char C)
{
    u8 Result = (C > '9') ? ((C - 'a') + 0xa) : (C - '0');
    return(Result);
}

inline char
HexDigitToChar(u8 D)
{
    char Result = (D > 0x9) ? ('a' + (D - 0xa)) : ('0' + D);
    return(Result);
}

internal u32
UInt32ToStr(u32 Val, char *Buffer, u32 BufferSize)
{
    char Reverse[64];
    u32 DigitCount;
    if(Val)
    {
        DigitCount = 0;
        for(; Val; Val /= 10)
        {
            Reverse[DigitCount++] = ('0' + (Val % 10));
        }
    }
    else
    {
        DigitCount = 1;
        Reverse[0] = '0';
    }
    Assert(DigitCount > 0);
    Assert(DigitCount <= BufferSize);
    char *Dest = Buffer;
    char *Source = Reverse + DigitCount - 1;
    for(u32 DigitIndex = 0;
        DigitIndex < DigitCount;
        ++DigitIndex)
    {
        *Dest++ = *Source--;
    }
    return(DigitCount);
}

#endif
