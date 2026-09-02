#include "lib.h"

#include <stdlib.h>
#include <stdio.h>

global_variable memory_arena GlobalArena_;
global_variable memory_arena *GlobalArena = &GlobalArena_;

struct bit_string
{
    char *Bits;
    u32 BitCount;
};

internal bit_string
ToBitString(char *Bits)
{
    bit_string Result;
    Result.BitCount = StringLength(Bits);
    u32 DataSize = Result.BitCount + 1;
    Result.Bits = PushArray(GlobalArena, DataSize, char);
    for(u32 Index = 0;
        Index < DataSize;
        ++Index)
    {
        Result.Bits[Index] = Bits[Index];
    }
    return(Result);
}

internal bit_string
DragonCurve(bit_string Input)
{
    bit_string Result;
    Result.BitCount = 2*Input.BitCount + 1;
    Result.Bits = PushArray(GlobalArena, Result.BitCount + 1, char);
    char *Dest = Result.Bits;
    char *Source = Input.Bits;
    for(u32 BitIndex = 0;
        BitIndex < Input.BitCount;
        ++BitIndex)
    {
        *Dest++ = *Source++;
    }
    *Dest++ = '0';
    Source = Input.Bits + Input.BitCount - 1;
    for(u32 BitIndex = 0;
        BitIndex < Input.BitCount;
        ++BitIndex)
    {
        char InBit = *Source--;
        char NewBit = ((InBit == '0') ? '1' : '0');
        *Dest++ = NewBit;
    }
    *Dest = 0;
    return(Result);
}

internal bit_string
FindChecksumInternal(bit_string Input)
{
    Assert((Input.BitCount % 2) == 0);
    bit_string Result;
    Result.BitCount = Input.BitCount / 2;
    Result.Bits = PushArray(GlobalArena, Result.BitCount + 1, char);
    char *Dest = Result.Bits;
    for(u32 BitIndex = 0;
        BitIndex < Input.BitCount;
        BitIndex += 2)
    {
        char C1 = Input.Bits[BitIndex];
        char C2 = Input.Bits[BitIndex + 1];
        *Dest++ = ((C1 == C2) ? '1' : '0');
    }
    *Dest = 0;
    return(Result);
}

internal bit_string
FindChecksum(bit_string Input)
{
    bit_string Result = Input;
    do
    {
        Result = FindChecksumInternal(Result);
    } while((Result.BitCount % 2) == 0);
    return(Result);
}

internal bit_string
FillDisk(u32 DiskLength, bit_string Input)
{
    bit_string Result = Input;
    while(Result.BitCount < DiskLength)
    {
        Result = DragonCurve(Result);
    }
    Result.BitCount = DiskLength;
    Result.Bits[Result.BitCount] = 0;
    return(Result);
}

inline b32
BitStringsAreEqual(bit_string A, bit_string B)
{
    b32 Result;
    if(A.BitCount == B.BitCount)
    {
        Result = true;
        for(u32 BitIndex = 0;
            BitIndex < A.BitCount;
            ++BitIndex)
        {
            if(A.Bits[BitIndex] != B.Bits[BitIndex])
            {
                Result = false;
                break;
            }
        }
    }
    else
    {
        Result = false;
    }
    return(Result);
}

internal void
DragonCurveTestCase(char *InitBits, char *ExpectedBits)
{
    bit_string Str = ToBitString(InitBits);
    bit_string Expected = ToBitString(ExpectedBits);
    bit_string New = DragonCurve(Str);
    Assert(BitStringsAreEqual(New, Expected));
}

internal void
ChecksumTestCase(char *InitBits, char *ExpectedBits)
{
    bit_string Str = ToBitString(InitBits);
    bit_string Expected = ToBitString(ExpectedBits);
    bit_string Checksum = FindChecksum(Str);
    Assert(BitStringsAreEqual(Checksum, Expected));
}

internal void
FillDiskTestCase(u32 DiskLength, char *InitBits, char *ExpectedDiskBits)
{
    bit_string Init = ToBitString(InitBits);
    bit_string ExpectedDisk = ToBitString(ExpectedDiskBits);
    bit_string Disk = FillDisk(DiskLength, Init);
    Assert(BitStringsAreEqual(Disk, ExpectedDisk));
}

internal bit_string
FillDiskAndFindChecksum(u32 DiskLength, char *InitBits)
{
    bit_string Init = ToBitString(InitBits);
    bit_string Disk = FillDisk(DiskLength, Init);
    bit_string Checksum = FindChecksum(Disk);
    return(Checksum);
}

int
main(void)
{
    memory_index ArenaSize = Megabytes(512);
    void *ArenaBase = malloc(ArenaSize);
    Assert(ArenaBase);
    InitializeArena(GlobalArena, ArenaSize, ArenaBase);

    DragonCurveTestCase("1", "100");
    DragonCurveTestCase("0", "001");
    DragonCurveTestCase("11111", "11111000000");
    DragonCurveTestCase("111100001010", "1111000010100101011110000");

    ChecksumTestCase("110010110100", "100");
    ChecksumTestCase("10000011110010000111", "01100");

    FillDiskTestCase(20, "10000", "10000011110010000111");

    char *InputBits = "10011111011011001";

    bit_string Checksum1 = FillDiskAndFindChecksum(272, InputBits);
    Assert(BitStringsAreEqual(Checksum1, ToBitString("10111110010110110")));

    bit_string Checksum2 = FillDiskAndFindChecksum(35651584, InputBits);
    Assert(BitStringsAreEqual(Checksum2, ToBitString("01101100001100100")));

    return(0);
}
