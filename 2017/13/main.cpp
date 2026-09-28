#include <stdint.h>
#include <stdio.h>

#define internal static
#define local_persist static

#define Assert(C) if(!(C)) { *(int *)0 = 0; }
#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))

typedef uint32_t u32;
typedef int32_t b32;

internal void
ReadEntireFileAndNullTerminate(char *FileName, u32 BufferSize, char *Buffer)
{
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    Assert(FileSize > 0);
    Assert((u32)(FileSize + 1) <= BufferSize);
    fseek(File, 0, SEEK_SET);
    fread(Buffer, FileSize, 1, File);
    Buffer[FileSize] = 0;
}

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') || (C == '\t') || (C == '\r') || (C == '\n'));
    return(Result);
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

struct parser
{
    char *At;
};

inline u32
ParseU32(parser *Parser)
{
    u32 Result = 0;
    while(IsDigit(Parser->At[0]))
    {
        Result = 10*Result + (Parser->At[0] - '0');
        ++Parser->At;
    }
    return(Result);
}

inline void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

inline void
SkipString(parser *Parser, char *String)
{
    for(char *At = String;
        *At;
        ++At, ++Parser->At)
    {
        Assert(Parser->At[0] == *At);
    }
}

struct layer
{
    u32 Depth;
    u32 Range;
};

internal u32
ParseInput(char *Input, u32 MaxLayerCount, layer *Layers)
{
    u32 LayerCount = 0;
    parser Parser;
    Parser.At = Input;
    SkipWhitespace(&Parser);
    while(Parser.At[0])
    {
        Assert(LayerCount < MaxLayerCount);
        layer *Layer = Layers + LayerCount++;
        Layer->Depth = ParseU32(&Parser);
        SkipString(&Parser, ": ");
        Layer->Range = ParseU32(&Parser);
        SkipWhitespace(&Parser);
    }
    return(LayerCount);
}

/*
  NOTE(slava):

  range = 2

   0   1   2   3
  [S] [ ] [S] [ ]
  [ ] [S] [ ] [S]

  range = 3

   0   1   2   3   4   5
  [S] [ ] [ ] [ ] [S] [ ]
  [ ] [S] [ ] [S] [ ] [S]
  [ ] [ ] [S] [ ] [ ] [ ]

  range = 4

   0   1   2   3   4   5   6
  [S] [ ] [ ] [ ] [ ] [ ] [S]
  [ ] [S] [ ] [ ] [ ] [S] [ ]
  [ ] [ ] [S] [ ] [S] [ ] [ ]
  [ ] [ ] [ ] [S] [ ] [ ] [ ]

  period = 2*(range - 1)
*/

internal u32
FindTripSeverity(u32 LayerCount, layer *Layers)
{
    u32 Result = 0;
    for(u32 LayerIndex = 0;
        LayerIndex < LayerCount;
        ++LayerIndex)
    {
        layer Layer = Layers[LayerIndex];
        u32 Period = 2*(Layer.Range - 1);
        if((Layer.Depth % Period) == 0)
        {
            u32 Severity = Layer.Depth * Layer.Range;
            Result += Severity;
        }
    }
    return(Result);
}

internal void
TestFindSeverity(char *FileName, u32 ExpectedSeverity)
{
    local_persist char Input[512];
    ReadEntireFileAndNullTerminate(FileName, sizeof(Input), Input);
    local_persist layer Layers[64];
    u32 LayerCount = ParseInput(Input, ArrayCount(Layers), Layers);
    u32 Severity = FindTripSeverity(LayerCount, Layers);
    Assert(Severity == ExpectedSeverity);
}

int
main(void)
{
    TestFindSeverity("test_input.txt", 24);
    TestFindSeverity("input.txt", 2604);
    return(0);
}
