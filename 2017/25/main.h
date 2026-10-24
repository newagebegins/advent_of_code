#if !defined(MAIN_H)
#define MAIN_H

#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef int32_t b32;

#define internal static
#define local_persist static

#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))

#define Assert(C) if(!(C)) { *(int *)0 = 0; }
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); } break

struct parser
{
    char *At;
};

struct state_action
{
    u8 ValueToWrite;
    s32 MoveDir;
    u32 NextStateIndex;
};

struct state
{
    state_action Actions[2];
};

struct blueprint
{
    u32 StartStateIndex;
    u32 DiagnosticStepCount;

    u32 StateCount;
    state States[8];
};

struct machine
{
    u32 CurrentStateIndex;
    s32 TapeAt;
    u8 Tape[1 << 24];
};

#endif
