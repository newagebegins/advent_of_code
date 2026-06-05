#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define Assert(Cond) if(!(Cond)) { *(int *)0 = 0; }

#define internal static

typedef uint32_t b32;
typedef int32_t s32;
typedef uint32_t u32;

struct entire_file
{
    void *Contents;
    u32 ContentsSize;
};

internal entire_file
ReadEntireFile(char *FileName)
{
    entire_file Result = {};
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    Result.ContentsSize = ftell(File);
    Assert(Result.ContentsSize > 0);
    fseek(File, 0, SEEK_SET);
    Result.Contents = malloc(Result.ContentsSize);
    Assert(Result.Contents);
    size_t ReadCount = fread(Result.Contents, Result.ContentsSize, 1, File);
    Assert(ReadCount == 1);
    fclose(File);
    return(Result);
}

internal b32
SupportsTLS(char *String, u32 IPBeginIndex, u32 IPOnePastEndIndex)
{
    Assert(String);

    b32 Result = false;
    b32 WithinHypernet = false;

    for(u32 CharIndex = IPBeginIndex + 1;
        CharIndex < IPOnePastEndIndex;
        ++CharIndex)
    {
        if(String[CharIndex] == '[')
        {
            Assert(!WithinHypernet);
            WithinHypernet = true;
        }
        else if(String[CharIndex] == ']')
        {
            Assert(WithinHypernet);
            WithinHypernet = false;
        }
        else if(String[CharIndex] != String[CharIndex - 1])
        {
            if(((CharIndex + 2) < IPOnePastEndIndex) &&
               (String[CharIndex + 1] == String[CharIndex]) &&
               (String[CharIndex + 2] == String[CharIndex - 1]))
            {
                if(WithinHypernet)
                {
                    Result = false;
                    break;
                }
                else
                {
                    Result = true;
                    CharIndex += 2;
                }
            }
        }
    }

    return(Result);
}

struct xyx
{
    char X;
    char Y;
};

internal b32
SupportsSSL(char *String, u32 IPBeginIndex, u32 IPOnePastEndIndex)
{
    Assert(String);

#define MAX_XYX_COUNT 32
    xyx ABAs[MAX_XYX_COUNT];
    u32 ABACount = 0;

    xyx BABs[MAX_XYX_COUNT];
    u32 BABCount = 0;

    b32 WithinHypernet = false;

    for(u32 CharIndex = IPBeginIndex;
        CharIndex < (IPOnePastEndIndex - 2);
        ++CharIndex)
    {
        if(String[CharIndex] == '[')
        {
            Assert(!WithinHypernet);
            WithinHypernet = true;
        }
        else if(String[CharIndex] == ']')
        {
            Assert(WithinHypernet);
            WithinHypernet = false;
        }
        else if((String[CharIndex] != String[CharIndex + 1]) &&
                (String[CharIndex] == String[CharIndex + 2]))
        {
            if(WithinHypernet)
            {
                Assert(BABCount < MAX_XYX_COUNT);
                BABs[BABCount++] = {String[CharIndex], String[CharIndex + 1]};
            }
            else
            {
                Assert(ABACount < MAX_XYX_COUNT);
                ABAs[ABACount++] = {String[CharIndex], String[CharIndex + 1]};
            }
        }
    }

    b32 Result = false;

    for(u32 ABAIndex = 0;
        ABAIndex < ABACount;
        ++ABAIndex)
    {
        for(u32 BABIndex = 0;
            BABIndex < BABCount;
            ++BABIndex)
        {
            xyx ABA = ABAs[ABAIndex];
            xyx BAB = BABs[BABIndex];
            if((ABA.X == BAB.Y) && (ABA.Y == BAB.X))
            {
                Result = true;
                break;
            }
        }
    }

    return(Result);
}

internal u32
StringLength(char *String)
{
    char *Begin = String;
    while(*String)
    {
        ++String;
    }
    u32 Result = (u32)(String - Begin);
    return(Result);
}

internal b32
SupportsTLS(char *IP)
{
    b32 Result = SupportsTLS(IP, 0, StringLength(IP));
    return(Result);
}

internal b32
SupportsSSL(char *IP)
{
    b32 Result = SupportsSSL(IP, 0, StringLength(IP));
    return(Result);
}

internal u32
GetTLSCount(entire_file File)
{
    u32 Result = 0;
    u32 IPBeginIndex = 0;

    for(u32 Index = 1;
        Index < File.ContentsSize;
        ++Index)
    {
        if(((char *)File.Contents)[Index] == '\n')
        {
            u32 IPOnePastEndIndex = Index;
            if(SupportsTLS((char *)File.Contents, IPBeginIndex, IPOnePastEndIndex))
            {
                ++Result;
            }
            IPBeginIndex = Index + 1;
        }
    }
    if(SupportsTLS((char *)File.Contents, IPBeginIndex, File.ContentsSize))
    {
        ++Result;
    }
    return(Result);
}

internal u32
GetSSLCount(entire_file File)
{
    u32 Result = 0;
    u32 IPBeginIndex = 0;

    for(u32 Index = 1;
        Index < File.ContentsSize;
        ++Index)
    {
        if(((char *)File.Contents)[Index] == '\n')
        {
            u32 IPOnePastEndIndex = Index;
            if(SupportsSSL((char *)File.Contents, IPBeginIndex, IPOnePastEndIndex))
            {
                ++Result;
            }
            IPBeginIndex = Index + 1;
        }
    }
    if(SupportsSSL((char *)File.Contents, IPBeginIndex, File.ContentsSize))
    {
        ++Result;
    }
    return(Result);
}

int
main(void)
{
    Assert(SupportsTLS("abba[mnop]qrst"));
    Assert(!SupportsTLS("abcd[bddb]xyyx"));
    Assert(!SupportsTLS("aaaa[qwer]tyui"));
    Assert(SupportsTLS("ioxxoj[asdfgh]zxcvbn"));

    Assert(SupportsSSL("aba[bab]xyz"));
    Assert(!SupportsSSL("xyx[xyx]xyx"));
    Assert(SupportsSSL("aaa[kek]eke"));
    Assert(SupportsSSL("zazbz[bzb]cdb"));

    entire_file File = ReadEntireFile("input.txt");
    Assert(GetTLSCount(File) == 115);

    printf("%u\n", GetSSLCount(File));

    return(0);
}
