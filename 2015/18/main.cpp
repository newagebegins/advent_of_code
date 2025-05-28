#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_COUNT(a) (sizeof(a)/sizeof((a)[0]))

#define LIGHT_ON  '#'
#define LIGHT_OFF '.'

struct Offset
{
    int x;
    int y;
};

static void getNextLights(char* currentLights, char* nextLights, int size)
{
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            int index = y * size + x;
            char currentVal = currentLights[index];

            int onNeighborsCount = 0;
            static Offset neighborOffsets[] = {
                {-1,-1},   {0,-1},   {1,-1},
                {-1, 0}, /*{0, 0},*/ {1, 0},
                {-1, 1},   {0, 1},   {1, 1},
            };
            for (int offsetIndex = 0; offsetIndex < ARRAY_COUNT(neighborOffsets); ++offsetIndex)
            {
                Offset offset = neighborOffsets[offsetIndex];
                int neighborX = x + offset.x;
                int neighborY = y + offset.y;
                char neighbor;
                if (0 <= neighborX && neighborX < size && 0 <= neighborY && neighborY < size)
                {
                    int neighborIndex = neighborY * size + neighborX;
                    neighbor = currentLights[neighborIndex];
                }
                else
                {
                    neighbor = LIGHT_OFF;
                }
                if (neighbor == LIGHT_ON)
                {
                    ++onNeighborsCount;
                }
            }

            char nextVal;
            if (currentVal == LIGHT_ON)
            {
                switch (onNeighborsCount)
                {
                case 2:
                case 3:
                    nextVal = LIGHT_ON;
                    break;
                default:
                    nextVal = LIGHT_OFF;
                    break;
                }
            }
            else
            {
                if (onNeighborsCount == 3)
                {
                    nextVal = LIGHT_ON;
                }
                else
                {
                    nextVal = LIGHT_OFF;
                }
            }
            nextLights[index] = nextVal;
        }
    }
}

static void printLights(const char* lights, int size)
{
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            printf("%c", lights[y * size + x]);
        }
        printf("\n");
    }
}

static char* readEntireFile(const char* filename)
{
    char* contents = nullptr;
    FILE* file;
    fopen_s(&file, filename, "r");
    if (file)
    {
        if (fseek(file, 0, SEEK_END) == 0)
        {
            long fileSize = ftell(file);
            if (fileSize >= 0)
            {
                rewind(file);
                contents = (char*)malloc(fileSize + 1);
                if (contents)
                {
                    if (fread(contents, 1, fileSize, file) == fileSize)
                    {
                        contents[fileSize] = '\0';
                    }
                    else
                    {
                        free(contents);
                        contents = nullptr;
                    }
                }
            }
        }
    }
    return contents;
}

static int getOnLightsCountAfterNSteps(const char* initialLights, int size, int steps)
{
    int onCount = -1;

    char* currentLights = (char*)malloc(size * size);
    char* nextLights = (char*)malloc(size * size);

    if (currentLights && nextLights)
    {
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                int index = y * size + x;
                currentLights[index] = initialLights[index];
            }
        }

#if 0
        printf("Initial state:\n");
        printLights(currentLights, size);
#endif

        for (int i = 0; i < steps; ++i)
        {
            getNextLights(currentLights, nextLights, size);

            char* tmp = currentLights;
            currentLights = nextLights;
            nextLights = tmp;

#if 0
            printf("\nAfter %d step(s):\n", i + 1);
            printLights(currentLights, size);
#endif
        }

        onCount = 0;
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                int index = y * size + x;
                if (currentLights[index] == LIGHT_ON)
                {
                    ++onCount;
                }
            }
        }

        free(currentLights);
        free(nextLights);
    }
    else
    {
        fprintf(stderr, "Failed to allocate memory\n");
    }

    return onCount;
}


static void getNextLights2(char* currentLights, char* nextLights, int size)
{
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            bool isCorner = false;

            Offset corners[] = {
                {0,0}, {size - 1,0},
                {0,size - 1}, {size - 1, size - 1},
            };

            for (int cornerIndex = 0; cornerIndex < ARRAY_COUNT(corners); ++cornerIndex)
            {
                Offset corner = corners[cornerIndex];
                if (x == corner.x && y == corner.y)
                {
                    isCorner = true;
                    break;
                }
            }

            int index = y * size + x;
            char nextVal;

            if (isCorner)
            {
                nextVal = LIGHT_ON;
            }
            else
            {
                char currentVal = currentLights[index];

                int onNeighborsCount = 0;
                static Offset neighborOffsets[] = {
                    {-1,-1},   {0,-1},   {1,-1},
                    {-1, 0}, /*{0, 0},*/ {1, 0},
                    {-1, 1},   {0, 1},   {1, 1},
                };
                for (int offsetIndex = 0; offsetIndex < ARRAY_COUNT(neighborOffsets); ++offsetIndex)
                {
                    Offset offset = neighborOffsets[offsetIndex];
                    int neighborX = x + offset.x;
                    int neighborY = y + offset.y;
                    char neighbor;
                    if (0 <= neighborX && neighborX < size && 0 <= neighborY && neighborY < size)
                    {
                        int neighborIndex = neighborY * size + neighborX;
                        neighbor = currentLights[neighborIndex];
                    }
                    else
                    {
                        neighbor = LIGHT_OFF;
                    }
                    if (neighbor == LIGHT_ON)
                    {
                        ++onNeighborsCount;
                    }
                }

                if (currentVal == LIGHT_ON)
                {
                    switch (onNeighborsCount)
                    {
                    case 2:
                    case 3:
                        nextVal = LIGHT_ON;
                        break;
                    default:
                        nextVal = LIGHT_OFF;
                        break;
                    }
                }
                else
                {
                    if (onNeighborsCount == 3)
                    {
                        nextVal = LIGHT_ON;
                    }
                    else
                    {
                        nextVal = LIGHT_OFF;
                    }
                }
            }

            nextLights[index] = nextVal;
        }
    }
}

static int getOnLightsCountAfterNSteps2(const char* initialLights, int size, int steps)
{
    int onCount = -1;

    char* currentLights = (char*)malloc(size * size);
    char* nextLights = (char*)malloc(size * size);

    if (currentLights && nextLights)
    {
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                int index = y * size + x;
                currentLights[index] = initialLights[index];
            }
        }

        Offset corners[] = {
            {0,0}, {size - 1,0},
            {0,size - 1}, {size-1, size-1},
        };

        for (int cornerIndex = 0; cornerIndex < ARRAY_COUNT(corners); ++cornerIndex)
        {
            Offset corner = corners[cornerIndex];
            int index = corner.y * size + corner.x;
            currentLights[index] = LIGHT_ON;
        }

#if 0
        printf("Initial state:\n");
        printLights(currentLights, size);
#endif

        for (int i = 0; i < steps; ++i)
        {
            getNextLights2(currentLights, nextLights, size);

            char* tmp = currentLights;
            currentLights = nextLights;
            nextLights = tmp;

#if 0
            printf("\nAfter %d step(s):\n", i + 1);
            printLights(currentLights, size);
#endif
        }

        onCount = 0;
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                int index = y * size + x;
                if (currentLights[index] == LIGHT_ON)
                {
                    ++onCount;
                }
            }
        }

        free(currentLights);
        free(nextLights);
    }
    else
    {
        fprintf(stderr, "Failed to allocate memory\n");
    }

    return onCount;
}

int main()
{
    const char* exampleLights =
        ".#.#.#"
        "...##."
        "#....#"
        "..#..."
        "#.#..#"
        "####..";
    int exampleAnswer = getOnLightsCountAfterNSteps(exampleLights, 6, 4);
    assert(exampleAnswer == 4);

    char* input = readEntireFile("input.txt");
    if (input)
    {
        int size = 0;
        while (input[size] != '\n')
        {
            ++size;
        }
        char* lights = (char*)malloc(size * size);
        if (lights)
        {
            for (int y = 0; y < size; ++y)
            {
                for (int x = 0; x < size; ++x)
                {
                    lights[y * size + x] = input[y * (size + 1) + x];
                }
            }

            int answer1 = getOnLightsCountAfterNSteps(lights, size, 100);
            assert(answer1 == 821);
            printf("%d\n", answer1);

            int answer2 = getOnLightsCountAfterNSteps2(lights, size, 100);
            printf("%d\n", answer2);
        }
        else
        {
            fprintf(stderr, "Failed to allocate memory\n");
        }
    }
    else
    {
        fprintf(stderr, "Failed to read the input file\n");
    }

    return 0;
}
