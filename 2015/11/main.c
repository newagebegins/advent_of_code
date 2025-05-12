#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void incrementPassword(char* password)
{
    int i = (int)strlen(password) - 1;
    while (true)
    {
        if (password[i] == 'z')
        {
            password[i] = 'a';
            if (i == 0)
            {
                break;
            }
            else
            {
                --i;
            }
        }
        else
        {
            ++password[i];
            break;
        }
    }
}

bool passwordContainsIncreasingStraightOfThreeLetters(char* password)
{
    int len = (int)strlen(password);
    for (int i = 0; i < len - 2; ++i)
    {
        if ((password[i + 1] == (password[i] + 1)) && (password[i + 2] == (password[i] + 2)))
        {
            return true;
        }
    }
    return false;
}

bool passwordContainsIOL(char* password)
{
    char c;
    while ((c = *password++))
    {
        switch (c)
        {
        case 'i':
        case 'o':
        case 'l':
            return true;
        }
    }
    return false;
}

bool passwordContainsTwoDifferentNonOverlappingPairsOfLetters(char* password)
{
    int len = (int)strlen(password);
    char firstPairChar = 0;
    int i = 0;
    for (; i < len - 1; ++i)
    {
        if (password[i] == password[i + 1])
        {
            firstPairChar = password[i];
            i = i + 2;
            break;
        }
    }
    if (firstPairChar)
    {
        for (; i < len - 1; ++i)
        {
            if ((password[i] == password[i + 1]) && (password[i] != firstPairChar))
            {
                return true;
            }
        }
    }
    return false;
}

bool isValidPassword(char* password)
{
    return passwordContainsIncreasingStraightOfThreeLetters(password)
        && !passwordContainsIOL(password)
        && passwordContainsTwoDifferentNonOverlappingPairsOfLetters(password);
}

void toNextPassword(char* password)
{
    do
    {
        incrementPassword(password);
    } while (!isValidPassword(password));
}

int main(void)
{
    assert(isValidPassword("ghjaabcc"));
    char password[] = "vzbxkghb";
    toNextPassword(password);
    printf("%s\n", password);
    toNextPassword(password);
    printf("%s\n", password);
    return 0;
}
