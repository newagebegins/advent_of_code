#include <stdint.h>

typedef uint32_t u32;

#define internal static
#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))
#define Assert(C) if(!(C)) {*(int *)0 = 0;}

internal void
Reverse(u32 ListSize, u32 *List, u32 StartPosition, u32 Length)
{
    for(u32 Offset = 0;
        Offset < (Length / 2);
        ++Offset)
    {
        u32 LeftIndex = (StartPosition + Offset) % ListSize;
        u32 RightIndex = (StartPosition + Length - Offset - 1) % ListSize;
        Assert(RightIndex >= 0);
        u32 Temp = List[LeftIndex];
        List[LeftIndex] = List[RightIndex];
        List[RightIndex] = Temp;
    }
}

internal void
FindHash(u32 ListSize, u32 *List, u32 LengthCount, u32 *Lengths)
{
    for(u32 ElementIndex = 0;
        ElementIndex < ListSize;
        ++ElementIndex)
    {
        List[ElementIndex] = ElementIndex;
    }

    u32 CurrentPosition = 0;
    u32 SkipSize = 0;

    for(u32 LengthIndex = 0;
        LengthIndex < LengthCount;
        ++LengthIndex)
    {
        u32 Length = Lengths[LengthIndex];
        Reverse(ListSize, List, CurrentPosition, Length);
        CurrentPosition = (CurrentPosition + (Length + SkipSize)) % ListSize;
        ++SkipSize;
    }
}

int
main(void)
{
    {
        u32 List[5];
        u32 Lengths[] = {3,4,1,5};
        FindHash(ArrayCount(List), List, ArrayCount(Lengths), Lengths);
        Assert((List[0] * List[1]) == 12);
    }

    {
        u32 List[256];
        u32 Lengths[] = {14,58,0,116,179,16,1,104,2,254,167,86,255,55,122,244};
        FindHash(ArrayCount(List), List, ArrayCount(Lengths), Lengths);
        Assert((List[0] * List[1]) == 1935);
    }

    return(0);
}
