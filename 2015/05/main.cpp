#include <cassert>
#include <fstream>
#include <string>
#include <iostream>

static int countVowels(const char* str)
{
    int result = 0;
    for (const char* p = str; *p; ++p)
    {
        switch (*p)
        {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
            ++result;
            break;
        }
    }
    return result;
}

static bool containsAtLeast3Vowels(const char* str)
{
    return countVowels(str) >= 3;
}

static bool containsDoubleLetter(const char* str)
{
    char prevLetter = 0;
    for (const char* p = str; *p; ++p)
    {
        if (*p == prevLetter)
        {
            return true;
        }
        prevLetter = *p;
    }
    return false;
}

static bool containsBadStrings(const char* str)
{
    char prevLetter = 0;
    bool found = false;
    for (const char* p = str; *p && !found; ++p)
    {
        switch (*p)
        {
        case 'b':
            found = (prevLetter == 'a');
            break;
        case 'd':
            found = (prevLetter == 'c');
            break;
        case 'q':
            found = (prevLetter == 'p');
            break;
        case 'y':
            found = (prevLetter == 'x');
            break;
        }
        prevLetter = *p;
    }
    return found;
}

static bool isNiceString(const char* str)
{
    return containsAtLeast3Vowels(str) && containsDoubleLetter(str) && !containsBadStrings(str);
}

int countLetters(const char* str)
{
    int result = 0;
    for (; *str; ++str)
    {
        ++result;
    }
    return result;
}

static bool containsPairOf2Letters(const char* str)
{
    int length = countLetters(str);
    for (int i = 0; i < length-1; ++i)
    {
        char letter1 = str[i];
        char letter2 = str[i+1];
        for (int j = i+2; j < length-1; ++j)
        {
            if (str[j] == letter1 && str[j+1] == letter2)
            {
                return true;
            }
        }
    }
    return false;
}

static bool containsLettersWhichRepeatWithOneLetterBetweenThem(const char* str)
{
    int length = countLetters(str);
    for (int i = 0; i < length-2; ++i)
    {
        if (str[i] == str[i + 2])
        {
            return true;
        }
    }
    return false;
}

static bool isNiceStringV2(const char* str)
{
    return containsPairOf2Letters(str) && containsLettersWhichRepeatWithOneLetterBetweenThem(str);
}

int main()
{
    assert(isNiceString("ugknbfddgicrmopn") == true);
    assert(isNiceString("aaa") == true);
    assert(isNiceString("jchzalrnumimnmhp") == false);
    assert(isNiceString("haegwjzuvuyypxyu") == false);
    assert(isNiceString("dvszwmarrgswjxmb") == false);

    assert(containsPairOf2Letters("xyxy") == true);
    assert(containsPairOf2Letters("aabcdefgaa") == true);
    assert(containsPairOf2Letters("aaa") == false);

    assert(containsLettersWhichRepeatWithOneLetterBetweenThem("xyx") == true);
    assert(containsLettersWhichRepeatWithOneLetterBetweenThem("abcdefeghi") == true);
    assert(containsLettersWhichRepeatWithOneLetterBetweenThem("aaa") == true);
    assert(containsLettersWhichRepeatWithOneLetterBetweenThem("xxb") == false);

    assert(isNiceStringV2("qjhvhtzxzqqjkmpb") == true);
    assert(isNiceStringV2("xxyxx") == true);
    assert(isNiceStringV2("uurcxstgmygtbstg") == false);
    assert(isNiceStringV2("ieodomkazucvgmuy") == false);

    int niceStringsCount1{ 0 };
    int niceStringsCount2{ 0 };
    std::ifstream inputFileStream{ "input.txt" };
    std::string line;
    while (std::getline(inputFileStream, line))
    {
        if (isNiceString(line.c_str()))
        {
            ++niceStringsCount1;
        }
        if (isNiceStringV2(line.c_str()))
        {
            ++niceStringsCount2;
        }
    }
    std::cout << niceStringsCount1 << '\n';
    std::cout << niceStringsCount2 << '\n';

    return 0;
}
