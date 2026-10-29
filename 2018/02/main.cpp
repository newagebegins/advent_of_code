#include "lib.h"

struct parse_result
{
    u32 IDCount;
    str *IDs;
};

internal parse_result
ParseInput(char *Input)
{
    parse_result Result = {};
    Result.IDCount = CountLines(Input);
    Result.IDs = (str *)malloc(Result.IDCount*sizeof(str));

    parser Parser = MakeParser(Input);
    u32 IDIndex = 0;

    while(StillParsing(&Parser))
    {
        Result.IDs[IDIndex++] = ParseWord(&Parser);
        SkipWhitespace(&Parser);
    }

    Assert(IDIndex == Result.IDCount);

    return(Result);
}

struct letter_counts
{
    u8 Counts['z' - 'a' + 1];
};

inline letter_counts
CountLetters(str ID)
{
    letter_counts Result = {};
    for(u32 LetterIndex = 0;
        LetterIndex < ID.Length;
        ++LetterIndex)
    {
        char Letter = ID.Chars[LetterIndex];
        ++Result.Counts[Letter - 'a'];
    }
    return(Result);
}

internal u32
FindChecksum(u32 IDCount, str *IDs)
{
    u32 TwoCount = 0;
    u32 ThreeCount = 0;
    for(u32 IDIndex = 0;
        IDIndex < IDCount;
        ++IDIndex)
    {
        str ID = IDs[IDIndex];
        u32 HasExactlyTwoOfAnyLetter = 0;
        u32 HasExactlyThreeOfAnyLetter = 0;
        letter_counts Counts = CountLetters(ID);
        for(u32 LetterIndex = 0;
            LetterIndex < ArrayCount(Counts.Counts);
            ++LetterIndex)
        {
            u8 Count = Counts.Counts[LetterIndex];
            switch(Count)
            {
                case 2:
                {
                    HasExactlyTwoOfAnyLetter = 1;
                } break;

                case 3:
                {
                    HasExactlyThreeOfAnyLetter = 1;
                } break;
            }
        }
        TwoCount += HasExactlyTwoOfAnyLetter;
        ThreeCount += HasExactlyThreeOfAnyLetter;
    }
    u32 Result = TwoCount * ThreeCount;
    return(Result);
}

internal void
SolvePart1(u32 IDCount, str *IDs)
{
    u32 Checksum = FindChecksum(IDCount, IDs);
    printf("%u\n", Checksum);
}

struct differ_result
{
    b32 Success;
    u32 LetterIndex;
};

internal differ_result
DifferByOneLetter(str A, str B)
{
    differ_result Result = {};
    if(A.Length == B.Length)
    {
        u32 DiffCount = 0;
        for(u32 LetterIndex = 0;
            (LetterIndex < A.Length) && (DiffCount < 2);
            ++LetterIndex)
        {
            if(A.Chars[LetterIndex] != B.Chars[LetterIndex])
            {
                ++DiffCount;
                Result.LetterIndex = LetterIndex;
            }
        }
        Result.Success = (DiffCount == 1);
    }
    return(Result);
}

internal void
SolvePart2(u32 IDCount, str *IDs)
{
    str FoundID = {};
    u32 DiffLetterIndex = 0;
    for(u32 IndexA = 0;
        (IndexA < (IDCount - 1)) && !(FoundID.Length);
        ++IndexA)
    {
        str A = IDs[IndexA];
        for(u32 IndexB = IndexA + 1;
            (IndexB < IDCount) && !(FoundID.Length);
            ++IndexB)
        {
            str B = IDs[IndexB];
            differ_result Differ = DifferByOneLetter(A, B);
            if(Differ.Success)
            {
                FoundID = A;
                DiffLetterIndex = Differ.LetterIndex;
            }
        }
    }
    Assert(FoundID.Length);
    Assert(FoundID.Chars);
    printf("%.*s", DiffLetterIndex, FoundID.Chars);
    Assert(DiffLetterIndex < FoundID.Length);
    printf("%.*s\n", FoundID.Length - (DiffLetterIndex + 1), FoundID.Chars + DiffLetterIndex + 1);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        parse_result ParseResult = ParseInput(Input);
        SolvePart1(ParseResult.IDCount, ParseResult.IDs);
        SolvePart2(ParseResult.IDCount, ParseResult.IDs);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
