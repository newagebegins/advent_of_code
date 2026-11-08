#if !defined(MAIN_H)
#define MAIN_H

#define MAX_STEP_COUNT ('Z'-'A'+1)

struct step
{
    u8 InCount;
    u8 OutCount;
    u8 Out[MAX_STEP_COUNT];
};

struct worker
{
    u8 StepIndex;
    u32 SecondsLeft;
};

#endif
