#if !defined(MAIN_H)
#define MAIN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t s32;
typedef int64_t s64;
typedef int32_t b32;

#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))
#define Assert assert
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); } break

typedef int64_t register_value;

#define MAX_REGISTER_VALUE INT64_MAX
#define MIN_REGISTER_VALUE INT64_MIN

enum instruction_type
{
    Instruction_Unknown,
    Instruction_snd,
    Instruction_set,
    Instruction_add,
    Instruction_mul,
    Instruction_mod,
    Instruction_rcv,
    Instruction_jgz,

    Instruction_Count,
};

enum instruction_arg_type
{
    InstructionArg_Register,
    InstructionArg_Constant,
};

struct instruction_arg
{
    instruction_arg_type Type;
    union
    {
        u32 RegisterIndex;
        register_value ConstantValue;
    };
};

struct instruction_blueprint
{
    u32 ArgCount;
    b32 RegisterOnly[2];
};

static instruction_blueprint InstructionBlueprints[Instruction_Count] =
{
    // NOTE(slava): Instruction_Unknown
    {},
    // NOTE(slava): Instruction_snd
    {1},
    // NOTE(slava): Instruction_set
    {2, {true}},
    // NOTE(slava): Instruction_add
    {2, {true}},
    // NOTE(slava): Instruction_mul
    {2, {true}},
    // NOTE(slava): Instruction_mod
    {2, {true}},
    // NOTE(slava): Instruction_rcv
    {1},
    // NOTE(slava): Instruction_jgz
    {2},
};

struct instruction
{
    instruction_type Type;
    instruction_arg Args[2];
};

struct queue
{
    register_value Values[128];
    u32 BeginIndex;
    u32 EndIndex;
};

enum program_state
{
    ProgramState_Running,
    ProgramState_Waiting,
    ProgramState_Overflow,
    ProgramState_InvalidInstructionIndex,
};

struct program
{
    register_value Registers[26];
    register_value InstructionIndex;

    register_value InstructionCount;
    instruction *Instructions;

    program_state State;
    u32 ID;
    queue *SndQueue;
    queue *RcvQueue;
    u32 SendCount;
};

struct str
{
    char *Chars;
    u32 Length;
};

enum token_type
{
    Token_String,
    Token_Number,
    Token_EndOfFile,
};

struct token
{
    token_type Type;
    union
    {
        str String;
        register_value Number;
    };
};

struct parse_result
{
    register_value InstructionCount;
    instruction *Instructions;
};

struct parser
{
    char *At;
    b32 Error;
};

#endif
