#include <stdint.h>

#define internal static
#define local_persist static

#define ArrayCount(A) (sizeof(A)/sizeof(A[0]))
#define Assert(C) if(!(C)) {*(int *)0 = 0;}

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t b32;

struct memory_state
{
    u8 Banks[16];
};

internal u32
ChooseBankToRedistribute(memory_state *State, u32 BankCount)
{
    u32 Result = 0;
    u8 MaxBlocks = 0;
    for(u32 BankIndex = 0;
        BankIndex < BankCount;
        ++BankIndex)
    {
        if(MaxBlocks < State->Banks[BankIndex])
        {
            MaxBlocks = State->Banks[BankIndex];
            Result = BankIndex;
        }
    }
    return(Result);
}

internal void
RedistributeBlocks(memory_state *State, u32 BankCount)
{
    u32 BankIndexToRedistribute = ChooseBankToRedistribute(State, BankCount);
    u32 BlocksToRedistribute = State->Banks[BankIndexToRedistribute];
    State->Banks[BankIndexToRedistribute] = 0;
    u32 BankIndex = BankIndexToRedistribute;
    while(BlocksToRedistribute)
    {
        ++BankIndex;
        if(BankIndex == BankCount)
        {
            BankIndex = 0;
        }
        ++State->Banks[BankIndex];
        --BlocksToRedistribute;
    }
}

internal b32
StatesAreEqual(memory_state *A, memory_state *B, u32 BankCount)
{
    b32 Result = true;
    for(u32 BankIndex = 0;
        BankIndex < BankCount;
        ++BankIndex)
    {
        if(A->Banks[BankIndex] != B->Banks[BankIndex])
        {
            Result = false;
            break;
        }
    }
    return(Result);
}

struct count_cycles_result
{
    u32 UntilRepeat;
    u32 LoopSize;
};

internal count_cycles_result
CountRedistributionCyclesUntilRepeat(memory_state *State, u32 BankCount)
{
    count_cycles_result Result = {};
    local_persist memory_state Cache[4096];
    u32 CycleCount = 0;
    b32 FoundRepeat = false;

    while(!FoundRepeat)
    {
        RedistributeBlocks(State, BankCount);

        Assert(CycleCount < ArrayCount(Cache));
        Cache[CycleCount] = *State;
        ++CycleCount;

        for(u32 CacheIndex = 0;
            CacheIndex < (CycleCount - 1);
            ++CacheIndex)
        {
            if(StatesAreEqual(State, Cache + CacheIndex, BankCount))
            {
                Result.UntilRepeat = CycleCount;
                Result.LoopSize = (CycleCount - 1) - CacheIndex;
                FoundRepeat = true;
                break;
            }
        }
    }
    return(Result);
}

int
main(void)
{
    {
        memory_state State = {0, 2, 7, 0};
        count_cycles_result Cycles = CountRedistributionCyclesUntilRepeat(&State, 4);
        Assert(Cycles.UntilRepeat == 5);
        Assert(Cycles.LoopSize == 4);
    }

    {
        memory_state State = {11, 11, 13, 7, 0, 15, 5, 5, 4, 4, 1, 1, 7, 1, 15, 11};
        count_cycles_result Cycles = CountRedistributionCyclesUntilRepeat(&State, 16);
        Assert(Cycles.UntilRepeat == 4074);
        Assert(Cycles.LoopSize == 2793);
    }
}
