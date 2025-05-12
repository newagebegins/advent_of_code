#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KILOBYTES(x) (1024 * x)
#define MEGABYTES(x) (1024 * KILOBYTES(x))

struct Arena
{
    void* base;
    void* top;
    int size;
};

static int countNextDigit(char* input)
{
    int result = 1;
    char digit = *input++;
    while (*input++ == digit)
    {
        ++result;
    }
    return result;
}

static char* lookAndSay(char* input, struct Arena* arena)
{
    char* p = (char*)arena->top;
    char* result = p;
    char digit;
    while ((digit = *input))
    {
        int count = countNextDigit(input);
        assert(count < 10);
        *p++ = '0' + (char)count;
        *p++ = digit;
        input += count;
    }
    *p++ = 0;
    arena->top = p;
    return result;
}

int main(void)
{
    struct Arena arena;
    arena.size = MEGABYTES(100);
    arena.base = malloc(arena.size);
    arena.top = arena.base;

    assert(strcmp(lookAndSay("1", &arena), "11") == 0);
    assert(strcmp(lookAndSay("11", &arena), "21") == 0);
    assert(strcmp(lookAndSay("21", &arena), "1211") == 0);
    assert(strcmp(lookAndSay("1211", &arena), "111221") == 0);
    assert(strcmp(lookAndSay("111221", &arena), "312211") == 0);

    char* input = "1113122113";
    for (int i = 0; i < 50; ++i)
    {
        char* result = lookAndSay(input, &arena);
        //printf("%s -> %s\n", input, result);
        input = result;
    }

    printf("%zu\n", strlen(input));

    return 0;
}
