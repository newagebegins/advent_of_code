#include "lib.h"
#include "md5.cpp"

#include <stdlib.h>
#include <stdio.h>

#define NIBBLE_COUNT (2*ArrayCount(((md5 *)0)->Bytes))

internal u32
UInt32ToStr(u32 Val, char *Buffer, u32 BufferSize)
{
    char Reverse[64];
    u32 DigitCount;
    if(Val)
    {
        DigitCount = 0;
        for(; Val; Val /= 10)
        {
            Reverse[DigitCount++] = ('0' + (Val % 10));
        }
    }
    else
    {
        DigitCount = 1;
        Reverse[0] = '0';
    }
    Assert(DigitCount > 0);
    Assert(DigitCount <= BufferSize);
    char *Dest = Buffer;
    char *Source = Reverse + DigitCount - 1;
    for(u32 DigitIndex = 0;
        DigitIndex < DigitCount;
        ++DigitIndex)
    {
        *Dest++ = *Source--;
    }
    return(DigitCount);
}

struct test1_result
{
    b32 Success;
    u8 Nibble;
};

inline u8
GetNibble(md5 Hash, u32 NibbleIndex)
{
    u8 Byte = Hash.Bytes[NibbleIndex/2];
    u32 Shift = (((NibbleIndex % 2) == 0) ? 4 : 0);
    u8 Result = ((Byte >> Shift) & 0xF);
    return(Result);
}

inline test1_result
DoTest1(md5 Hash)
{
    test1_result Result = {};
    for(u32 MatchNibbleIndex = 0;
        MatchNibbleIndex < (NIBBLE_COUNT - 2);
        ++MatchNibbleIndex)
    {
        u8 MatchNibble = GetNibble(Hash, MatchNibbleIndex);
        if((GetNibble(Hash, MatchNibbleIndex + 1) == MatchNibble) &&
           (GetNibble(Hash, MatchNibbleIndex + 2) == MatchNibble))
        {
            Result.Success = true;
            Result.Nibble = MatchNibble;
            break;
        }
    }
    return(Result);
}

inline b32
Contains5InRow(md5 Hash, u8 MatchNibble)
{
    b32 Result = false;
    for(u32 NibbleIndex = 0;
        NibbleIndex < (NIBBLE_COUNT - 4);
        ++NibbleIndex)
    {
        if((GetNibble(Hash, NibbleIndex + 0) == MatchNibble) &&
           (GetNibble(Hash, NibbleIndex + 1) == MatchNibble) &&
           (GetNibble(Hash, NibbleIndex + 2) == MatchNibble) &&
           (GetNibble(Hash, NibbleIndex + 3) == MatchNibble) &&
           (GetNibble(Hash, NibbleIndex + 4) == MatchNibble))
        {
            Result = true;
            break;
        }
    }
    return(Result);
}

internal void
MD5ToStr(md5 Hash, char *Buffer)
{
    for(u32 NibbleIndex = 0;
        NibbleIndex < NIBBLE_COUNT;
        ++NibbleIndex)
    {
        u8 Nibble = GetNibble(Hash, NibbleIndex);
        char NibbleChar = HexDigitToChar(Nibble);
        Buffer[NibbleIndex] = NibbleChar;
    }
    Buffer[NIBBLE_COUNT] = 0;
}

struct context
{
    memory_arena *Arena;
    char *Message;
    u32 SaltLength;
    u32 MaxIndexLength;

    md5 *CachedHashes;
    u32 OnePastMaxCachedHashIndex;
};

internal md5
FindHash(context *Context, u32 HashIndex, b32 Stretch)
{
    md5 Result;
    if(HashIndex < Context->OnePastMaxCachedHashIndex)
    {
        Result = Context->CachedHashes[HashIndex];
    }
    else
    {
        u32 IndexLength = UInt32ToStr(HashIndex, Context->Message + Context->SaltLength, Context->MaxIndexLength);
        u32 MessageLength = Context->SaltLength + IndexLength;
        Result = MD5(Context->Arena, (u8 *)Context->Message, MessageLength);
        if(Stretch)
        {
            char HashStr[NIBBLE_COUNT + 1];
            for(u32 Iteration = 0;
                Iteration < 2016;
                ++Iteration)
            {
                MD5ToStr(Result, HashStr);
                Result = MD5(Context->Arena, (u8 *)HashStr, NIBBLE_COUNT);
            }
        }
        Assert(HashIndex == Context->OnePastMaxCachedHashIndex);
        md5 *CachedHash = PushStruct(Context->Arena, md5);
        *CachedHash = Result;
        ++Context->OnePastMaxCachedHashIndex;
    }
    return(Result);
}

inline b32
DoTest2(context *Context, u8 MatchNibble, u32 StartIndex, b32 Stretch)
{
    b32 Result = false;
    u32 OnePastEndIndex = StartIndex + 1000;
    for(u32 Index = StartIndex;
        Index < OnePastEndIndex;
        ++Index)
    {
        md5 Hash = FindHash(Context, Index, Stretch);
        if(Contains5InRow(Hash, MatchNibble))
        {
            Result = true;
            break;
        }
    }
    return(Result);
}

internal b32
IsKey(context *Context, md5 Hash, u32 HashIndex, b32 Stretch)
{
    b32 Result = false;
    test1_result Test1Result = DoTest1(Hash);
    if(Test1Result.Success)
    {
        Result = DoTest2(Context, Test1Result.Nibble, HashIndex + 1, Stretch);
    }
    return(Result);
}

internal u32
FindIndexThatProducesKey64(memory_arena *Arena, char *Salt, b32 Stretch)
{
    u32 Result = 0;

    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    char Message[64];
    context Context = {};
    Context.Arena = Arena;
    Context.Message = Message;
    Context.SaltLength = StringLength(Salt);
    Assert(Context.SaltLength < ArrayCount(Message));
    Copy(Context.SaltLength, Salt, Message);
    Context.MaxIndexLength = ArrayCount(Message) - Context.SaltLength;
    Context.CachedHashes = (md5 *)GetWatermark(Arena);

    u32 KeyCount = 0;

    for(u32 Index = 0;
        ;
        ++Index)
    {
        md5 Hash = FindHash(&Context, Index, Stretch);
        if(IsKey(&Context, Hash, Index, Stretch))
        {
            if(++KeyCount == 64)
            {
                Result = Index;
                break;
            }
        }
    }

    EndTemporaryMemory(TempMem);

    return(Result);
}

int
main(void)
{
    memory_arena Arena;
    memory_index ArenaSize = Megabytes(1);
    void *ArenaBase = malloc(ArenaSize);
    Assert(ArenaBase);
    InitializeArena(&Arena, ArenaSize, ArenaBase);

    TestMD5(&Arena);

#if 1
    char *TestSalt = "abc";

    u32 TestResult = FindIndexThatProducesKey64(&Arena, TestSalt, false);
    Assert(TestResult == 22728);

    u32 TestResult2 = FindIndexThatProducesKey64(&Arena, TestSalt, true);
    Assert(TestResult2 == 22551);
#endif

    char *Salt = "ihaygndm";

    u32 Result = FindIndexThatProducesKey64(&Arena, Salt, false);
    Assert(Result == 15035);

    u32 Result2 = FindIndexThatProducesKey64(&Arena, Salt, true);
    Assert(Result2 == 19968);

    return(0);
}
