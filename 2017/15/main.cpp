#include <stdint.h>

typedef uint32_t u32;
typedef uint64_t u64;

#define Assert(C) if(!(C)) { *(int *)0 = 0; }
#define internal static

struct generator
{
    u32 Value;
    u32 Factor;
};

inline void
InitializeGenerator(generator *Generator, u32 StartValue, u32 Factor)
{
    Generator->Value = StartValue;
    Generator->Factor = Factor;
}

inline u32
GenerateValue(generator *Generator)
{
    u64 Product = (u64)Generator->Value * (u64)Generator->Factor;
    Generator->Value = Product % 2147483647;
    return(Generator->Value);
}

internal u32
CountMatchingPairs(u32 StartA, u32 StartB)
{
    u32 Result = 0;
    generator GeneratorA, GeneratorB;
    InitializeGenerator(&GeneratorA, StartA, 16807);
    InitializeGenerator(&GeneratorB, StartB, 48271);
    for(u32 Iteration = 0;
        Iteration < 40000000;
        ++Iteration)
    {
        u32 ValueA = GenerateValue(&GeneratorA);
        u32 ValueB = GenerateValue(&GeneratorB);
        if((ValueA & 0xFFFF) == (ValueB & 0xFFFF))
        {
            ++Result;
        }
    }
    return(Result);
}

internal void
Test(u32 StartA, u32 StartB, u32 ExpectedCount)
{
    u32 Count = CountMatchingPairs(StartA, StartB);
    Assert(Count == ExpectedCount);
}

int
main(void)
{
    Test(65, 8921, 588);
    Test(591, 393, 619);
    return(0);
}
