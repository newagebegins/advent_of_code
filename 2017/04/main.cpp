#include "lib.h"

inline b32
IsNewLine(char C)
{
    b32 Result = ((C == '\n') ||
                  (C == '\r'));
    return(Result);
}

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') ||
                  (C == '\t') ||
                  IsNewLine(C));
    return(Result);
}

internal u32
StringLength(char *String)
{
    char *At = String;
    while(*At)
    {
        ++At;
    }
    u32 Result = (u32)(At - String);
    return(Result);
}

struct str
{
    char *Chars;
    u32 Length;
};

inline str
Str(char *String)
{
    str Result;
    Result.Chars = String;
    Result.Length = StringLength(String);
    return(Result);
}

internal b32
StringsAreEqual(str A, str B)
{
    b32 Result = false;
    if(A.Length == B.Length)
    {
        Result = true;
        for(u32 CharIndex = 0;
            CharIndex < A.Length;
            ++CharIndex)
        {
            if(A.Chars[CharIndex] != B.Chars[CharIndex])
            {
                Result = false;
                break;
            }
        }
    }
    return(Result);
}

internal u32
GetWords(str Sentence, str *Words, u32 MaxWordCount)
{
    u32 WordCount = 0;
    for(u32 CharIndex = 0;
        CharIndex < Sentence.Length;
        ++CharIndex)
    {
        if(Sentence.Chars[CharIndex] != ' ')
        {
            u32 StartIndex = CharIndex;
            ++CharIndex;
            while((Sentence.Chars[CharIndex] != ' ') && (CharIndex < Sentence.Length))
            {
                ++CharIndex;
            }
            Assert(WordCount < MaxWordCount);
            Words[WordCount].Chars = Sentence.Chars + StartIndex;
            Words[WordCount].Length = CharIndex - StartIndex;
            ++WordCount;
        }
    }
    return(WordCount);
}

internal b32
IsValidPassphrase(str Passphrase)
{
    b32 Result = true;
    str Words[16];
    u32 WordCount = GetWords(Passphrase, Words, ArrayCount(Words));
    for(u32 AIndex = 0;
        (AIndex < WordCount) && Result;
        ++AIndex)
    {
        for(u32 BIndex = AIndex + 1;
            (BIndex < WordCount) && Result;
            ++BIndex)
        {
            if(StringsAreEqual(Words[AIndex], Words[BIndex]))
            {
                Result = false;
            }
        }
    }
    return(Result);
}

internal void
TestIsValidPassphrase(char *Passphrase, b32 Expected)
{
    str PassphraseStr = Str(Passphrase);
    b32 IsValid = IsValidPassphrase(PassphraseStr);
    Assert(IsValid == Expected);
}

struct parser
{
    char *At;
};

inline parser
CreateParser(char *Input)
{
    parser Parser;
    Parser.At = Input;
    return(Parser);
}

inline b32
Parsing(parser *Parser)
{
    b32 Result = (Parser->At[0] != 0);
    return(Result);
}

internal void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

internal str
GetPassphrase(parser *Parser)
{
    str Result = {};
    Assert(Parser->At[0]);
    Result.Chars = Parser->At;
    ++Parser->At;
    while(Parser->At[0] && !IsNewLine(Parser->At[0]))
    {
        ++Parser->At;
    }
    Result.Length = (u32)(Parser->At - Result.Chars);
    return(Result);
}

internal u32
CountValidPassphrases(char *Input)
{
    u32 Result = 0;
    parser Parser = CreateParser(Input);
    SkipWhitespace(&Parser);
    while(Parsing(&Parser))
    {
        str Passphrase = GetPassphrase(&Parser);
        if(IsValidPassphrase(Passphrase))
        {
            ++Result;
        }
        SkipWhitespace(&Parser);
    }
    return(Result);
}

int
main(void)
{
    char *PuzzleInput = ReadEntireFileAndNullTerminate("input.txt");

    TestIsValidPassphrase("aa bb cc dd ee", true);
    TestIsValidPassphrase("aa bb cc dd aa", false);
    TestIsValidPassphrase("aa bb cc dd aaa", true);

    u32 Count = CountValidPassphrases(PuzzleInput);
    Assert(Count == 386);
}
