#include "lib.h"

inline b32
IsSpaceOrTab(char C)
{
    b32 Result = ((C == ' ') || (C == '\t'));
    return(Result);
}

inline b32
IsNewLine(char C)
{
    b32 Result = ((C == '\r') || (C == '\n'));
    return(Result);
}

inline b32
IsWhitespace(char C)
{
    b32 Result = (IsSpaceOrTab(C) || IsNewLine(C));
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

internal parser
CreateParser(char *Str)
{
    parser Parser = {};
    Parser.At = Str;
    return(Parser);
}

internal b32
ParsingSpreadsheet(parser *Parser)
{
    b32 Result = (Parser->At[0] != 0);
    return(Result);
}

internal b32
ParsingRow(parser *Parser)
{
    char C = Parser->At[0];
    b32 Result = (C && !IsNewLine(C));
    return(Result);
}

internal void
SkipAllWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

internal void
SkipSpacesAndTabs(parser *Parser)
{
    while(IsSpaceOrTab(Parser->At[0]))
    {
        ++Parser->At;
    }
}

internal u32
GetU32(parser *Parser)
{
    u32 Result = 0;
    for(;
        IsDigit(Parser->At[0]);
        ++Parser->At)
    {
        Result = 10*Result + (Parser->At[0] - '0');
    }
    return(Result);
}

internal u32
CalculateChecksum(char *SpreadsheetText)
{
    u32 Checksum = 0;
    parser Parser = CreateParser(SpreadsheetText);
    while(ParsingSpreadsheet(&Parser))
    {
        u32 RowMin = UINT_MAX;
        u32 RowMax = 0;
        SkipAllWhitespace(&Parser);
        while(ParsingRow(&Parser))
        {
            u32 Value = GetU32(&Parser);
            if(RowMax < Value)
            {
                RowMax = Value;
            }
            if(Value < RowMin)
            {
                RowMin = Value;
            }
            SkipSpacesAndTabs(&Parser);
        }
        if(RowMin <= RowMax)
        {
            u32 RowDiff = RowMax - RowMin;
            Checksum += RowDiff;
        }
    }
    return(Checksum);
}

internal u32
CalculateChecksum2(char *SpreadsheetText)
{
    u32 Checksum = 0;
    u32 Row[64];
    parser Parser = CreateParser(SpreadsheetText);

    while(ParsingSpreadsheet(&Parser))
    {
        u32 RowSize = 0;
        SkipAllWhitespace(&Parser);
        while(ParsingRow(&Parser))
        {
            u32 Value = GetU32(&Parser);
            Assert(RowSize < ArrayCount(Row));
            Row[RowSize++] = Value;
            SkipSpacesAndTabs(&Parser);
        }
        if(RowSize)
        {
            b32 Found = false;
            for(u32 I0 = 0;
                (I0 < RowSize) && !Found;
                ++I0)
            {
                u32 N0 = Row[I0];
                for(u32 I1 = I0 + 1;
                    I1 < RowSize;
                    ++I1)
                {
                    u32 N1 = Row[I1];
                    Assert(N0 != N1);
                    u32 Bigger = ((N0 < N1) ? N1 : N0);
                    u32 Smaller = ((N0 < N1) ? N0 : N1);
                    if((Bigger % Smaller) == 0)
                    {
                        Checksum += (Bigger / Smaller);
                        Found = true;
                        break;
                    }
                }
            }
            Assert(Found);
        }
    }
    return(Checksum);
}

int
main(void)
{
    // NOTE(slava): Part 1

    char *TestInput = ReadEntireFileAndNullTerminate("test_input.txt");
    u32 TestChecksum = CalculateChecksum(TestInput);
    Assert(TestChecksum == 18);

    char *Input = ReadEntireFileAndNullTerminate("input.txt");

    u32 Checksum = CalculateChecksum(Input);
    Assert(Checksum == 50376);

    // NOTE(slava): Part 2

    char *TestInput2 = ReadEntireFileAndNullTerminate("test_input2.txt");
    u32 TestChecksum2 = CalculateChecksum2(TestInput2);
    Assert(TestChecksum2 == 9);

    u32 Checksum2 = CalculateChecksum2(Input);
    Assert(Checksum2 == 267);
}
