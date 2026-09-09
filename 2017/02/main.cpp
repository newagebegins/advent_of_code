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

int
main(void)
{
    char *TestInput = ReadEntireFileAndNullTerminate("test_input.txt");
    u32 TestChecksum = CalculateChecksum(TestInput);
    Assert(TestChecksum == 18);

    char *Input = ReadEntireFileAndNullTerminate("input.txt");
    u32 Checksum = CalculateChecksum(Input);
    Assert(Checksum == 50376);
}
