#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define TEST_GET_WIRE_INDEX 0

#define ARRAY_LENGTH(a) (sizeof((a))/sizeof((a)[0]))

enum Gate : uint8_t
{
    Gate_None,
    Gate_AND,
    Gate_OR,
    Gate_NOT,
    Gate_LSHIFT,
    Gate_RSHIFT,
};

// Flags
#define IS_COMPUTED  (1 << 0)
#define ARG1_IS_WIRE (1 << 1)
#define ARG2_IS_WIRE (1 << 2)

struct Wire
{
    uint8_t flags;
    Gate gate;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t value;
};

struct WireName
{
    char str[3];
};

constexpr static bool isDigit(char c)
{
    return '0' <= c && c <= '9';
}

constexpr static bool isLowerCaseLetter(char c)
{
    return 'a' <= c && c <= 'z';
}

constexpr static bool isUpperCaseLetter(char c)
{
    return 'A' <= c && c <= 'Z';
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

static char* readEntireFile(const char* filePath)
{
    char* result{ nullptr };
    FILE* file;
    fopen_s(&file, filePath, "r");
    if (file)
    {
        if (fseek(file, 0, SEEK_END) == 0)
        {
            const long fileSizeSigned{ ftell(file) };
            if (fileSizeSigned != -1L)
            {
                const size_t fileSize{ static_cast<size_t>(fileSizeSigned) };
                rewind(file);
                result = (char*)malloc(fileSize + 1);
                if (result)
                {
                    if (fread(result, fileSize, 1, file) == 1)
                    {
                        result[fileSize] = 0;
                    }
                    else
                    {
                        free(result);
                        result = nullptr;
                    }
                }
            }
        }
    }
    return result;
}

constexpr static uint16_t getWireIndex(WireName name)
{
    int result{ 0 };
    constexpr int base{ 'z' - 'a' + 1 };
    for (const char* p{ name.str }; *p; ++p)
    {
        result = result * base + (*p - 'a' + 1);
    }
    assert(result > 0);
    return static_cast<uint16_t>(result);
}

#if TEST_GET_WIRE_INDEX
constexpr static void testGetWireIndex()
{
    WireName name{};
    for (char c{ 'a' }; c <= 'z'; ++c)
    {
        name.str[0] = c;
        printf("%s -> %d\n", name.str, getWireIndex(name));
    }
    for (char i{ 'a' }; i <= 'z'; ++i)
    {
        name.str[0] = i;
        for (char j{ 'a' }; j <= 'z'; ++j)
        {
            name.str[1] = j;
            printf("%s -> %d\n", name.str, getWireIndex(name));
        }
    }
}
#endif

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

constexpr static Gate eatGate(const char** ptr)
{
    eatWhiteSpace(ptr);

    Gate gate{ Gate_None };

    switch (**ptr)
    {
    case 'A':
        gate = Gate_AND;
        eatString(ptr, "AND");
        break;
    case 'O':
        gate = Gate_OR;
        eatString(ptr, "OR");
        break;
    case 'N':
        gate = Gate_NOT;
        eatString(ptr, "NOT");
        break;
    case 'L':
        gate = Gate_LSHIFT;
        eatString(ptr, "LSHIFT");
        break;
    case 'R':
        gate = Gate_RSHIFT;
        eatString(ptr, "RSHIFT");
        break;
    default:
        assert(0);
    }

    return gate;
}

constexpr static WireName eatWireName(const char** ptr)
{
    eatWhiteSpace(ptr);
    WireName name{};
    int len{ 0 };
    assert(isLowerCaseLetter(**ptr));
    while (isLowerCaseLetter(**ptr))
    {
        assert(len < ARRAY_LENGTH(name.str));
        name.str[len] = **ptr;
        ++(*ptr);
        ++len;
    }
    return name;
}

constexpr static void eatArrow(const char** ptr)
{
    eatWhiteSpace(ptr);
    eatString(ptr, "->");
}

static uint16_t eatUInt16(const char** ptr)
{
    uint16_t result{ 0 };
    assert(isDigit(**ptr));
    while (isDigit(**ptr))
    {
        result = result * 10u + (**ptr - '0');
        ++(*ptr);
    }
    return result;
}

constexpr static int eatWire(const char** ptr, Wire* wire)
{
    eatWhiteSpace(ptr);
    int outputWireIndex{ -1 };
    if (isUpperCaseLetter(**ptr))
    {
        // Example: NOT dq -> dr
        wire->gate = eatGate(ptr);
        assert(wire->gate == Gate_NOT);
        wire->arg1 = getWireIndex(eatWireName(ptr));
        wire->flags |= ARG1_IS_WIRE;
        eatArrow(ptr);
        outputWireIndex = getWireIndex(eatWireName(ptr));
    }
    else
    {
        // Examples:
        //   1674 -> b
        //   lx -> a
        //   1 AND cx -> cy
        //   b RSHIFT 2 -> d
        //   dd AND do -> dq

        if (isDigit(**ptr))
        {
            wire->arg1 = eatUInt16(ptr);
        }
        else
        {
            wire->arg1 = getWireIndex(eatWireName(ptr));
            wire->flags |= ARG1_IS_WIRE;
        }       
        eatWhiteSpace(ptr);
        if (isUpperCaseLetter(**ptr))
        {
            wire->gate = eatGate(ptr);
            eatWhiteSpace(ptr);
            if (isDigit(**ptr))
            {
                wire->arg2 = eatUInt16(ptr);
            }
            else
            {
                wire->arg2 = getWireIndex(eatWireName(ptr));
                wire->flags |= ARG2_IS_WIRE;
            }
        }
        eatArrow(ptr);
        outputWireIndex = getWireIndex(eatWireName(ptr));
    }
    assert(outputWireIndex > 0);
    return outputWireIndex;
}

static uint16_t getWireValue(Wire* wires, uint16_t wireIndex)
{
    Wire* wire{ &wires[wireIndex] };
    if (!(wire->flags & IS_COMPUTED))
    {
        uint16_t arg1Value{ (wire->flags & ARG1_IS_WIRE) ? getWireValue(wires, wire->arg1) : wire->arg1 };
        uint16_t arg2Value{ (wire->flags & ARG2_IS_WIRE) ? getWireValue(wires, wire->arg2) : wire->arg2 };

        int value{};

        switch (wire->gate)
        {
        case Gate_None:
            value = arg1Value;
            break;
        case Gate_AND:
            value = arg1Value & arg2Value;
            break;
        case Gate_OR:
            value = arg1Value | arg2Value;
            break;
        case Gate_NOT:
            value = ~arg1Value;
            break;
        case Gate_LSHIFT:
            value = arg1Value << arg2Value;
            break;
        case Gate_RSHIFT:
            value = arg1Value >> arg2Value;
            break;
        default:
            assert(0);
        }

        wire->value = static_cast<uint16_t>(value);
        wire->flags |= IS_COMPUTED;
    }
    return wire->value;
}

void resetWires(Wire* wires, int wiresCount)
{
    for (int i{ 0 }; i < wiresCount; ++i)
    {
        wires[i].flags &= ~IS_COMPUTED;
    }
}

int main()
{
#if TEST_GET_WIRE_INDEX
    testGetWireIndex();
#endif
    const char* const inputText{ readEntireFile("input.txt") };
    assert(inputText);

    constexpr int maxWiresCount{ 1024 };
    Wire* wires{ (Wire*)calloc(maxWiresCount, sizeof(wires[0])) };
    assert(wires);

    // Populate wires
    {
        const char* ptr{ inputText };
        while (*ptr)
        {
            Wire wire{};
            const int outputWireIndex{ eatWire(&ptr, &wire) };
            assert(outputWireIndex < maxWiresCount);
            wires[outputWireIndex] = wire;
            eatWhiteSpace(&ptr);
        }
    }

    // Part1

    uint16_t answer1{ getWireValue(wires, getWireIndex({ "a" })) };
    assert(answer1 == 46065);
    printf("Part 1\na: %u\n", answer1);

    // Part 2

    Wire* wireB{ &wires[getWireIndex({ "b" })] };
    wireB->arg1 = answer1;

    resetWires(wires, maxWiresCount);

    uint16_t answer2{ getWireValue(wires, getWireIndex({ "a" })) };
    printf("Part 2\na: %u\n", answer2);

    return 0;
}
