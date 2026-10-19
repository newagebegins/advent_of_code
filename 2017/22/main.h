#if !defined(MAIN_H)
#define MAIN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef int32_t b32;

typedef size_t memory_index;

#define internal static
#define local_persist static

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#define Assert(Condition) if(!(Condition)) { *(int *)0 = 0; }
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); } break

#define ZeroStruct(Struct) ZeroMemory(sizeof(Struct), &(Struct))

inline void
ZeroMemory(memory_index Size, void *Ptr)
{
    u8 *Bytes = (u8 *)Ptr;
    for(memory_index Index = 0;
        Index < Size;
        ++Index)
    {
        Bytes[Index] = 0;
    }
}

struct memory_arena
{
    void *Base;
    memory_index Size;
    memory_index Used;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Base = Base;
    Arena->Size = Size;
    Arena->Used = 0;
}

inline void
Clear(memory_arena *Arena)
{
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))

inline void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = (u8 *)Arena->Base + Arena->Used;
    Arena->Used += Size;
    return(Result);
}

enum node_state
{
    Node_Clean,
    Node_Infected,
    Node_Weakened,
    Node_Flagged,
};

struct node
{
    node_state State;
    s32 X;
    s32 Y;
    node *NextInHash;
};

#define HASH_SIZE_POW_2 12
#define HASH_SIZE (1 << HASH_SIZE_POW_2)
#define HASH_MASK (HASH_SIZE - 1)

struct grid
{
    node *Nodes[HASH_SIZE];
};

struct parser
{
    char *At;
};

#endif
