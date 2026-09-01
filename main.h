#if !defined(MAIN_H)
#define MAIN_H

struct str
{
    u32 Length;
    char *Text;
};

#define MAX_REGISTERS 4

struct computer_state
{
    u32 InstructionIndex;
    s32 Registers[MAX_REGISTERS];
};

enum instruction_type
{
    Instruction_cpy,
    Instruction_inc,
    Instruction_dec,
    Instruction_jnz,
    Instruction_tgl,
    Instruction_out,
};

enum argument_type
{
    Argument_None,
    Argument_Constant,
    Argument_Register,
};

struct argument
{
    argument_type Type;
    s32 Value;
};

struct instruction
{
    instruction_type Type;
    argument Arguments[2];
};

#endif
