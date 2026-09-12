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

#define IS_VALID_PASSPHRASE(name) b32 name(str Passphrase)
typedef IS_VALID_PASSPHRASE(is_valid_passphrase);

internal
IS_VALID_PASSPHRASE(IsValidPassphrase)
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
CountLetters(str Word, u8 *Counts, u32 CountsCapacity)
{
    for(u32 CharIndex = 0;
        CharIndex < Word.Length;
        ++CharIndex)
    {
        char C = Word.Chars[CharIndex];
        Assert((C >= 'a') && (C <= 'z'));
        Assert((u32)C < CountsCapacity);
        ++Counts[C];
    }
}

internal b32
AreAnagrams(str A, str B)
{
    b32 Result = false;
    if(A.Length == B.Length)
    {
        Result = true;
        u8 LetterCountA['z' + 1] = {};
        u8 LetterCountB['z' + 1] = {};
        CountLetters(A, LetterCountA, ArrayCount(LetterCountA));
        CountLetters(B, LetterCountB, ArrayCount(LetterCountB));
        for(char Letter = 'a';
            Letter <= 'z';
            ++Letter)
        {
            if(LetterCountA[Letter] != LetterCountB[Letter])
            {
                Result = false;
                break;
            }
        }
    }
    return(Result);
}

internal
IS_VALID_PASSPHRASE(IsValidPassphrase2)
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
            if(AreAnagrams(Words[AIndex], Words[BIndex]))
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

internal void
TestIsValidPassphrase2(char *Passphrase, b32 Expected)
{
    str PassphraseStr = Str(Passphrase);
    b32 IsValid = IsValidPassphrase2(PassphraseStr);
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
CountValidPassphrases(char *Input, is_valid_passphrase IsValid)
{
    u32 Result = 0;
    parser Parser = CreateParser(Input);
    SkipWhitespace(&Parser);
    while(Parsing(&Parser))
    {
        str Passphrase = GetPassphrase(&Parser);
        if(IsValid(Passphrase))
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

    // NOTE(slava): Part 1

    TestIsValidPassphrase("aa bb cc dd ee", true);
    TestIsValidPassphrase("aa bb cc dd aa", false);
    TestIsValidPassphrase("aa bb cc dd aaa", true);

    u32 Count = CountValidPassphrases(PuzzleInput, IsValidPassphrase);
    Assert(Count == 386);

    // NOTE(slava): Part 2

    TestIsValidPassphrase2("abcde fghij", true);
    TestIsValidPassphrase2("abcde xyz ecdab", false);
    TestIsValidPassphrase2("a ab abc abd abf abj", true);
    TestIsValidPassphrase2("iiii oiii ooii oooi oooo", true);
    TestIsValidPassphrase2("oiii ioii iioi iiio", false);

    u32 Count2 = CountValidPassphrases(PuzzleInput, IsValidPassphrase2);
    Assert(Count2 == 208);
}
