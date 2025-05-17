#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

constexpr int maxPersonCount = 16;
constexpr int maxNameSize = 16;
constexpr int excludedSize = maxPersonCount - 1;

static char* readEntireFile(const char* path)
{
    char* result = NULL;
    FILE* file;
    fopen_s(&file, path, "r");
    if (file)
    {
        if (fseek(file, 0, SEEK_END) == 0)
        {
            long fileSize = ftell(file);
            if (fileSize > 0)
            {
                rewind(file);
                char* buffer = (char*)malloc(fileSize + 1);
                if (buffer)
                {
                    if (fread(buffer, 1, fileSize, file) == fileSize)
                    {
                        buffer[fileSize] = 0;
                        result = buffer;
                    }
                    else
                    {
                        assert(0);
                    }
                }
                else
                {
                    assert(0);
                }
            }
            else
            {
                assert(0);
            }
        }
        else
        {
            assert(0);
        }
        fclose(file);
    }
    else
    {
        assert(0);
    }
    return result;
}

static constexpr int factorial(int n)
{
    int result = 1;
    for (int i = 2; i <= n; ++i)
    {
        result *= i;
    }
    return result;
}

static constexpr int nCk(int n, int k)
{
    return factorial(n) / (factorial(k) * factorial(n - k));
}

static constexpr int getSeatingArrangementsCount(int personCount)
{
    return nCk(personCount - 1, 2) * factorial(personCount - 3);
}

static void findSeatingArrangements_(int personCount, int* excluded, int excludedCount, int** arrangements)
{
    for (int person = 1; person < personCount; ++person)
    {
        bool skip = false;
        for (int excludedIndex = 0; excludedIndex < excludedCount; ++excludedIndex)
        {
            if (person == excluded[excludedIndex])
            {
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            if (excludedCount == personCount - 2)
            {
                *(*arrangements)++ = excluded[0];
                *(*arrangements)++ = 0;
                for (int i = 1; i < excludedCount; ++i)
                {
                    *(*arrangements)++ = excluded[i];
                }
                *(*arrangements)++ = person;
            }
            else
            {
                assert(excludedCount < excludedSize);
                excluded[excludedCount] = person;
                findSeatingArrangements_(personCount, excluded, excludedCount + 1, arrangements);
            }
        }
    }
}

struct SeatingArrangements
{
    int* arrangements;
    int arrangementsCount;
};

static SeatingArrangements findSeatingArrangements(int personCount)
{
    int excluded[excludedSize];
    int arrangementsCount = getSeatingArrangementsCount(personCount);
    int entriesCount = arrangementsCount * personCount;
    int* arrangements = (int*)malloc(entriesCount * sizeof(arrangements[0]));
    assert(arrangements);
    int* p = arrangements;
    
    for (int i = 1; i < personCount; ++i)
    {
        excluded[0] = i;
        for (int j = i + 1; j < personCount; ++j)
        {
            excluded[1] = j;
            findSeatingArrangements_(personCount, excluded, 2, &p);
        }
    }

    assert(p == arrangements + entriesCount);
    return { arrangements, arrangementsCount };
}

static void skipWhitespace(const char** input)
{
    bool finish = false;
    while (!finish)
    {
        switch (**input)
        {
        case ' ':
        case '\t':
        case '\n':
            ++(*input);
            break;
        default:
            finish = true;
            break;
        }
    }
}

static bool stringsAreEqual(const char* s1, const char* s2)
{
    while (true)
    {
        if (*s1 == *s2)
        {
            if (*s1 == 0)
            {
                return true;
            }
            else
            {
                ++s1;
                ++s2;
            }
        }
        else
        {
            return false;
        }
    }

    assert(0);
    return false;
}

static int extractName(const char** input, char* names)
{
    skipWhitespace(input);

    char name[maxNameSize]{};
    int nameLen = 0;
    while (**input != ' ' && **input != '.')
    {
        name[nameLen++] = *(*input)++;
    }

    for (int personIndex = 0; personIndex < maxPersonCount; ++personIndex)
    {
        char* existingName = &names[maxNameSize * personIndex];
        if (stringsAreEqual(name, existingName))
        {
            return personIndex;
        }
    }

    for (int personIndex = 0; personIndex < maxPersonCount; ++personIndex)
    {
        char* existingName = &names[maxNameSize * personIndex];
        if (!*existingName)
        {
            for (int i = 0; i < nameLen; ++i)
            {
                existingName[i] = name[i];
            }
            return personIndex;
        }
    }

    assert(0);
    return -1;
}

static void skipString(const char** input, const char* str)
{
    skipWhitespace(input);

    for (; *str; ++str, ++(*input))
    {
        assert(**input == *str);
    }
}

static void extractWord(const char** input, char* buf, int bufSize)
{
    skipWhitespace(input);
    int wordLen = 0;
    while (**input != ' ')
    {
        buf[wordLen++] = *(*input)++;
    }
    assert(wordLen < bufSize);
    buf[wordLen] = 0;
}

static int extractSign(const char** input)
{
    char word[16];
    extractWord(input, word, sizeof(word));
    if (stringsAreEqual(word, "gain"))
    {
        return 1;
    }
    if (stringsAreEqual(word, "lose"))
    {
        return -1;
    }
    assert(0);
    return 0;
}

static bool isDigit(char c)
{
    switch (c)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return true;
    default:
        return false;
    }
}

static int extractPositiveInteger(const char** input)
{
    skipWhitespace(input);
    int result = 0;
    while (isDigit(**input))
    {
        result = result * 10 + (**input - '0');
        ++(*input);
    }
    return result;
}

struct ParseResult
{
    int personCount;
    int* happiness;
};

static ParseResult parseInput(const char* input)
{
    char names[maxPersonCount * maxNameSize]{};
    int* happinessTable = (int*)calloc(maxPersonCount * maxPersonCount, sizeof(int));
    assert(happinessTable);

    while (*input)
    {
        int personIndex = extractName(&input, names);
        skipString(&input, "would");
        int sign = extractSign(&input);
        int happinessValue = extractPositiveInteger(&input);
        skipString(&input, "happiness units by sitting next to");
        int neighborIndex = extractName(&input, names);
        happinessTable[personIndex * maxPersonCount + neighborIndex] = sign * happinessValue;
        skipString(&input, ".");
        skipWhitespace(&input);
    }

    int personCount = 0;
    for (char* p = names; *p; p += maxNameSize, ++personCount)
    {
    }

    return { personCount, happinessTable };
}

int findMaxHappiness(int personCount, const int* happiness)
{
    SeatingArrangements seatingArrangements = findSeatingArrangements(personCount);
    int happinessSumMax = INT_MIN;
    int entryIndex = 0;

    for (int arrangementIndex = 0; arrangementIndex < seatingArrangements.arrangementsCount; ++arrangementIndex)
    {
        int happinessSum = 0;

        for (int person = 0; person < personCount; ++person)
        {
            int leftNeighbor = (person + personCount - 1) % personCount;
            int rightNeighbor = (person + 1) % personCount;
            happinessSum += happiness[seatingArrangements.arrangements[entryIndex + person] * maxPersonCount + seatingArrangements.arrangements[entryIndex + leftNeighbor]];
            happinessSum += happiness[seatingArrangements.arrangements[entryIndex + person] * maxPersonCount + seatingArrangements.arrangements[entryIndex + rightNeighbor]];
        }

        entryIndex += personCount;

        if (happinessSum > happinessSumMax)
        {
            happinessSumMax = happinessSum;
        }
    }

    return happinessSumMax;
}

int main()
{
    const char* exampleInput = R"(Alice would gain 54 happiness units by sitting next to Bob.
Alice would lose 79 happiness units by sitting next to Carol.
Alice would lose 2 happiness units by sitting next to David.
Bob would gain 83 happiness units by sitting next to Alice.
Bob would lose 7 happiness units by sitting next to Carol.
Bob would lose 63 happiness units by sitting next to David.
Carol would lose 62 happiness units by sitting next to Alice.
Carol would gain 60 happiness units by sitting next to Bob.
Carol would gain 55 happiness units by sitting next to David.
David would gain 46 happiness units by sitting next to Alice.
David would lose 7 happiness units by sitting next to Bob.
David would gain 41 happiness units by sitting next to Carol.
)";

    ParseResult exampleRarseResult = parseInput(exampleInput);
    printf("%d\n", findMaxHappiness(exampleRarseResult.personCount, exampleRarseResult.happiness));

    char* input = readEntireFile("input.txt");
    ParseResult parseResult = parseInput(input);

    int answer1 = findMaxHappiness(parseResult.personCount, parseResult.happiness);
    assert(answer1 == 709);
    printf("%d\n", answer1);

    int answer2 = findMaxHappiness(parseResult.personCount + 1, parseResult.happiness);
    assert(answer2 == 668);
    printf("%d\n", answer2);

    return 0;
}
