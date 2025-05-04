#include <windows.h>
#include <stdint.h>
#include <stdio.h>

#define ASSERT(x) if (!(x)) { *(int*)0 = 1; }

#define PART2

constexpr int gridSize = 1000;

static int8_t lights[gridSize][gridSize];

enum Command
{
    Command_Toggle,
    Command_TurnOn,
    Command_TurnOff,
};

constexpr static int getStringLength(const char* str)
{
    int result = 0;
    for (; *str; ++str)
    {
        ++result;
    }
    return result;
}

static bool isDigit(char c)
{
    return '0' <= c && c <= '9';
}

static int eatInteger(const char** input)
{
    ASSERT(isDigit(**input));
    int result = 0;
    const char* p = *input;
    while (true)
    {
        char c = *p;
        if (isDigit(c))
        {
            result = result*10 + (c - '0');
            ++p;
        }
        else
        {
            break;
        }
    }
    *input = p;
    return result;
}

static void eatString(const char** input, const char* str)
{
    ASSERT(**input);
    ASSERT(*str);
    for (; *str; ++str, ++(*input))
    {
        ASSERT(*str == **input);
    }
}

static void eatChar(const char** input, char c)
{
    ASSERT(**input == c);
    ++(*input);
}

static void toggle(int x1, int y1, int x2, int y2)
{
    for (int y = y1; y <= y2; ++y)
    {
        for (int x = x1; x <= x2; ++x)
        {
#ifdef PART2
            lights[y][x] += 2;
#else
            lights[y][x] ^= 1;
#endif
        }
    }
}

static void turnOn(int x1, int y1, int x2, int y2)
{
    for (int y = y1; y <= y2; ++y)
    {
        for (int x = x1; x <= x2; ++x)
        {
#ifdef PART2
            lights[y][x] += 1;
#else
            lights[y][x] = 1;
#endif
        }
    }
}

static void turnOff(int x1, int y1, int x2, int y2)
{
    for (int y = y1; y <= y2; ++y)
    {
        for (int x = x1; x <= x2; ++x)
        {
#ifdef PART2
            lights[y][x] -= 1;
            if (lights[y][x] < 0)
            {
                lights[y][x] = 0;
            }
#else
            lights[y][x] = 0;
#endif
        }
    }
}

static int countLitLights()
{
    int result = 0;
    for (int y = 0; y < gridSize; ++y)
    {
        for (int x = 0; x < gridSize; ++x)
        {
            ASSERT(result < INT_MAX - lights[y][x]);
            result += lights[y][x];
        }
    }
    return result;
}

int main()
{
    const char* const inputFilePath = "input.txt";

    const HANDLE inputFileHandle = CreateFileA(
        inputFilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    const DWORD fileSize = GetFileSize(
        inputFileHandle,
        NULL
    );

    ASSERT(fileSize != INVALID_FILE_SIZE);

    if (inputFileHandle != INVALID_HANDLE_VALUE)
    {
        const HANDLE fileMappingObjectHandle = CreateFileMappingA(
            inputFileHandle,
            NULL,
            PAGE_READONLY,
            0,
            0,
            NULL
        );

        if (fileMappingObjectHandle)
        {
            const void* inputFileMemory = MapViewOfFile(
                fileMappingObjectHandle,
                FILE_MAP_READ,
                0,
                0,
                0
            );

            if (inputFileMemory)
            {
                const char* p = (char*)inputFileMemory;
                while (p - (const char*)inputFileMemory < fileSize)
                {
                    eatChar(&p, 't');
                    Command command{};
                    switch (*p)
                    {
                    case 'o': // toggle
                        eatString(&p, "oggle");
                        command = Command_Toggle;
                        break;
                    case 'u': // turn
                        eatString(&p, "urn o");
                        switch (*p)
                        {
                        case 'n': // on
                            ++p;
                            command = Command_TurnOn;
                            break;
                        case 'f': // off
                            eatString(&p, "ff");
                            command = Command_TurnOff;
                            break;
                        default:
                            ASSERT(0);
                            break;
                        }
                        break;
                    default:
                        ASSERT(0);
                        break;
                    }
                    eatChar(&p, ' ');
                    int x1 = eatInteger(&p);
                    eatChar(&p, ',');
                    int y1 = eatInteger(&p);
                    eatString(&p, " through ");
                    int x2 = eatInteger(&p);
                    eatChar(&p, ',');
                    int y2 = eatInteger(&p);
                    eatChar(&p, '\n');

                    ASSERT(x1 <= x2);
                    ASSERT(y1 <= y1);
                    switch (command)
                    {
                    case Command_Toggle:
                        toggle(x1, y1, x2, y2);
                        break;
                    case Command_TurnOn:
                        turnOn(x1, y1, x2, y2);
                        break;
                    case Command_TurnOff:
                        turnOff(x1, y1, x2, y2);
                        break;
                    default:
                        ASSERT(0);
                    }
                }
            }
            else
            {
                ASSERT(0);
            }
        }
        else
        {
            ASSERT(0);
        }
    }
    else
    {
        ASSERT(0);
    }

    printf("%d\n", countLitLights());

    return 0;
}
