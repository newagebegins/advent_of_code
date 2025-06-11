#include <windows.h>

#define ASSERT(x) if(!(x)){*((int*)0) = 1;}
#define ARRAY_COUNT(a) (sizeof(a)/sizeof((a)[0]))

#define KILOBYTES(x) (1024LL*(x))
#define MEGABYTES(x) (1024LL*KILOBYTES(x))
#define GIGABYTES(x) (1024LL*MEGABYTES(x))

struct Arena
{
    char* base;
    char* top;
    size_t size;
};

struct StringSet
{
    Arena arena;
    char** data;
    size_t count;
    size_t capacity;
};

struct Replacement
{
    const char* from;
    const char* to;
};

static Arena makeArena(size_t size)
{
    Arena arena = {};
    arena.size = size;
    arena.base = (char*)VirtualAlloc(NULL, arena.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    arena.top = arena.base;
    return arena;
}

static char* arenaAlloc(Arena* arena, size_t size)
{
    char* result = nullptr;
    ASSERT(arena->top + size <= arena->base + arena->size);
    result = arena->top;
    arena->top += size;
    for (char* p = result; p < arena->top; ++p)
    {
        *p = 0;
    }
    return result;
}

static void arenaFree(Arena* arena)
{
    arena->top = arena->base;
}

static int getCStringLength(const char* cStr)
{
    int result = 0;
    while (*cStr)
    {
        ++result;
        ++cStr;
    }
    return result;
}

static void intToString(int n, char* buf)
{
    ASSERT(n >= 0);
    int digitsCount;
    if (n == 0)
    {
        digitsCount = 1;
    }
    else
    {
        digitsCount = 0;
        for (int i = n; i > 0; i /= 10)
        {
            ++digitsCount;
        }
    }

    for (int index = digitsCount - 1; index >= 0; --index, n /= 10)
    {
        int digit = n % 10;
        buf[index] = '0' + (char)digit;
    }

    buf[digitsCount] = 0;
}

static void printLine(const char* str)
{
    HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdoutHandle != NULL && stdoutHandle != INVALID_HANDLE_VALUE)
    {
        WriteConsoleA(stdoutHandle, str, getCStringLength(str), NULL, NULL);
        WriteConsoleA(stdoutHandle, "\n", 1, NULL, NULL);
    }
}

struct ParseResult
{
    char* molecule;
    Replacement* replacements;
    int replacementsCount;
    bool error;
};

static int getReplacementsCount(char* input)
{
    int result = 0;
    while (true)
    {
        if (*input == '\n')
        {
            ++result;
            ++input;
            if (*input == '\n')
            {
                break;
            }
        }
        else
        {
            ++input;
        }
    }
    return result;
}

static bool isWhitespace(char c)
{
    bool result;
    switch (c)
    {
    case ' ':
    case '\n':
    case '\t':
        result = true;
        break;
    default:
        result = false;
        break;
    }
    return result;
}

static void skipWhitespace(char** input)
{
    while (isWhitespace(**input))
    {
        ++(*input);
    }
}

static int getWordLength(char* input)
{
    int result = 0;
    while (!isWhitespace(*input))
    {
        ++result;
        ++input;
    }
    return result;
}

static char* extractWord(Arena* arena, char** input)
{
    skipWhitespace(input);
    int wordLength = getWordLength(*input);
    char* result = arenaAlloc(arena, wordLength + 1);
    for (int letterIndex = 0; letterIndex < wordLength; ++letterIndex, ++(*input))
    {
        result[letterIndex] = **input;
    }
    result[wordLength] = 0;
    return result;
}

static void skipString(char** input, const char* str)
{
    skipWhitespace(input);
    while (*str)
    {
        if (**input == *str)
        {
            ++(*input);
            ++str;
        }
        else
        {
            ASSERT(0);
        }
    }
}

static Replacement extractReplacement(Arena* arena, char** input)
{
    Replacement result = {};
    result.from = extractWord(arena, input);
    skipString(input, "=>");
    result.to = extractWord(arena, input);
    return result;
}

static ParseResult parseInput(Arena* arena, char* input)
{
    ParseResult result = {};
    result.replacementsCount = getReplacementsCount(input);
    result.replacements = (Replacement*)arenaAlloc(arena, result.replacementsCount * sizeof(result.replacements[0]));
    if (result.replacements)
    {
        for (int replacementIndex = 0; replacementIndex < result.replacementsCount; ++replacementIndex)
        {
            result.replacements[replacementIndex] = extractReplacement(arena, &input);
        }
        result.molecule = extractWord(arena, &input);
    }
    else
    {
        result.error = true;
    }
    return result;
}

static char* readEntireFile(Arena* arena, const char* path)
{
    char* result = nullptr;

    HANDLE fileHandle = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            char* buffer = arenaAlloc(arena, fileSize.LowPart + 1);
            if (buffer)
            {
                DWORD bytesRead;
                if (ReadFile(fileHandle, buffer, fileSize.LowPart, &bytesRead, NULL) && bytesRead == fileSize.LowPart)
                {
                    buffer[fileSize.LowPart] = 0;
                    result = buffer;
                }
                else
                {
                    arena->top = buffer;
                }
            }
        }
        CloseHandle(fileHandle);
    }

    return result;
}

static bool stringsAreEqual(const char* a, const char* b)
{
    bool result;
    while (true)
    {
        if (*a == *b)
        {
            if (*a)
            {
                ++a;
                ++b;
            }
            else
            {
                result = true;
                break;
            }
        }
        else
        {
            result = false;
            break;
        }
    }
    return result;
}

static StringSet makeStringSet(Arena* arena, int capacity)
{
    StringSet set = {};
    set.data = (char**)arenaAlloc(arena, capacity * sizeof(set.data[0]));
    set.capacity = capacity;

    set.arena.size = GIGABYTES(4);
    set.arena.base = arenaAlloc(arena, set.arena.size);
    set.arena.top = set.arena.base;
    
    return set;
}

static void clearStringSet(StringSet* set)
{
    arenaFree(&set->arena);
    set->count = 0;
    for (int i = 0; i < set->capacity; ++i)
    {
        set->data[i] = 0;
    }
}

static long long computeStringHash(const char* s) {
    const int p = 31;
    const int m = (int)1e9 + 9;
    long long result = 0;
    long long pPow = 1;
    while (*s)
    {
        result = (result + (*s) * pPow) % m;
        pPow = (pPow * p) % m;
        ++s;
    }
    return result;
}

static bool stringSetContains(StringSet* set, const char* toFind, size_t* outIndex)
{
    bool result;
    long long hash = computeStringHash(toFind);
    size_t index = hash % set->capacity;
    while (true)
    {
        ASSERT(index >= 0);
        ASSERT(index < set->capacity);
        if (set->data[index])
        {
            if (stringsAreEqual(set->data[index], toFind))
            {
                result = true;
                break;
            }
            else
            {
                ++index;
            }
        }
        else
        {
            result = false;
            *outIndex = index;
            break;
        }
    }
    return result;
}

static void addStringToSet(StringSet* set, char* toAdd, size_t index)
{
    ASSERT(set->count < set->capacity);
    ++set->count;
    set->data[index] = toAdd;
}

static void getMoleculesAfterOneReplacement(const char* molecule, Replacement* replacements, int replacementsCount, StringSet* set)
{
    int moleculeLength = getCStringLength(molecule);
    for (int replacementIndex = 0; replacementIndex < replacementsCount; ++replacementIndex)
    {
        Replacement replacement = replacements[replacementIndex];
        int fromLength = getCStringLength(replacement.from);
        int toLength = getCStringLength(replacement.to);
        for (int moleculeLetterIndex = 0; moleculeLetterIndex < moleculeLength; ++moleculeLetterIndex)
        {
            if (moleculeLetterIndex + fromLength - 1 < moleculeLength)
            {
                bool foundFrom = true;
                for (int fromLetterIndex = 0; fromLetterIndex < fromLength; ++fromLetterIndex)
                {
                    if (replacement.from[fromLetterIndex] != molecule[moleculeLetterIndex + fromLetterIndex])
                    {
                        foundFrom = false;
                    }
                }
                if (foundFrom)
                {
                    int newMoleculeLength = moleculeLength + (toLength - fromLength);
                    char* savedArenaTop = set->arena.top;
                    char* newMolecule = arenaAlloc(&set->arena, newMoleculeLength + 1);

                    int newIndex = 0;
                    for (int prefixIndex = 0; prefixIndex < moleculeLetterIndex; ++prefixIndex, ++newIndex)
                    {
                        newMolecule[newIndex] = molecule[prefixIndex];
                    }
                    for (int toLetterIndex = 0; toLetterIndex < toLength; ++toLetterIndex, ++newIndex)
                    {
                        newMolecule[newIndex] = replacement.to[toLetterIndex];
                    }
                    for (int suffixIndex = moleculeLetterIndex + fromLength; suffixIndex < moleculeLength; ++suffixIndex, ++newIndex)
                    {
                        newMolecule[newIndex] = molecule[suffixIndex];
                    }
                    ASSERT(newIndex == newMoleculeLength);
                    newMolecule[newMoleculeLength] = 0;

                    size_t newMoleculeIndex;
                    if (!stringSetContains(set, newMolecule, &newMoleculeIndex))
                    {
                        addStringToSet(set, newMolecule, newMoleculeIndex);
                    }
                    else
                    {
                        set->arena.top = savedArenaTop;
                    }
                }
            }
        }
    }
}

static int findFewestNumberOfStepsToTheMedicineMolecule(Arena* arena, const char* medicineMolecule, Replacement* replacements, int replacementsCount)
{
    int steps;
    char* savedArenaTop = arena->top;
    
    constexpr int setCapacity = 100000073;
    StringSet currentSet = makeStringSet(arena, setCapacity);
    StringSet nextSet = makeStringSet(arena, setCapacity);

    char* startMolecule = arenaAlloc(&currentSet.arena, 2);
    startMolecule[0] = 'e';
    startMolecule[1] = 0;
    size_t startMoleculeIndex;
    if (!stringSetContains(&currentSet, startMolecule, &startMoleculeIndex))
    {
        addStringToSet(&currentSet, startMolecule, startMoleculeIndex);
    }
    else
    {
        ASSERT(0);
    }

    size_t dummy;
    for (steps = 0; !stringSetContains(&currentSet, medicineMolecule, &dummy); ++steps)
    {
#if 1
        //printLine("");
        char buf[64];
        intToString(steps, buf);
        printLine(buf);
#endif
        for (int i = 0; i < currentSet.capacity; ++i)
        {
            const char* molecule = currentSet.data[i];
            if (molecule)
            {
#if 0
                printLine(molecule);
#endif
                getMoleculesAfterOneReplacement(molecule, replacements, replacementsCount, &nextSet);
            }
        }
        
        StringSet temp = currentSet;
        currentSet = nextSet;
        nextSet = temp;
        clearStringSet(&nextSet);
    }

#if 0
    printLine("\nFinal:");
    for (int i = 0; i < currentSet.capacity; ++i)
    {
        const char* molecule = currentSet.data[i];
        if (molecule)
        {
            printLine(molecule);
        }
    }
#endif
    arena->top = savedArenaTop;

    return steps;
}

static void testExample1(Arena* arena)
{
    char* savedArenaTop = arena->top;
    Replacement exampleReplacements[] = {
        {"H", "HO"},
        {"H", "OH"},
        {"O", "HH"}
    };
    StringSet exampleSet = makeStringSet(arena, MEGABYTES(4));
    getMoleculesAfterOneReplacement("HOH", exampleReplacements, ARRAY_COUNT(exampleReplacements), &exampleSet);
    ASSERT(exampleSet.count == 4);
    char buf[64];
    intToString((int)exampleSet.count, buf);
    printLine(buf);
    arena->top = savedArenaTop;
}

static void testExample2(Arena* arena)
{
    char* savedArenaTop = arena->top;

    Replacement exampleReplacements[] = {
        {"e", "H"},
        {"e", "O"},
        {"H", "HO"},
        {"H", "OH"},
        {"O", "HH"}
    };

    int answer1 = findFewestNumberOfStepsToTheMedicineMolecule(arena, "HOH", exampleReplacements, ARRAY_COUNT(exampleReplacements));
    ASSERT(answer1 == 3);

    int answer2 = findFewestNumberOfStepsToTheMedicineMolecule(arena, "HOHOHO", exampleReplacements, ARRAY_COUNT(exampleReplacements));
    ASSERT(answer2 == 6);

    arena->top = savedArenaTop;
}

static void doPart1(Arena* arena, ParseResult* parseResult)
{
    char* savedArenaTop = arena->top;
    StringSet set = makeStringSet(arena, 500057);
    getMoleculesAfterOneReplacement(parseResult->molecule, parseResult->replacements, parseResult->replacementsCount, &set);
    ASSERT(set.count == 518);
    char buf[64];
    intToString((int)set.count, buf);
    printLine(buf);
    arena->top = savedArenaTop;
}

static void doPart2(Arena* arena, ParseResult* parseResult)
{
    int answer = findFewestNumberOfStepsToTheMedicineMolecule(arena, parseResult->molecule, parseResult->replacements, parseResult->replacementsCount);
    char buf[64];
    intToString(answer, buf);
    printLine(buf);
}

int main()
{
    Arena arena = makeArena(GIGABYTES(12));

    if (arena.base)
    {
        testExample1(&arena);
        testExample2(&arena);

        char* input = readEntireFile(&arena, "input.txt");
        if (input)
        {
            ParseResult parseResult = parseInput(&arena, input);
            if (!parseResult.error)
            {
                doPart1(&arena, &parseResult);
                doPart2(&arena, &parseResult);
            }
            else
            {
                printLine("Failed to parse the input");
            }
        }
        else
        {
            printLine("Failed to read input file");
        }
    }
    else
    {
        printLine("VirtualAlloc() failed");
    }

    return 0;
}
