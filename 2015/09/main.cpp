#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof((a)[0]))

constexpr int maxLocations = 16;
constexpr int maxLocationNameSize = 16;

struct LocationDistances
{
    int table[maxLocations][maxLocations];
};

struct LocationName
{
    char str[maxLocationNameSize];
};

struct LocationNames
{
    LocationName list[maxLocations];
    int count;
};

static char* readEntireFile(const char* path)
{
    std::fstream fs{};
    fs.exceptions(std::fstream::failbit | std::fstream::badbit);
    fs.open(path, std::fstream::in | std::fstream::ate);
    const int fileSize{ static_cast<int>(fs.tellg()) };
    char* fileContents{ new char[fileSize + 1] };
    fs.seekg(0);
    fs.read(fileContents, fileSize);
    fileContents[fileSize] = 0;
    return fileContents;
}

constexpr static bool isWhiteSpace(char c)
{
    switch (c)
    {
    case ' ':
    case '\n':
        return true;
    }
    return false;
}

constexpr static bool isDigit(char c)
{
    return '0' <= c && c <= '9';
}

constexpr static void eatWhiteSpace(const char** ptr)
{
    while (isWhiteSpace(**ptr))
    {
        ++(*ptr);
    }
}

constexpr static void eatChar(const char** ptr, char c)
{
    assert(**ptr == c);
    ++(*ptr);
}

constexpr static void eatString(const char** ptr, const char* str)
{
    assert(*str);
    while (*str)
    {
        eatChar(ptr, *str);
        ++str;
    }
}

constexpr static int eatPositiveInteger(const char** ptr)
{
    int result{ 0 };
    assert(isDigit(**ptr));
    while (isDigit(**ptr))
    {
        result = result * 10 + (**ptr - '0');
        ++(*ptr);
    }
    return result;
}

constexpr static LocationName eatLocationName(const char** ptr)
{
    LocationName result{};
    int len{ 0 };
    while (**ptr != ' ')
    {
        result.str[len] = **ptr;
        ++len;
        ++(*ptr);
    }
    return result;
}

constexpr static bool stringsAreEqual(const char* s1, const char* s2)
{
    while (true)
    {
        if (*s1 == *s2)
        {
            if (!*s1)
            {
                return true;
            }
            ++s1;
            ++s2;
        }
        else
        {
            return false;
        }
    }
    return true;
}

constexpr static int getLocationIndex(LocationNames& names, const LocationName& name)
{
    for (int i = 0; i < names.count; ++i)
    {
        if (stringsAreEqual(names.list[i].str, name.str))
        {
            return i;
        }
    }
    assert(names.count < ARRAY_LENGTH(names.list));
    names.list[names.count++] = name;
    return names.count - 1;
}

constexpr static void findPermutations(int totalCount, int* excluded, int excludedCount, int* permutations, int* permutationsCount)
{
    for (int i = 0; i < totalCount; ++i)
    {
        bool skip = false;
        for (int excludedIndex = 0; excludedIndex < excludedCount; ++excludedIndex)
        {
            if (excluded[excludedIndex] == i)
            {
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            if (excludedCount == totalCount - 1)
            {
                for (int excludedIndex = 0; excludedIndex < excludedCount; ++excludedIndex)
                {
                    permutations[*permutationsCount] = excluded[excludedIndex];
                    *permutationsCount += 1;
                }
                permutations[*permutationsCount] = i;
                *permutationsCount += 1;
            }
            else
            {
                excluded[excludedCount] = i;
                findPermutations(totalCount, excluded, excludedCount + 1, permutations, permutationsCount);
            }
        }
    }
}

constexpr int factorial(int n)
{
    int result = 1;
    for (; n > 0; --n)
    {
        result *= n;
    }
    return result;
}

int main()
{
    LocationNames locationNames{};
    LocationDistances distances{};

    const char* inputText{ readEntireFile("input.txt") };
    const char* p = inputText;
    while (*p)
    {
        LocationName name1 = eatLocationName(&p);
        eatString(&p, " to ");
        LocationName name2 = eatLocationName(&p);
        eatString(&p, " = ");
        int dist = eatPositiveInteger(&p);
        eatWhiteSpace(&p);

        int index1 = getLocationIndex(locationNames, name1);
        int index2 = getLocationIndex(locationNames, name2);
        distances.table[index1][index2] = distances.table[index2][index1] = dist;
    }

    int* excluded = new int[locationNames.count];
    int permutationsCount = factorial(locationNames.count);
    int permutationsSize = permutationsCount * locationNames.count;
    int* permutations = new int[permutationsSize];
    int permutationsFilledSoFar = 0;
    findPermutations(locationNames.count, excluded, 0, permutations, &permutationsFilledSoFar);
    assert(permutationsFilledSoFar == permutationsSize);

    int minDistance = std::numeric_limits<int>::max();
    int maxDistance = -1;

    for (int permutationIndex = 0; permutationIndex < permutationsCount; ++permutationIndex)
    {
        int distance = 0;
        for (int locationIndex = 0; locationIndex < locationNames.count-1; ++locationIndex)
        {
            int index = permutationIndex * locationNames.count + locationIndex;
            int loc1 = permutations[index];
            int loc2 = permutations[index+1];
            distance += distances.table[loc1][loc2];
        }
        if (distance < minDistance)
        {
            minDistance = distance;
        }
        if (distance > maxDistance)
        {
            maxDistance = distance;
        }
    }
    std::cout << "min = " << minDistance << ", max = " << maxDistance << "\n";

    return 0;
}
