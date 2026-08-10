#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

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

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

//
//
//

internal s32
FindWinnerElfIndex(s32 ElfCount)
{
    u8 *Elves = (u8 *)malloc(ElfCount * sizeof(u8));
    Assert(Elves);
    memset(Elves, 1, ElfCount);

    s32 WithPresentsCount = ElfCount;
    s32 ElfIndex = 0;

    for(;;)
    {
        if(Elves[ElfIndex])
        {
            if(WithPresentsCount == 1)
            {
                break;
            }
            else
            {
                do
                {
                    ++ElfIndex;
                    if(ElfIndex == ElfCount)
                    {
                        ElfIndex = 0;
                    }
                } while(!Elves[ElfIndex]);
                Elves[ElfIndex] = 0;
                --WithPresentsCount;
            }
        }
        else
        {
            ++ElfIndex;
            if(ElfIndex == ElfCount)
            {
                ElfIndex = 0;
            }
        }
    }

    free(Elves);
    return(ElfIndex + 1);
}

struct elf
{
    elf *Next;
    u32 Index;
};

internal u32
FindWinnerElfIndex2(u32 InitElfCount)
{
    elf *Elves = (elf *)malloc(InitElfCount * sizeof(elf));
    Assert(Elves);

    for(u32 ElfIndex = 0;
        ElfIndex < InitElfCount;
        ++ElfIndex)
    {
        elf *Elf = Elves + ElfIndex;
        Elf->Index = ElfIndex + 1;
        Elf->Next = Elves + ((ElfIndex + 1) % InitElfCount);
    }

    u32 ElfCount = InitElfCount;
    elf *Stealer = Elves + 0;
    elf *Elf = Stealer;
    s32 HopsDone = 0;

    while(ElfCount > 1)
    {
        s32 HopsToTarget = (ElfCount / 2);
        while(HopsDone < (HopsToTarget - 1))
        {
            Elf = Elf->Next;
            ++HopsDone;
        }
        Elf->Next = Elf->Next->Next;

        --ElfCount;
        Stealer = Stealer->Next;
        HopsDone -= 1;
    }

    u32 Winner = Stealer->Index;
    return(Winner);
}

int main(void)
{
    Assert(FindWinnerElfIndex(5) == 3);
    Assert(FindWinnerElfIndex2(5) == 2);

    u32 Input = 3017957;
    Assert(FindWinnerElfIndex(Input) == 1841611);
    Assert(FindWinnerElfIndex2(Input) == 1423634);

    return(0);
}
