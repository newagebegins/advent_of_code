#include "lib.h"

internal u32
FindSumOfDigitsThatMatchTheNextDigit(char *Digits)
{
    u32 Result = 0;
    if(Digits[0] && Digits[1])
    {
        char *At;
        for(At = Digits;
            At[0];
            ++At)
        {
            if(At[0] == At[1])
            {
                Result += (At[0] - '0');
            }
        }
        if(At[-1] == Digits[0])
        {
            Result += (Digits[0] - '0');
        }
    }
    return(Result);
}

internal void
TestFindSumOfDigitsThatMatchTheNextDigit(char *Input, u32 Expected)
{
    u32 Result = FindSumOfDigitsThatMatchTheNextDigit(Input);
    Assert(Result == Expected);
}

internal void
PreprocessInput(char *Input)
{
    Assert((*Input >= '0') && (*Input <= '9'));
    for(char *At = Input;
        *At;
        ++At)
    {
        if((*At == '\n') || (*At == '\r'))
        {
            *At = 0;
            break;
        }
    }
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
    PreprocessInput(Input);
    TestFindSumOfDigitsThatMatchTheNextDigit(Input, 1047);
}
