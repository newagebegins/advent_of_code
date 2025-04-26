#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

char* readFileIntoString(const char* filePath)
{
    char* buffer = 0;
    FILE* file;
    fopen_s(&file, filePath, "r");
    if (file)
    {
        if (fseek(file, 0, SEEK_END) == 0)
        {
            long fileSize = ftell(file);
            if (fileSize >= 0)
            {
                if (fseek(file, 0, SEEK_SET) == 0)
                {
                    buffer = malloc(fileSize + 1);
                    if (buffer)
                    {
                        if (fread(buffer, 1, fileSize, file) == fileSize)
                        {
                            buffer[fileSize] = 0;
                        }
                        else
                        {
                            fprintf(stderr, "fread() failed\n");
                        }
                    }
                    else
                    {
                        fprintf(stderr, "malloc() failed\n");
                    }
                }
                else
                {
                    fprintf(stderr, "fseek() failed\n");
                }
            }
            else
            {
                fprintf(stderr, "ftell() failed\n");
            }
        }
        else
        {
            fprintf(stderr, "fseek() failed\n");
        }
    }
    else
    {
        fprintf(stderr, "fopen_s() failed\n");
    }
    return buffer;
}

#define SIZE 256
static char presents[SIZE][SIZE];

static int countHousesWithPresents(const char* directions)
{
    for (int y = 0; y < SIZE; ++y)
    {
        for (int x = 0; x < SIZE; ++x)
        {
            presents[y][x] = 0;
        }
    }

    int x = SIZE / 2;
    int y = SIZE / 2;

    presents[x][y] = 1;
    int result = 1;

    for (const char* d = directions; *d; ++d)
    {
        switch (*d)
        {
        case '>': ++x; break;
        case '<': --x; break;
        case 'v': ++y; break;
        case '^': --y; break;
        default: assert(0);
        }

        assert(0 <= x && x < SIZE);
        assert(0 <= y && y < SIZE);

        if (presents[y][x] == 0)
        {
            ++result;
        }

        ++presents[y][x];
    }

    return result;
}

typedef struct
{
    int x;
    int y;
} Position;

static int countHousesWithPresentsWithRoboSanta(const char* directions)
{
    for (int y = 0; y < SIZE; ++y)
    {
        for (int x = 0; x < SIZE; ++x)
        {
            presents[y][x] = 0;
        }
    }

    Position positions[2];

    for (int i = 0; i < ARRAY_LENGTH(positions); ++i)
    {
        positions[i].x = SIZE / 2;
        positions[i].y = SIZE / 2;
    }

    presents[SIZE / 2][SIZE / 2] = 2;
    int result = 1;
    int positionIndex = 0;

    for (const char* d = directions; *d; ++d)
    {
        Position* position = &positions[positionIndex];

        switch (*d)
        {
        case '>': ++position->x; break;
        case '<': --position->x; break;
        case 'v': ++position->y; break;
        case '^': --position->y; break;
        default: assert(0);
        }

        assert(0 <= position->x && position->x < SIZE);
        assert(0 <= position->y && position->y < SIZE);

        if (presents[position->y][position->x] == 0)
        {
            ++result;
        }

        ++presents[position->y][position->x];

        positionIndex = (positionIndex + 1) % ARRAY_LENGTH(positions);
    }

    return result;
}

int main()
{
    assert(countHousesWithPresents(">") == 2);
    assert(countHousesWithPresents("^>v<") == 4);
    assert(countHousesWithPresents("^v^v^v^v^v") == 2);

    assert(countHousesWithPresentsWithRoboSanta("^v") == 3);
    assert(countHousesWithPresentsWithRoboSanta("^>v<") == 3);
    assert(countHousesWithPresentsWithRoboSanta("^v^v^v^v^v") == 11);

    char* input = readFileIntoString("input.txt");
    printf("%d\n", countHousesWithPresents(input));
    printf("%d\n", countHousesWithPresentsWithRoboSanta(input));

    return 0;
}
