#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define ASSERT(x) if(!(x)){*((int*)0) = 1;}
#define ARRAY_COUNT(a) (sizeof(a)/sizeof((a)[0]))

#define KILOBYTES(x) (1024LL*(x))
#define MEGABYTES(x) (1024LL*KILOBYTES(x))
#define GIGABYTES(x) (1024LL*MEGABYTES(x))

//
// Arena
//

struct Arena
{
    void* base;
    size_t size;
    size_t used;
};

static Arena makeArena(void* base, size_t size)
{
    Arena arena = {};
    arena.base = base;
    arena.size = size;
    return arena;
}

static void* pushSize(Arena* arena, size_t size)
{
    void* result = (char*)arena->base + arena->used;
    ASSERT(arena->used + size <= arena->size);
    arena->used += size;
    return result;
}

static Arena makeSubArena(Arena* arena, size_t size)
{
    Arena result = {};
    result.size = size;
    result.base = pushSize(arena, size);
    return result;
}

#define pushType(arena, type) (type*) pushSize(arena, sizeof(type))
#define pushArray(arena, type, count) (type*) pushSize(arena, (count) * sizeof(type))
#define pushString(arena, length) pushArray(arena, char, (length) + 1)

//
//

struct AtomNameSet
{
    const char** names;
    int count;
    int capacity;
};

struct Replacement
{
    char atom;
    char* molecule;
};

struct ReplacementList
{
    Replacement* replacements;
    int count;
    int capacity;
};

struct ParseResult
{
    AtomNameSet atomNames;
    ReplacementList replacements;
    char* goal;
};

struct HeapNode
{
    char* molecule;
    int matchedPrefixLength; // The higher the matchedPrefixLength, the closer the given molecule to the goal
    int steps; // The number of replacements made to the start molecule to get to the given molecule
};

struct Heap
{
    HeapNode* nodes;
    int count;
    int capacity;
};

//
// Strings
//

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

//
// String Set
//

struct StringSet
{
    char** strings;
    size_t count;
    size_t capacity;
};

static StringSet makeStringSet(Arena* arena, int capacity)
{
    StringSet set = {};
    set.strings = pushArray(arena, char*, capacity);
    for (int i = 0; i < capacity; ++i)
    {
        set.strings[i] = 0;
    }
    set.capacity = capacity;
    return set;
}

static void clearStringSet(StringSet* set)
{
    set->count = 0;
    for (int i = 0; i < set->capacity; ++i)
    {
        set->strings[i] = 0;
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

static bool addStringToSet(StringSet* set, char* toAdd)
{
    ASSERT(set->count < set->capacity);
    bool added;
    long long hash = computeStringHash(toAdd);
    size_t index = hash % set->capacity;
    while (true)
    {
        ASSERT(index < set->capacity);
        if (set->strings[index])
        {
            if (stringsAreEqual(set->strings[index], toAdd))
            {
                added = false;
                break;
            }
            else
            {
                index = (index + 1) % set->capacity;
            }
        }
        else
        {
            added = true;
            ++set->count;
            set->strings[index] = toAdd;
            break;
        }
    }
    return added;
}

//
//
//

static Heap makeHeap(Arena* arena, int capacity)
{
    Heap result = {};
    result.capacity = capacity;
    result.nodes = pushArray(arena, HeapNode, capacity);
    return result;
}

static bool isGreater(HeapNode* a, HeapNode* b)
{
    if (a->matchedPrefixLength == b->matchedPrefixLength)
    {
        return a->steps < b->steps;
    }
    return a->matchedPrefixLength > b->matchedPrefixLength;
}

static void insert(Heap* heap, char* molecule, int matchedPrefixLength, int steps)
{
    int newIndex = heap->count;
    ASSERT(newIndex < heap->capacity);

    ++heap->count;

    heap->nodes[newIndex].molecule = molecule;
    heap->nodes[newIndex].matchedPrefixLength = matchedPrefixLength;
    heap->nodes[newIndex].steps = steps;

    // Bubble up
    while (newIndex > 0)
    {
        int parentIndex = (newIndex - 1) / 2;
        if (isGreater(&heap->nodes[newIndex], &heap->nodes[parentIndex]))
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
    ASSERT(heap->count > 0);
    HeapNode result = heap->nodes[0];
    --heap->count;
    if (heap->count > 0)
    {
        int newIndex = 0;
        heap->nodes[newIndex] = heap->nodes[heap->count];

        // Bubble-down
        while (true)
        {
            int leftChildIndex = newIndex * 2 + 1;
            if (leftChildIndex < heap->count)
            {
                int greaterChildIndex;
                int rightChildIndex = newIndex * 2 + 2;
                if (rightChildIndex < heap->count)
                {
                    greaterChildIndex = isGreater(&heap->nodes[leftChildIndex], &heap->nodes[rightChildIndex]) ? leftChildIndex : rightChildIndex;
                }
                else
                {
                    greaterChildIndex = leftChildIndex;
                }
                if (isGreater(&heap->nodes[greaterChildIndex], &heap->nodes[newIndex]))
                {
                    HeapNode tmp = heap->nodes[newIndex];
                    heap->nodes[newIndex] = heap->nodes[greaterChildIndex];
                    heap->nodes[greaterChildIndex] = tmp;
                    newIndex = greaterChildIndex;
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

static int getStringLength(const char* str)
{
    int result = 0;
    while (*str)
    {
        ++result;
        ++str;
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
        WriteConsoleA(stdoutHandle, str, getStringLength(str), NULL, NULL);
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

static char* extractWord(Arena* arena, char** input)
{
    skipWhitespace(input);
    int wordLength = getWordLength(*input);
    char* result = pushString(arena, wordLength);
    for (int letterIndex = 0; letterIndex < wordLength; ++letterIndex, ++(*input))
    {
        result[letterIndex] = **input;
    }
    result[wordLength] = 0;
    return result;
}

static int getRawAtomLength(const char* str)
{
    int length = 0;
    if (*str == 'e')
    {
        length = 1;
    }
    else
    {
        if (isUppercase(*str))
        {
            ++length;
            ++str;
            while (isLowercase(*str))
            {
                ++length;
                ++str;
            }
        }
    }
    return length;
}

static char* extractRawAtom(Arena* arena, const char** input)
{
    int length = getRawAtomLength(*input);
    ASSERT(length > 0);
    char* rawAtom = pushString(arena, length);
    for (int letterIndex = 0; letterIndex < length; ++letterIndex)
    {
        rawAtom[letterIndex] = **input;
        ++(*input);
    }
    rawAtom[length] = 0;
    return rawAtom;
}

static char encodeAtom(AtomNameSet* atomNames, const char* rawAtom)
{
    int index = -1;
    for (int atomIndex = 0; atomIndex < atomNames->count; ++atomIndex)
    {
        if (stringsAreEqual(atomNames->names[atomIndex], rawAtom))
        {
            index = atomIndex;
        }
    }

    if (index == -1)
    {
        index = atomNames->count;
        ASSERT(atomNames->count < atomNames->capacity);
        atomNames->names[atomNames->count] = rawAtom;
        ++atomNames->count;
    }

    char result = (char)index + 'A';
    return result;
}

static int getRawMoleculeAtomCount(const char* rawMolecule)
{
    int atomCount;
    if (*rawMolecule == 'e')
    {
        atomCount = 1;
    }
    else
    {
        atomCount = 0;
        while (*rawMolecule)
        {
            if (isUppercase(*rawMolecule))
            {
                ++atomCount;
                ++rawMolecule;
            }
            else if (isLowercase(*rawMolecule))
            {
                ++rawMolecule;
            }
            else
            {
                break;
            }
        }
    }
    return atomCount;
}

static char* encodeMolecule(Arena* arena, AtomNameSet* atomNames, const char* rawMolecule)
{
    int atomCount = getRawMoleculeAtomCount(rawMolecule);
    ASSERT(atomCount > 0);
    char* result = pushString(arena, atomCount);
    for (int atomIndex = 0; atomIndex < atomCount; ++atomIndex)
    {
        char* rawAtom = extractRawAtom(arena, &rawMolecule);
        result[atomIndex] = encodeAtom(atomNames, rawAtom);
    }
    result[atomCount] = 0;
    return result;
}

static const char* decodeAtom(char atom, AtomNameSet* atomNames)
{
    int index = atom - 'A';
    ASSERT(index < atomNames->count);
    const char* result = atomNames->names[index];
    return result;
}

static void decodeMolecule(const char* molecule, AtomNameSet* atomNames, char* buf, int bufSize)
{
    int len = 0;
    while (*molecule)
    {
        const char* decodedAtom = decodeAtom(*molecule, atomNames);
        while (*decodedAtom)
        {
            buf[len] = *decodedAtom;
            ++len;
            ++decodedAtom;
        }
        ++molecule;
    }
    ASSERT(len < bufSize);
    buf[len] = 0;
}

static Replacement extractReplacement(Arena* arena, AtomNameSet* atomNames, char** input)
{
    Replacement result = {};
    char* atomWord = extractWord(arena, input);
    result.atom = encodeAtom(atomNames, atomWord);
    skipString(input, "=>");
    char* moleculeWord = extractWord(arena, input);
    result.molecule = encodeMolecule(arena, atomNames, moleculeWord);
    return result;
}

static AtomNameSet makeAtomNameSet(Arena* arena, int capacity)
{
    AtomNameSet result = {};
    result.capacity = capacity;
    result.names = pushArray(arena, const char*, capacity);
    return result;
}

static ReplacementList makeReplacementList(Arena* arena, int capacity)
{
    ReplacementList result = {};
    result.capacity = capacity;
    result.replacements = pushArray(arena, Replacement, capacity);
    return result;
}

static ParseResult parseInput(Arena* arena, char* input)
{
    ParseResult result = {};

    result.atomNames = makeAtomNameSet(arena, 128);

    int replacementsCount = getReplacementsCount(input);
    result.replacements = makeReplacementList(arena, replacementsCount);
    result.replacements.count = replacementsCount;

    for (int replacementIndex = 0; replacementIndex < replacementsCount; ++replacementIndex)
    {
        skipWhitespace(&input);
        result.replacements.replacements[replacementIndex] = extractReplacement(arena, &result.atomNames, &input);
    }

    skipWhitespace(&input);
    result.goal = encodeMolecule(arena, &result.atomNames, input);

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
            char* buffer = pushString(arena, fileSize.LowPart);
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
                    arena->used = 0;
                }
            }
        }
        CloseHandle(fileHandle);
    }

    return result;
}

static void getMoleculesAfterOneReplacement(const char* molecule, ReplacementList* replacements, Arena* stringArena, StringSet* stringSet)
{
    int moleculeLen = getStringLength(molecule);
    for (int replacementIndex = 0; replacementIndex < replacements->count; ++replacementIndex)
    {
        Replacement* replacement = &replacements->replacements[replacementIndex];
        int toLen = getStringLength(replacement->molecule);
        for (int atomIndex = 0; atomIndex < moleculeLen; ++atomIndex)
        {
            if (molecule[atomIndex] == replacement->atom)
            {
                int newMoleculeLen = moleculeLen + toLen - 1;
                size_t stringArenaSavedUsed = stringArena->used;
                char* newMolecule = pushString(stringArena, newMoleculeLen);

                int newIndex = 0;
                for (int prefixIndex = 0; prefixIndex < atomIndex; ++prefixIndex, ++newIndex)
                {
                    newMolecule[newIndex] = molecule[prefixIndex];
                }
                for (int toAtomIndex = 0; toAtomIndex < toLen; ++toAtomIndex, ++newIndex)
                {
                    newMolecule[newIndex] = replacement->molecule[toAtomIndex];
                }
                for (int suffixIndex = atomIndex + 1; suffixIndex < moleculeLen; ++suffixIndex, ++newIndex)
                {
                    newMolecule[newIndex] = molecule[suffixIndex];
                }
                ASSERT(newIndex == newMoleculeLen);
                newMolecule[newMoleculeLen] = 0;

                if (!addStringToSet(stringSet, newMolecule))
                {
                    stringArena->used = stringArenaSavedUsed;
                }
            }
        }
    }
}

static int minimum(int a, int b)
{
    return (a < b) ? a : b;
}

struct ReachabilityMatrix
{
    char* data;
    int dimSize;
};

static bool canReach(ReachabilityMatrix* matrix, char atomSrc, char atomDst)
{
    return matrix->data[(atomSrc - 'A') * matrix->dimSize + (atomDst - 'A')] == 1;
}

static ReachabilityMatrix makeReachabilityMatrix(Arena* arena, ReplacementList* replacements, AtomNameSet* atomNames)
{
    ReachabilityMatrix result = {};
    result.dimSize = atomNames->count;
    int sizeBytes = result.dimSize * result.dimSize;
    result.data = pushArray(arena, char, sizeBytes);

    // Clear to zero
    for (int byteIndex = 0; byteIndex < sizeBytes; ++byteIndex)
    {
        result.data[byteIndex] = 0;
    }

    // Atoms can reach themselves
    for (int atom = 0; atom < result.dimSize; ++atom)
    {
        result.data[atom * result.dimSize + atom] = 1;
    }

    for (int replacementIndex = 0; replacementIndex < replacements->count; ++replacementIndex)
    {
        Replacement* replacement = &replacements->replacements[replacementIndex];
        result.data[(replacement->atom - 'A') * result.dimSize + (replacement->molecule[0] - 'A')] = 1;
    }
    
    bool changesWereMade = true;
    while (changesWereMade)
    {
        changesWereMade = false;
        for (int atomSrc = 0; atomSrc < result.dimSize; ++atomSrc)
        {
            for (int atomDst = 0; atomDst < result.dimSize; ++atomDst)
            {
                if (result.data[atomSrc * result.dimSize + atomDst] == 1)
                {
                    for (int i = 0; i < result.dimSize; ++i)
                    {
                        if (result.data[atomDst * result.dimSize + i] == 1 && result.data[atomSrc * result.dimSize + i] == 0)
                        {
                            changesWereMade = true;
                            result.data[atomSrc * result.dimSize + i] = 1;
                        }
                    }
                }
            }
        }
    }

    return result;
}

static int findStepsToGoal(Arena* arena, const char* goal, ReplacementList* replacements, AtomNameSet* atomNames)
{
    size_t savedArenaUsed = arena->used;

    Heap heap = makeHeap(arena, 100000);
    StringSet stringSet = makeStringSet(arena, 10000139);
    ReachabilityMatrix reachabilityMatrix = makeReachabilityMatrix(arena, replacements, atomNames);

    int goalLen = getStringLength(goal);

    char* startMolecule = encodeMolecule(arena, atomNames, "e");
    insert(&heap, startMolecule, 0, 0);

    int result = -1;

    size_t topsRemovedCount = 0;
    time_t startTime = time(NULL);
    char decodedMolecule[1024];
    int prefixLenToSkip = 0;

    while (result < 0)
    {
        printf("\n");
        printf("%zu| Top of heap (%d nodes", topsRemovedCount, heap.count);

        if (heap.count > 0)
        {
            int equalToTopPrefixCount = 0;
            for (int i = 0; i < heap.count; ++i)
            {
                if (heap.nodes[i].matchedPrefixLength == heap.nodes[0].matchedPrefixLength)
                {
                    ++equalToTopPrefixCount;
                }
            }
            printf(", %d of them with %d prefix)\n", equalToTopPrefixCount, heap.nodes[0].matchedPrefixLength);
        }
        else
        {
            printf(")\n");
        }

        int N = 10;
        if (heap.count < N)
        {
            N = heap.count;
        }
        char decodedMolecule[1024];
        for (int index = 0; index < N; ++index)
        {
            HeapNode* node = &heap.nodes[index];
            decodeMolecule(node->molecule + prefixLenToSkip, atomNames, decodedMolecule, ARRAY_COUNT(decodedMolecule));
            printf("%d: %d|%d|%s\n", index, node->matchedPrefixLength, node->steps, decodedMolecule);
        }
        printf("\n");

        HeapNode top = removeTop(&heap);
        int topMoleculeLen = getStringLength(top.molecule);

        prefixLenToSkip = top.matchedPrefixLength;
        if (*(top.molecule + prefixLenToSkip) == 0 || *(goal + prefixLenToSkip) == 0)
        {
            --prefixLenToSkip;
        }

        ++topsRemovedCount;
        if (topsRemovedCount % 1 == 0)
        {
            time_t now = time(NULL);

            //printf("Seconds: %jd\n", now - startTime);
            //printf("Tops removed: %zu\n", topsRemovedCount);
            //printf("Top matched prefix length: %d\n", top.matchedPrefixLength);
            //printf("Top steps: %d\n", top.steps);
            //printf("Top / goal len: %d / %d\n", topMoleculeLen, goalLen);
            //printf("Arena used: %zu / %zu (%f)\n", arena->used, arena->size, (float)arena->used / arena->size);
            //printf("String set used: %zu / %zu (%f)\n", stringSet.count, stringSet.capacity, (float)stringSet.count / stringSet.capacity);
            //printf("Heap used: %d / %d (%f)\n", heap.count, heap.capacity, (float)heap.count / heap.capacity);
            
            decodeMolecule(top.molecule + prefixLenToSkip, atomNames, decodedMolecule, ARRAY_COUNT(decodedMolecule));
            printf("%s\n", decodedMolecule);

            decodeMolecule(goal + prefixLenToSkip, atomNames, decodedMolecule, ARRAY_COUNT(decodedMolecule));
            printf("%s\n", decodedMolecule);
        }

        if (top.matchedPrefixLength == goalLen)
        {
            result = top.steps;
        }
        else
        {
            ASSERT(top.matchedPrefixLength < goalLen);
            if (top.matchedPrefixLength == topMoleculeLen)
            {
                --top.matchedPrefixLength;
                ASSERT(top.molecule[top.matchedPrefixLength] == goal[top.matchedPrefixLength]);
            }
            else
            {
                ASSERT(top.matchedPrefixLength < topMoleculeLen);
                ASSERT(top.molecule[top.matchedPrefixLength] != goal[top.matchedPrefixLength]);
            }
            printf("Replacing: %s -> %s\n", decodeAtom(top.molecule[top.matchedPrefixLength], atomNames), decodeAtom(goal[top.matchedPrefixLength], atomNames));
            for (int replacementIndex = 0; replacementIndex < replacements->count; ++replacementIndex)
            {
                Replacement* replacement = &replacements->replacements[replacementIndex];
                int toLen = getStringLength(replacement->molecule);
                if (replacement->atom == top.molecule[top.matchedPrefixLength])
                {
                    decodeMolecule(replacement->molecule, atomNames, decodedMolecule, ARRAY_COUNT(decodedMolecule));
                    printf("-- Possible replacement: %s > ", decodedMolecule);
                    if (canReach(&reachabilityMatrix, replacement->molecule[0], goal[top.matchedPrefixLength]))
                    {
                        int newMoleculeLen = topMoleculeLen + toLen - 1;
                        if (newMoleculeLen <= goalLen)
                        {
                            size_t savedArenaUsed2 = arena->used;
                            char* newMolecule = pushString(arena, newMoleculeLen);

                            int newIndex = 0;
                            for (int prefixIndex = 0; prefixIndex < top.matchedPrefixLength; ++prefixIndex, ++newIndex)
                            {
                                newMolecule[newIndex] = top.molecule[prefixIndex];
                            }
                            for (int toAtomIndex = 0; toAtomIndex < toLen; ++toAtomIndex, ++newIndex)
                            {
                                newMolecule[newIndex] = replacement->molecule[toAtomIndex];
                            }
                            for (int suffixIndex = top.matchedPrefixLength + 1; suffixIndex < topMoleculeLen; ++suffixIndex, ++newIndex)
                            {
                                newMolecule[newIndex] = top.molecule[suffixIndex];
                            }
                            ASSERT(newIndex == newMoleculeLen);
                            newMolecule[newMoleculeLen] = 0;

                            decodeMolecule(newMolecule + prefixLenToSkip, atomNames, decodedMolecule, ARRAY_COUNT(decodedMolecule));
                            printf("%s", decodedMolecule);

                            if (addStringToSet(&stringSet, newMolecule))
                            {
                                int newMatchedPrefixLength = 0;
                                int minLen = minimum(newMoleculeLen, goalLen);
                                while (newMatchedPrefixLength < minLen && newMolecule[newMatchedPrefixLength] == goal[newMatchedPrefixLength])
                                {
                                    ++newMatchedPrefixLength;
                                }
                                ASSERT(newMatchedPrefixLength >= top.matchedPrefixLength);

                                int newSteps = top.steps + 1;
                                printf("|%d|%d\n", newMatchedPrefixLength, newSteps);
                                insert(&heap, newMolecule, newMatchedPrefixLength, newSteps);
                            }
                            else
                            {
                                printf("already seen, skip\n");
                                arena->used = savedArenaUsed2;
                            }
                        }
                        else
                        {
                            printf("len (%d) > goalLen (%d), skip\n", newMoleculeLen, goalLen);
                        }
                    }
                    else
                    {
                        printf("can't reach %s from %s, skip\n", decodeAtom(goal[top.matchedPrefixLength], atomNames), decodeAtom(replacement->molecule[0], atomNames));
                    }
                }
            }
        }

        getc(stdin);
    }

    arena->used = savedArenaUsed;

    return result;
}

static void addReplacement(ReplacementList* list, Arena* arena, AtomNameSet* atomNames, const char* rawAtom, const char* rawMolecule)
{
    ASSERT(list->count < list->capacity);
    list->replacements[list->count].atom = encodeAtom(atomNames, rawAtom);
    list->replacements[list->count].molecule = encodeMolecule(arena, atomNames, rawMolecule);
    ++list->count;
}

static void printMoleculesInSet(StringSet* set, AtomNameSet* atomNames)
{
    for (int i = 0; i < set->capacity; ++i)
    {
        if (set->strings[i])
        {
            char decodedMolecule[512];
            decodeMolecule(set->strings[i], atomNames, decodedMolecule, ARRAY_COUNT(decodedMolecule));
            printf("%s\n", decodedMolecule);
        }
    }
}

static void testExample1(Arena* arena)
{
    size_t savedArenaUsed = arena->used;

    AtomNameSet atomNames = makeAtomNameSet(arena, 64);
    ReplacementList replacements = makeReplacementList(arena, 16);

    addReplacement(&replacements, arena, &atomNames, "H", "HO");
    addReplacement(&replacements, arena, &atomNames, "H", "OH");
    addReplacement(&replacements, arena, &atomNames, "O", "HH");

    char* goal = encodeMolecule(arena, &atomNames, "HOH");
    StringSet stringSet = makeStringSet(arena, 16);
    getMoleculesAfterOneReplacement(goal, &replacements, arena, &stringSet);

    ASSERT(stringSet.count == 4);

#if 0
    printMoleculesInSet(&stringSet, &atomNames);
#endif

    arena->used = savedArenaUsed;
}

static void testExample2(Arena* arena)
{
    size_t savedArenaUsed = arena->used;

    AtomNameSet atomNames = makeAtomNameSet(arena, 64);
    ReplacementList replacements = makeReplacementList(arena, 16);

    addReplacement(&replacements, arena, &atomNames, "e", "H");
    addReplacement(&replacements, arena, &atomNames, "e", "O");
    addReplacement(&replacements, arena, &atomNames, "H", "HO");
    addReplacement(&replacements, arena, &atomNames, "H", "OH");
    addReplacement(&replacements, arena, &atomNames, "O", "HH");

    char* goal1 = encodeMolecule(arena, &atomNames, "HOH");
    int answer1 = findStepsToGoal(arena, goal1, &replacements, &atomNames);
    ASSERT(answer1 == 3);

    char* goal2 = encodeMolecule(arena, &atomNames, "HOHOHO");
    int answer2 = findStepsToGoal(arena, goal2, &replacements, &atomNames);
    ASSERT(answer2 == 6);

    arena->used = savedArenaUsed;
}

static void doPart1(Arena* arena, ParseResult* parseResult)
{
    size_t savedArenaUsed = arena->used;
    StringSet stringSet = makeStringSet(arena, 2029);
    getMoleculesAfterOneReplacement(parseResult->goal, &parseResult->replacements, arena, &stringSet);
    ASSERT(stringSet.count == 518);
    arena->used = savedArenaUsed;
}

static void doPart2(Arena* arena, ParseResult* parseResult)
{
    int answer = findStepsToGoal(arena, parseResult->goal, &parseResult->replacements, &parseResult->atomNames);
    char buf[64];
    intToString(answer, buf);
    printLine(buf);
}

int main()
{
    size_t arenaSize = GIGABYTES(8);
    void* memory = VirtualAlloc(NULL, arenaSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    ASSERT(memory);
    Arena arena = makeArena(memory, arenaSize);

    //printf("testExample1()...");
    //testExample1(&arena);
    //printf("OK\n");

    //printf("testExample2()...");
    //testExample2(&arena);
    //printf("OK\n");

    char* input = readEntireFile(&arena, "input.txt");
    if (input)
    {
        ParseResult parseResult = parseInput(&arena, input);
        //printf("doPart1()...");
        //printf("OK\n");
        //doPart1(&arena, &parseResult);
        printf("doPart2()...");
        doPart2(&arena, &parseResult);
        printf("OK\n");
    }
    else
    {
        printLine("Failed to read input file");
    }

    return 0;
}
