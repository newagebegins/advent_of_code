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
    Instruction_cpy_const,
    Instruction_cpy_reg,
    Instruction_inc,
    Instruction_dec,
    Instruction_jnz_const,
    Instruction_jnz_reg,
};

struct instruction
{
    instruction_type Type;
    s32 X;
    s32 Y;
};

#endif
