#include <windows.h>
#include <stdint.h>

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

typedef uint8_t Atom;

struct Molecule
{
    Atom atoms[300];
    int atomsCount;
};

struct AtomName
{
    char str[3];
};

struct AtomNames
{
    AtomName names[64];
    int count;
};

struct Replacement
{
    Atom from;
    Molecule to;
};

struct Replacements
{
    Replacement replacements[64];
    int count;
};

struct ParseResult
{
    AtomNames atomNames;
    Replacements replacements;
    Molecule goal;
};

struct HeapNode
{
    Molecule molecule;
    int score;
    int steps;
};

struct Heap
{
    HeapNode nodes[4096*2*2*2*2*2*2*2*2*2*2];
    int nodesCount;
};

static bool isUppercase(char c)
{
    return c >= 'A' && c <= 'Z';
}

static bool isLowercase(char c)
{
    return c >= 'a' && c <= 'z';
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

bool moleculesAreEqual(Molecule* a, Molecule* b)
{
    if (a->atomsCount == b->atomsCount)
    {
        for (int i = 0; i < a->atomsCount; ++i)
        {
            if (a->atoms[i] != b->atoms[i])
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

static void insert(Heap* heap, Molecule* molecule, int score, int steps)
{
    int newIndex = heap->nodesCount;
    ASSERT(newIndex < ARRAY_COUNT(heap->nodes));

    ++heap->nodesCount;

    heap->nodes[newIndex].molecule = *molecule;
    heap->nodes[newIndex].score = score;
    heap->nodes[newIndex].steps = steps;

    while (newIndex > 0)
    {
        int parentIndex = (newIndex - 1) / 2;
        if (heap->nodes[parentIndex].score < score)
        {
            HeapNode tmp = heap->nodes[parentIndex];
            heap->nodes[parentIndex] = heap->nodes[newIndex];
            heap->nodes[newIndex] = tmp;
            newIndex = parentIndex;
        }
        else
        {
            break;
        }
    }
}

static HeapNode removeTop(Heap* heap)
{
    ASSERT(heap->nodesCount > 0);
    HeapNode result = heap->nodes[0];
    --heap->nodesCount;
    if (heap->nodesCount > 0)
    {
        int newIndex = 0;
        heap->nodes[newIndex] = heap->nodes[heap->nodesCount];
        while (true)
        {
            int leftChildIndex = newIndex * 2 + 1;
            if (leftChildIndex < heap->nodesCount)
            {
                int biggerChildIndex;
                int rightChildIndex = newIndex * 2 + 2;
                if (rightChildIndex < heap->nodesCount)
                {
                    biggerChildIndex = heap->nodes[leftChildIndex].score > heap->nodes[rightChildIndex].score ? leftChildIndex : rightChildIndex;
                }
                else
                {
                    biggerChildIndex = leftChildIndex;
                }
                if (heap->nodes[newIndex].score < heap->nodes[biggerChildIndex].score)
                {
                    HeapNode tmp = heap->nodes[newIndex];
                    heap->nodes[newIndex] = heap->nodes[biggerChildIndex];
                    heap->nodes[biggerChildIndex] = tmp;
                    newIndex = biggerChildIndex;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    return result;
}

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

static void extractWord(char** input, char* buf, int bufSize)
{
    skipWhitespace(input);
    int wordLength = getWordLength(*input);
    ASSERT(wordLength < bufSize);
    for (int letterIndex = 0; letterIndex < wordLength; ++letterIndex, ++(*input))
    {
        buf[letterIndex] = **input;
    }
    buf[wordLength] = 0;
}

static bool extractAtomName(const char** input, AtomName* atomName)
{
    bool result = false;
    int atomNameLength = 0;
    if (isUppercase(**input))
    {
        atomName->str[atomNameLength++] = **input;
        ++(*input);

        while (isLowercase(**input))
        {
            atomName->str[atomNameLength++] = **input;
            ++(*input);
        }
    }
    if (atomNameLength > 0)
    {
        ASSERT(atomNameLength < ARRAY_COUNT(atomName->str));
        atomName->str[atomNameLength] = 0;
        result = true;
    }
    return result;
}

static Atom stringToAtom(AtomNames* atomNames, const char* str)
{
    for (int atomIndex = 0; atomIndex < atomNames->count; ++atomIndex)
    {
        if (stringsAreEqual(atomNames->names[atomIndex].str, str))
        {
            return atomIndex;
        }
    }

    Atom result = (Atom)atomNames->count;
    ASSERT(atomNames->count < ARRAY_COUNT(atomNames->names));
    AtomName* name = &atomNames->names[atomNames->count];

    for (int i = 0; i < ARRAY_COUNT(name->str) && *str; ++i, ++str)
    {
        name->str[i] = *str;
    }
    ASSERT(*str == 0);
    ++atomNames->count;
    return result;
}

static Molecule stringToMolecule(AtomNames* atomNames, const char* str)
{
    Molecule result = {};
    AtomName atomName;
    while (extractAtomName(&str, &atomName))
    {
        result.atoms[result.atomsCount] = stringToAtom(atomNames, atomName.str);
        ++result.atomsCount;
    }
    return result;
}

static Replacement extractReplacement(AtomNames* atomNames, char** input)
{
    Replacement result = {};
    char buf[64];

    extractWord(input, buf, ARRAY_COUNT(buf));
    result.from = stringToAtom(atomNames, buf);

    skipString(input, "=>");

    extractWord(input, buf, ARRAY_COUNT(buf));
    result.to = stringToMolecule(atomNames, buf);

    return result;
}

static ParseResult parseInput(char* input)
{
    ParseResult result = {};
    result.replacements.count = getReplacementsCount(input);
    for (int replacementIndex = 0; replacementIndex < result.replacements.count; ++replacementIndex)
    {
        skipWhitespace(&input);
        result.replacements.replacements[replacementIndex] = extractReplacement(&result.atomNames, &input);
    }
    skipWhitespace(&input);
    result.goal = stringToMolecule(&result.atomNames, input);
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

struct MoleculesAfterOneReplacement
{
    Molecule molecules[1024];
    int count;
};

static void getMoleculesAfterOneReplacement(Molecule* molecule, Replacements* replacements, MoleculesAfterOneReplacement* out)
{
    for (int replacementIndex = 0; replacementIndex < replacements->count; ++replacementIndex)
    {
        Replacement* replacement = &replacements->replacements[replacementIndex];
        for (int atomIndex = 0; atomIndex < molecule->atomsCount; ++atomIndex)
        {
            if (molecule->atoms[atomIndex] == replacement->from)
            {
                Molecule newMolecule = {};
                newMolecule.atomsCount = molecule->atomsCount + replacement->to.atomsCount - 1;

                int newIndex = 0;
                for (int prefixIndex = 0; prefixIndex < atomIndex; ++prefixIndex, ++newIndex)
                {
                    newMolecule.atoms[newIndex] = molecule->atoms[prefixIndex];
                }
                for (int toAtomIndex = 0; toAtomIndex < replacement->to.atomsCount; ++toAtomIndex, ++newIndex)
                {
                    newMolecule.atoms[newIndex] = replacement->to.atoms[toAtomIndex];
                }
                for (int suffixIndex = atomIndex + 1; suffixIndex < molecule->atomsCount; ++suffixIndex, ++newIndex)
                {
                    newMolecule.atoms[newIndex] = molecule->atoms[suffixIndex];
                }
                ASSERT(newIndex == newMolecule.atomsCount);

                bool found = false;
                for (int i = 0; i < out->count; ++i)
                {
                    if (moleculesAreEqual(&newMolecule, &out->molecules[i]))
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    ASSERT(out->count < ARRAY_COUNT(out->molecules));
                    out->molecules[out->count] = newMolecule;
                    ++out->count;
                }
            }
        }
    }
}

static int computeScore(Molecule* molecule, Molecule* goal)
{
    ASSERT(molecule->atomsCount <= goal->atomsCount);
    int score = 0;
    int nextG = 0;
    for (int m = 0; m < molecule->atomsCount; ++m)
    {
        for (int g = nextG; g < goal->atomsCount; ++g)
        {
            if (molecule->atoms[m] == goal->atoms[g])
            {
                ++score;
                nextG = g + 1;
                break;
            }
        }
    }
    return score;
}

static void testComputeScore()
{
    AtomNames atomNames = {};
    Molecule m1;
    Molecule m2;

#define TEST_COMPUTE_SCORE(M1, M2, expectedScore)       \
    m1 = stringToMolecule(&atomNames, (M1));            \
    m2 = stringToMolecule(&atomNames, (M2));            \
    ASSERT(computeScore(&m1, &m2) == (expectedScore));

    TEST_COMPUTE_SCORE("H", "H", 1);
    TEST_COMPUTE_SCORE("H", "HO", 1);
    TEST_COMPUTE_SCORE("H", "O", 0);
    TEST_COMPUTE_SCORE("XOHOH", "OOOOHOH", 4);
    TEST_COMPUTE_SCORE("HHH", "HOHOHO", 3);
    TEST_COMPUTE_SCORE("OHHH", "HOHOHO", 3);
    TEST_COMPUTE_SCORE("AlCaH", "AlCaH", 3);

#undef TEST_COMPUTE_SCORE
}

Atom getAtomByName(AtomNames* atomNames, const char* name)
{
    for (int i = 0; i < atomNames->count; ++i)
    {
        if (stringsAreEqual(atomNames->names[i].str, name))
        {
            return i;
        }
    }
    ASSERT(0);
    return 0;
}

static int findFewestNumberOfStepsToTheMedicineMolecule(Arena* arena, Molecule* medicineMolecule, Replacements* replacements, AtomNames* atomNames)
{
    char* savedArenaTop = arena->top;

    Heap* heap = (Heap*)arenaAlloc(arena, sizeof(Heap));
    MoleculesAfterOneReplacement* moleculesAfterOneReplacement = (MoleculesAfterOneReplacement*)arenaAlloc(arena, sizeof(MoleculesAfterOneReplacement));

    Molecule startMolecule = {};
    startMolecule.atoms[0] = getAtomByName(atomNames, "e");
    startMolecule.atomsCount = 1;
    insert(heap, &startMolecule, 0, 0);

    int result = -1;
    while (result < 0)
    {
        HeapNode top = removeTop(heap);
        if (moleculesAreEqual(&top.molecule, medicineMolecule))
        {
            result = top.steps;
        }
        else
        {
            getMoleculesAfterOneReplacement(&top.molecule, replacements, moleculesAfterOneReplacement);
            for (int i = 0; i < moleculesAfterOneReplacement->count; ++i)
            {
                Molecule* molecule = &moleculesAfterOneReplacement->molecules[i];
                if (molecule->atomsCount <= medicineMolecule->atomsCount)
                {
                    int score = computeScore(molecule, medicineMolecule);
                    insert(heap, molecule, score, top.steps + 1);
                }
            }
            moleculesAfterOneReplacement->count = 0;
        }
    }
    
    arena->top = savedArenaTop;
    return result;
}

static void testExample1(Arena* arena)
{
    char* savedArenaTop = arena->top;

    AtomNames atomNames = {};
    Replacements replacements = {};

    replacements.replacements[replacements.count].from = stringToAtom(&atomNames, "H");
    replacements.replacements[replacements.count].to = stringToMolecule(&atomNames, "HO");
    ++replacements.count;

    replacements.replacements[replacements.count].from = stringToAtom(&atomNames, "H");
    replacements.replacements[replacements.count].to = stringToMolecule(&atomNames, "OH");
    ++replacements.count;

    replacements.replacements[replacements.count].from = stringToAtom(&atomNames, "O");
    replacements.replacements[replacements.count].to = stringToMolecule(&atomNames, "HH");
    ++replacements.count;

    MoleculesAfterOneReplacement* moleculesAfterOneReplacement = (MoleculesAfterOneReplacement*)arenaAlloc(arena, sizeof(MoleculesAfterOneReplacement));

    Molecule goal = stringToMolecule(&atomNames, "HOH");
    getMoleculesAfterOneReplacement(&goal, &replacements, moleculesAfterOneReplacement);

    ASSERT(moleculesAfterOneReplacement->count == 4);

    arena->top = savedArenaTop;
}

static void testExample2(Arena* arena)
{
    char* savedArenaTop = arena->top;

    AtomNames atomNames = {};
    Replacements replacements = {};

    replacements.replacements[replacements.count].from = stringToAtom(&atomNames, "e");
    replacements.replacements[replacements.count].to = stringToMolecule(&atomNames, "H");
    ++replacements.count;

    replacements.replacements[replacements.count].from = stringToAtom(&atomNames, "e");
    replacements.replacements[replacements.count].to = stringToMolecule(&atomNames, "O");
    ++replacements.count;

    replacements.replacements[replacements.count].from = stringToAtom(&atomNames, "H");
    replacements.replacements[replacements.count].to = stringToMolecule(&atomNames, "HO");
    ++replacements.count;

    replacements.replacements[replacements.count].from = stringToAtom(&atomNames, "H");
    replacements.replacements[replacements.count].to = stringToMolecule(&atomNames, "OH");
    ++replacements.count;

    replacements.replacements[replacements.count].from = stringToAtom(&atomNames, "O");
    replacements.replacements[replacements.count].to = stringToMolecule(&atomNames, "HH");
    ++replacements.count;

    Molecule goal1 = stringToMolecule(&atomNames, "HOH");
    int answer1 = findFewestNumberOfStepsToTheMedicineMolecule(arena, &goal1, &replacements, &atomNames);
    ASSERT(answer1 == 3);

    Molecule goal2 = stringToMolecule(&atomNames, "HOHOHO");
    int answer2 = findFewestNumberOfStepsToTheMedicineMolecule(arena, &goal2, &replacements, &atomNames);
    ASSERT(answer2 == 6);

    arena->top = savedArenaTop;
}

static void doPart1(Arena* arena, ParseResult* parseResult)
{
    char* savedArenaTop = arena->top;
    MoleculesAfterOneReplacement* moleculesAfterOneReplacement = (MoleculesAfterOneReplacement*)arenaAlloc(arena, sizeof(MoleculesAfterOneReplacement));
    getMoleculesAfterOneReplacement(&parseResult->goal, &parseResult->replacements, moleculesAfterOneReplacement);
    ASSERT(moleculesAfterOneReplacement->count == 518);
    arena->top = savedArenaTop;
}

static void doPart2(Arena* arena, ParseResult* parseResult)
{
    int answer = findFewestNumberOfStepsToTheMedicineMolecule(arena, &parseResult->goal, &parseResult->replacements, &parseResult->atomNames);
    char buf[64];
    intToString(answer, buf);
    printLine(buf);
}

int main()
{
    testComputeScore();

    Arena arena = makeArena(GIGABYTES(4));

    if (arena.base)
    {
        testExample1(&arena);
        testExample2(&arena);

        char* input = readEntireFile(&arena, "input.txt");
        if (input)
        {
            ParseResult parseResult = parseInput(input);
            doPart1(&arena, &parseResult);
            doPart2(&arena, &parseResult);
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
