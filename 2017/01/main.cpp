#include "lib.h"

struct digit_list
{
    u8 *Digits;
    u32 Count;
};

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

internal digit_list
CreateDigitList(char *String)
{
    digit_list Result = {};
    char *At = String;
    while(*At && !IsDigit(*At))
    {
        ++At;
    }
    char *StartOfDigits = At;
    while(IsDigit(*At))
    {
        ++Result.Count;
        ++At;
    }
    if(Result.Count)
    {
        Result.Digits = (u8 *)malloc(Result.Count * sizeof(Result.Digits[0]));
        if(Result.Digits)
        {
            for(u32 DigitIndex = 0;
                DigitIndex < Result.Count;
                ++DigitIndex)
            {
                Result.Digits[DigitIndex] = (StartOfDigits[DigitIndex] - '0');
            }
        }
    }
    return(Result);
}

inline void
FreeDigitList(digit_list *List)
{
    free(List->Digits);
    *List = {};
}

internal u32
FindSumOfDigitsThatMatchTheNextDigit(digit_list List)
{
    u32 Result = 0;
    if(List.Count > 1)
    {
        for(u32 DigitIndex = 0;
            (DigitIndex + 1) < List.Count;
            ++DigitIndex)
        {
            if(List.Digits[DigitIndex] == List.Digits[DigitIndex + 1])
            {
                Result += List.Digits[DigitIndex];
            }
        }
        if(List.Digits[0] == List.Digits[List.Count - 1])
        {
            Result += List.Digits[0];
        }
    }
    return(Result);
}

internal void
TestFindSumOfDigitsThatMatchTheNextDigit(char *Input, u32 Expected)
{
    digit_list List = CreateDigitList(Input);
    u32 Result = FindSumOfDigitsThatMatchTheNextDigit(List);
    Assert(Result == Expected);
    FreeDigitList(&List);
}

int
main(void)
{
    TestFindSumOfDigitsThatMatchTheNextDigit("1122", 3);
    TestFindSumOfDigitsThatMatchTheNextDigit("1111", 4);
    TestFindSumOfDigitsThatMatchTheNextDigit("1234", 0);
    TestFindSumOfDigitsThatMatchTheNextDigit("91212129", 9);
    TestFindSumOfDigitsThatMatchTheNextDigit("1", 0);

    char *Input = ReadEntireFileAndNullTerminate("input.txt");
    TestFindSumOfDigitsThatMatchTheNextDigit(Input, 1047);
}
