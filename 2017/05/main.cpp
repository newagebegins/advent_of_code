#include <stdio.h>
#include <assert.h>

#define ArrayCount(A) (sizeof(A)/sizeof(A[0]))

static void
ReadEntireFileAndNullTerminate(char *FileName, char *Buffer, int BufferSize)
{
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        long FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);
        assert(FileSize < BufferSize);
        fread(Buffer, FileSize, 1, File);
        Buffer[FileSize] = 0;
        fclose(File);
    }
}

inline bool
IsWhitespace(char C)
{
    bool Result = ((C == ' ') ||
                   (C == '\t') ||
                   (C == '\r') ||
                   (C == '\n'));
    return(Result);
}

inline bool
IsDigit(char C)
{
    bool Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

struct parser
{
    char *At;
};

static void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

static int
ParseInt(parser *Parser)
{
    int Result = 0;
    int Sign = 1;

    if(Parser->At[0] == '-')
    {
        Sign = -1;
        ++Parser->At;
    }

    while(IsDigit(Parser->At[0]))
    {
        Result = 10*Result + (Parser->At[0] - '0');
        ++Parser->At;
    }

    Result *= Sign;
    return(Result);
}

static int
ParseOffsets(char *Input, int *Offsets, int MaxOffsetCount)
{
    int OffsetCount = 0;

    parser Parser;
    Parser.At = Input;

    SkipWhitespace(&Parser);

    while(Parser.At[0])
    {
        int Offset = ParseInt(&Parser);
        assert(OffsetCount < MaxOffsetCount);
        Offsets[OffsetCount++] = Offset;

        SkipWhitespace(&Parser);
    }

    return(OffsetCount);
}

static int
CountStepsToReachExit(int *Offsets, int OffsetCount)
{
    int Result = 0;
    int OffsetIndex = 0;
    while((0 <= OffsetIndex) && (OffsetIndex < OffsetCount))
    {
        int Offset = Offsets[OffsetIndex];
        ++Offsets[OffsetIndex];
        OffsetIndex += Offset;
        ++Result;
    }
    return(Result);
}

int
main(void)
{
    static char PuzzleInput[8192];
    static int Offsets[2048];

    {
        ReadEntireFileAndNullTerminate("test_input.txt", PuzzleInput, ArrayCount(PuzzleInput));
        int OffsetCount = ParseOffsets(PuzzleInput, Offsets, ArrayCount(Offsets));
        int Steps = CountStepsToReachExit(Offsets, OffsetCount);
        assert(Steps == 5);
    }

    {
        ReadEntireFileAndNullTerminate("input.txt", PuzzleInput, ArrayCount(PuzzleInput));
        int OffsetCount = ParseOffsets(PuzzleInput, Offsets, ArrayCount(Offsets));
        int Steps = CountStepsToReachExit(Offsets, OffsetCount);
        assert(Steps == 339351);
    }
}
