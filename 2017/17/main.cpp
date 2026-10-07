#include <stdint.h>
#include <stdio.h>

#define local_persist static

typedef uint16_t u16;
typedef uint32_t u32;

int
main(/*int ArgCount, char **Args*/)
{
    local_persist u16 Buffer[2018];
    Buffer[0] = 0;
    u32 StepsPerInsert = 316;
    u32 CurrentIndex = 0;
    for(u16 Value = 1;
        Value < 2018;
        ++Value)
    {
        CurrentIndex = ((CurrentIndex + StepsPerInsert) % Value) + 1;
        for(u32 Index = Value - 1;
            Index >= CurrentIndex;
            --Index)
        {
            Buffer[Index + 1] = Buffer[Index];
        }
        Buffer[CurrentIndex] = Value;
    }
    printf("%u\n", Buffer[CurrentIndex + 1]);
    return(0);
}
