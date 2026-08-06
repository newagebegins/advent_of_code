#include "lib.h"
#include "md5.cpp"

#include <stdlib.h>
#include <stdio.h>

struct node
{
    s32 X;
    s32 Y;
    char *Path;
    u32 PathLength;
};

struct queue
{
    node *Nodes;
    u32 MaxCount;
    u32 Count;
    u32 BeginIndex;
    u32 OnePastEndIndex;
};

internal void
InitializeQueue(queue *Queue, memory_arena *Arena, u32 MaxCount)
{
    Queue->Nodes = PushArray(Arena, MaxCount, node);
    Queue->MaxCount = MaxCount;
    Queue->Count = 0;
    Queue->BeginIndex = 0;
    Queue->OnePastEndIndex = 0;
}

internal void
Enqueue(queue *Queue, u32 X, u32 Y, char *Path, u32 PathLength)
{
    Assert(Queue->Count < Queue->MaxCount);
    ++Queue->Count;

    node *Node = Queue->Nodes + Queue->OnePastEndIndex++;
    if(Queue->OnePastEndIndex == Queue->MaxCount)
    {
        Queue->OnePastEndIndex = 0;
    }
    Node->X = X;
    Node->Y = Y;
    Node->Path = Path;
    Node->PathLength = PathLength;
}

internal node
Dequeue(queue *Queue)
{
    Assert(Queue->Count);
    --Queue->Count;
    node Result = Queue->Nodes[Queue->BeginIndex++];
    if(Queue->BeginIndex == Queue->MaxCount)
    {
        Queue->BeginIndex = 0;
    }
    return(Result);
}

inline u8
GetNibble(md5 Hash, u32 NibbleIndex)
{
    u8 Byte = Hash.Bytes[NibbleIndex/2];
    u32 Shift = (((NibbleIndex % 2) == 0) ? 4 : 0);
    u8 Result = ((Byte >> Shift) & 0xF);
    return(Result);
}

struct direction
{
    char Char;
    s32 dX;
    s32 dY;
};

internal char *
FindShortestPath(memory_arena *Arena, char *Passcode)
{
    u32 MaxPathSize = 64;
    char *Result = PushArray(Arena, MaxPathSize, char);

    u32 PasscodeLength = StringLength(Passcode);

    direction Directions[4] =
    {
        {'U', 0, -1},
        {'D', 0, 1},
        {'L', -1, 0},
        {'R', 1, 0},
    };

    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    queue Queue;
    InitializeQueue(&Queue, Arena, 512);
    Enqueue(&Queue, 0, 0, "", 0);

    node Node;

    for(;;)
    {
        Node = Dequeue(&Queue);

        if((Node.X == 3) && (Node.Y == 3))
        {
            break;
        }

        u32 PasscodePlusPathLength = PasscodeLength + Node.PathLength;
        char *PasscodePlusPath = PushArray(Arena, PasscodePlusPathLength + 1, char);
        Copy(PasscodeLength, Passcode, PasscodePlusPath);
        Copy(Node.PathLength + 1, Node.Path, PasscodePlusPath + PasscodeLength);

        md5 Hash = MD5(Arena, (u8 *)PasscodePlusPath, PasscodePlusPathLength);

        for(u32 NibbleIndex = 0;
            NibbleIndex < 4;
            ++NibbleIndex)
        {
            u8 Nibble = GetNibble(Hash, NibbleIndex);
            if(Nibble > 0xa)
            {
                direction Dir = Directions[NibbleIndex];

                s32 NewX = Node.X + Dir.dX;
                s32 NewY = Node.Y + Dir.dY;

                if((NewX >= 0) && (NewX <= 3) &&
                   (NewY >= 0) && (NewY <= 3))
                {
                    u32 NewPathLength = Node.PathLength + 1;
                    char *NewPath = PushArray(Arena, NewPathLength + 1, char);
                    Copy(Node.PathLength, Node.Path, NewPath);
                    NewPath[NewPathLength - 1] = Dir.Char;
                    NewPath[NewPathLength] = 0;
                    Enqueue(&Queue, NewX, NewY, NewPath, NewPathLength);
                }
            }
        }
    }

    Assert(Node.PathLength < MaxPathSize);
    Copy(Node.PathLength + 1, Node.Path, Result);

    EndTemporaryMemory(TempMem);

    return(Result);
}

inline void
FindShortestPathTestCase(memory_arena *Arena, char *Passcode, char *ExpectedPath)
{
    temporary_memory TempMem = BeginTemporaryMemory(Arena);
    char *Path = FindShortestPath(Arena, Passcode);
    Assert(StringsAreEqual(Path, ExpectedPath));
    EndTemporaryMemory(TempMem);
}

int main(void)
{
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(64);
    void *ArenaBase = malloc(ArenaSize);
    Assert(ArenaBase);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

    FindShortestPathTestCase(&Arena, "ihgpwlah", "DDRRRD");
    FindShortestPathTestCase(&Arena, "kglvqrro", "DDUDRLRRUDRD");

    char *Path = FindShortestPath(&Arena, "njfxhljp");
    printf("%s\n", Path);

    return(0);
}
