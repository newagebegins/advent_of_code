#include "lib.h"

#include <stdlib.h>
#include <stdio.h>

struct disk
{
    u32 Position;
    u32 PositionCount;
};

#define MAX_DISK_COUNT 8

struct state
{
    disk Disks[MAX_DISK_COUNT];
    u32 DiskCount;
};

internal u32
FindTimeToPressButton(state *InitialState)
{
    u32 TimeToPressButton = 0;
    state State = *InitialState;
    for(;
        ;
        ++TimeToPressButton)
    {
        State = *InitialState;
        for(u32 DiskIndex = 0;
            DiskIndex < State.DiskCount;
            ++DiskIndex)
        {
            disk *Disk = State.Disks + DiskIndex;
            Disk->Position = (Disk->Position + TimeToPressButton) % Disk->PositionCount;
        }
        b32 Bounce = false;
        for(u32 DiskIndex = 0;
            DiskIndex < State.DiskCount;
            ++DiskIndex)
        {
            disk *Disk = State.Disks + DiskIndex;
            if(((Disk->Position + DiskIndex) % Disk->PositionCount) != 0)
            {
                Bounce = true;
                break;
            }
        }

        if(!Bounce)
        {
            --TimeToPressButton;
            break;
        }
    }
    return(TimeToPressButton);
}

int
main(void)
{
    state State = {};
    u32 Result;

    State.Disks[State.DiskCount++] = { 4, 5 };
    State.Disks[State.DiskCount++] = { 1, 2 };
    Result = FindTimeToPressButton(&State);
    Assert(Result == 5);

    State = {};
    State.Disks[State.DiskCount++] = { 5, 17 };
    State.Disks[State.DiskCount++] = { 8, 19 };
    State.Disks[State.DiskCount++] = { 1, 7 };
    State.Disks[State.DiskCount++] = { 7, 13 };
    State.Disks[State.DiskCount++] = { 1, 5 };
    State.Disks[State.DiskCount++] = { 0, 3 };
    Result = FindTimeToPressButton(&State);
    Assert(Result == 16824);

    State.Disks[State.DiskCount++] = { 0, 11 };
    Result = FindTimeToPressButton(&State);
    Assert(Result == 3543984);

    return(0);
}
