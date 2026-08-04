#include "lib.h"

#include <stdlib.h>
#include <stdio.h>

#define MAX_BIT_COUNT 300

struct bit_string
{
    char Bits[MAX_BIT_COUNT + 1];
    u32 BitCount;
};

internal bit_string
ToBitString(char *Bits)
{
    bit_string Result;
    char *Source = Bits;
    char *Dest = Result.Bits;
    while(*Source)
    {
        *Dest++ = *Source++;
    }
    *Dest = 0;
    Result.BitCount = (u32)(Source - Bits);
    Assert(Result.BitCount <= MAX_BIT_COUNT);
    return(Result);
}

internal bit_string
DragonCurve(bit_string Input)
{
    bit_string Result;
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
    Result.BitCount = (u32)(Dest - Result.Bits);
    return(Result);
}

internal bit_string
FindChecksum(bit_string Input)
{
    Assert((Input.BitCount % 2) == 0);
    bit_string Result = {};
    char *Source = Input.Bits;
    u32 SourceBitCount = Input.BitCount;
    while((Result.BitCount % 2) == 0)
    {
        char *Dest = Result.Bits;
        for(u32 BitIndex = 0;
            BitIndex < SourceBitCount;
            BitIndex += 2)
        {
            char C1 = Source[BitIndex];
            char C2 = Source[BitIndex + 1];
            *Dest++ = ((C1 == C2) ? '1' : '0');
        }
        *Dest = 0;
        Result.BitCount = (u32)(Dest - Result.Bits);
        Source = Result.Bits;
        SourceBitCount = Result.BitCount;
    }
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

int
main(void)
{
    DragonCurveTestCase("1", "100");
    DragonCurveTestCase("0", "001");
    DragonCurveTestCase("11111", "11111000000");
    DragonCurveTestCase("111100001010", "1111000010100101011110000");

    ChecksumTestCase("110010110100", "100");
    ChecksumTestCase("10000011110010000111", "01100");

    FillDiskTestCase(20, "10000", "10000011110010000111");

    bit_string Init = ToBitString("10011111011011001");
    bit_string Disk = FillDisk(272, Init);
    bit_string Checksum = FindChecksum(Disk);
    printf("%s\n", Checksum.Bits);

    return(0);
}
