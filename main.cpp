#include "lib.h"
#include "md5.cpp"

#include <stdlib.h>

int
main(void)
{
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(1);
    void *ArenaBase = malloc(ArenaSize);
    Assert(ArenaBase);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

    TestMD5(&Arena);

    return(0);
}
