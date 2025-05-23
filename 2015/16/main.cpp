#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

#define KILOBYTES(x) (1024 * (x))
#define MEGABYTES(x) (1024 * KILOBYTES(x))

struct Arena
{
    char* base;
    char* top;
    int size;
};

static char* alloc(Arena* arena, int size)
{
    char* result = nullptr;
    if (arena->top + size <= arena->base + arena->size)
    {
        result = arena->top;
        arena->top += size;
    }
    else
    {
        fprintf(stderr, "Out of memory\n");
    }
    return result;
}

enum Property
{
    Property_children,
    Property_cats,
    Property_samoyeds,
    Property_pomeranians,
    Property_akitas,
    Property_vizslas,
    Property_goldfish,
    Property_trees,
    Property_cars,
    Property_perfumes,

    Property_count,
    Property_invalid,
};

const char* propertyNames[] = {
    "children",
    "cats",
    "samoyeds",
    "pomeranians",
    "akitas",
    "vizslas",
    "goldfish",
    "trees",
    "cars",
    "perfumes",
};

static_assert(ARRAY_LENGTH(propertyNames) == Property_count, "Property count mismatch");

struct Aunt
{
    int props[Property_count];
};

struct AuntList
{
    Aunt* aunts;
    int count;
};

static bool stringsEqual(const char* s1, const char* s2)
{
    bool result = false;
    while (true)
    {
        if (*s1 == *s2)
        {
            if (*s1 == 0)
            {
                result = true;
                break;
            }
            else
            {
                ++s1;
                ++s2;
            }
        }
        else
        {
            break;
        }
    }
    return result;
}

static Property strToProp(char* str)
{
    Property result = Property_invalid;
    for (int prop = 0; prop < Property_count; ++prop)
    {
        if (stringsEqual(str, propertyNames[prop]))
        {
            result = (Property)prop;
            break;
        }
    }
    return result;
}

static bool isGiftAunt1(Aunt* remembered, Aunt* gift)
{
    bool result = true;
    for (int prop = 0; prop < Property_count; ++prop)
    {
        if ((remembered->props[prop] >= 0) && (remembered->props[prop] != gift->props[prop]))
        {
            result = false;
            break;
        }
    }
    return result;
}

static bool isGiftAunt2(Aunt* remembered, Aunt* gift)
{
    bool result = true;

    static Property greaterThan[] = { Property_cats, Property_trees };
    static Property fewerThan[] = { Property_pomeranians, Property_goldfish };
    static Property other[] = { Property_children, Property_samoyeds, Property_akitas, Property_vizslas, Property_cars, Property_perfumes };

    if (result)
    {
        for (int i = 0; i < ARRAY_LENGTH(greaterThan); ++i)
        {
            Property prop = greaterThan[i];
            if ((remembered->props[prop] >= 0) && (remembered->props[prop] <= gift->props[prop]))
            {
                result = false;
                break;
            }
        }
    }

    if (result)
    {
        for (int i = 0; i < ARRAY_LENGTH(fewerThan); ++i)
        {
            Property prop = fewerThan[i];
            if ((remembered->props[prop] >= 0) && (remembered->props[prop] >= gift->props[prop]))
            {
                result = false;
                break;
            }
        }
    }

    if (result)
    {
        for (int i = 0; i < ARRAY_LENGTH(other); ++i)
        {
            Property prop = other[i];
            if ((remembered->props[prop] >= 0) && (remembered->props[prop] != gift->props[prop]))
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

static int findGiftAunt1(AuntList remembered, Aunt* gift)
{
    int result = -1;
    for (int rememberedIndex = 0; rememberedIndex < remembered.count; ++rememberedIndex)
    {
        if (isGiftAunt1(&remembered.aunts[rememberedIndex], gift))
        {
            result = rememberedIndex;
            break;
        }
    }
    return result;
}

static int findGiftAunt2(AuntList remembered, Aunt* gift)
{
    int result = -1;
    for (int rememberedIndex = 0; rememberedIndex < remembered.count; ++rememberedIndex)
    {
        if (isGiftAunt2(&remembered.aunts[rememberedIndex], gift))
        {
            result = rememberedIndex;
            break;
        }
    }
    return result;
}

struct ParseContext
{
    Arena* arena;
    char* input;
    bool error;
};

static bool isWhitespace(char c)
{
    bool result = false;
    switch (c)
    {
    case ' ':
    case '\t':
    case '\n':
        result = true;
        break;
    }
    return result;
}

static void skipWhitespace(ParseContext* ctx)
{
    if (!ctx->error)
    {
        while (isWhitespace(*(ctx->input)))
        {
            ++ctx->input;
        }
    }
}

static int getWordLength(ParseContext* ctx)
{
    assert(!ctx->error);
    int wordLen = 0;
    while (true)
    {
        char c = ctx->input[wordLen];
        if (isWhitespace(c) || c == 0)
        {
            break;
        }
        else
        {
            ++wordLen;
        }
    }
    return wordLen;
}

static char* extractWord(ParseContext* ctx)
{
    assert(!ctx->error);
    skipWhitespace(ctx);
    int wordLen = getWordLength(ctx);
    char* word = alloc(ctx->arena, wordLen + 1);
    if (word)
    {
        for (int i = 0; i < wordLen; ++i)
        {
            word[i] = *(ctx->input++);
        }
        word[wordLen] = 0;
    }
    return word;
}

static void skipWord(ParseContext* ctx, const char* wordToSkip)
{
    if (!ctx->error)
    {
        char* savedArenaTop = ctx->arena->top;
        char* word = extractWord(ctx);
        if (word)
        {
            if (stringsEqual(word, wordToSkip))
            {
                // success
            }
            else
            {
                ctx->error = true;
                fprintf(stderr, "Expected `%s`, found `%s`\n", wordToSkip, word);
            }
            ctx->arena->top = savedArenaTop;
        }
        else
        {
            ctx->error = true;
        }
    }
}

static bool isDigit(char c)
{
    bool result = false;
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
        result = true;
        break;
    }
    return result;
}

static int extractInt(ParseContext* ctx)
{
    int result = 0;
    if (!ctx->error)
    {
        skipWhitespace(ctx);
        if (isDigit(*(ctx->input)))
        {
            while (isDigit(*(ctx->input)))
            {
                result = result * 10 + (*(ctx->input) - '0');
                ++(ctx->input);
            }
        }
        else
        {
            ctx->error = true;
            fprintf(stderr, "Expected a digit, found: %c\n", *(ctx->input));
        }
    }
    return result;
}

static void skipInt(ParseContext* ctx, int numToSkip)
{
    if (!ctx->error)
    {
        int num = extractInt(ctx);
        if (num == numToSkip)
        {
            // success
        }
        else
        {
            ctx->error = true;
            fprintf(stderr, "Expected `%d`, found: `%d`\n", numToSkip, num);
        }
    }
}

static Property extractProperty(ParseContext* ctx)
{
    Property result = Property_invalid;
    if (!ctx->error)
    {
        skipWhitespace(ctx);

        int propertyLen = 0;
        while (true)
        {
            char c = ctx->input[propertyLen];
            if ('a' <= c && c <= 'z')
            {
                propertyLen++;
            }
            else
            {
                break;
            }
        }

        char* savedArenaTop = ctx->arena->top;
        char* word = alloc(ctx->arena, propertyLen + 1);
        if (word)
        {
            for (int i = 0; i < propertyLen; ++i)
            {
                word[i] = *(ctx->input++);
            }
            word[propertyLen] = 0;
            result = strToProp(word);
            if (result != Property_invalid)
            {
                // success
            }
            else
            {
                ctx->error = true;
                fprintf(stderr, "Expected a property name, found: `%s`\n", word);
            }
            ctx->arena->top = savedArenaTop;
        }
        else
        {
            ctx->error = true;
        }
    }
    return result;
}

static int countAunts(char* input)
{
    int result = 0;
    bool isEmptyLine = true;
    bool end = false;
    while (!end)
    {
        switch (*input)
        {
        case ' ':
        case '\t':
            ++input;
            break;
        case '\n':
        case 0:
            if (!isEmptyLine)
            {
                ++result;
            }
            isEmptyLine = true;
            if (*input == 0)
            {
                end = true;
            }
            else
            {
                ++input;
            }
            break;
        default:
            isEmptyLine = false;
            ++input;
            break;
        }
    }
    return result;
}

static AuntList parseInput(char* input, Arena* arena)
{
    int rememberedCount = countAunts(input);
    char* savedArenaTop = arena->top;
    Aunt* remembered = (Aunt*)alloc(arena, rememberedCount * sizeof(remembered[0]));
    int rememberedIndex = 0;

    if (remembered)
    {
        ParseContext ctx{ arena, input };

        while ((*ctx.input) && (!ctx.error))
        {
            for (int prop = 0; prop < Property_count; ++prop)
            {
                remembered[rememberedIndex].props[prop] = -1;
            }

            skipWord(&ctx, "Sue");
            skipInt(&ctx, rememberedIndex + 1);
            skipWord(&ctx, ":");

            bool endOfProperties = false;
            while (!ctx.error && !endOfProperties)
            {
                Property prop = extractProperty(&ctx);
                skipWord(&ctx, ":");
                int propVal = extractInt(&ctx);
                if (!ctx.error)
                {
                    remembered[rememberedIndex].props[prop] = propVal;
                    if (*ctx.input == ',')
                    {
                        ++ctx.input;
                    }
                    else
                    {
                        endOfProperties = true;
                    }
                }
            }

            skipWhitespace(&ctx);
            ++rememberedIndex;
        }

        if (ctx.error)
        {
            arena->top = savedArenaTop;
            remembered = nullptr;
            fprintf(stderr, "Parsing error for aunt #%d\n", rememberedIndex);
        }
    }

    return { remembered, rememberedCount };
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

int main()
{
    char* input = readEntireFile("input.txt");
    if (input)
    {
        Arena arena;
        arena.size = MEGABYTES(16);
        arena.base = (char*)malloc(arena.size);
        arena.top = arena.base;

        AuntList list = parseInput(input, &arena);
        if (list.aunts)
        {
            Aunt giftAunt;
            giftAunt.props[Property_children] = 3;
            giftAunt.props[Property_cats] = 7;
            giftAunt.props[Property_samoyeds] = 2;
            giftAunt.props[Property_pomeranians] = 3;
            giftAunt.props[Property_akitas] = 0;
            giftAunt.props[Property_vizslas] = 0;
            giftAunt.props[Property_goldfish] = 5;
            giftAunt.props[Property_trees] = 3;
            giftAunt.props[Property_cars] = 2;
            giftAunt.props[Property_perfumes] = 1;

            int found1 = findGiftAunt1(list, &giftAunt);
            assert(found1 + 1 == 103);
            printf("%d\n", found1 + 1);

            int found2 = findGiftAunt2(list, &giftAunt);
            printf("%d\n", found2 + 1);
        }
    }
    else
    {
        printf("Failed to read input.\n");
        return 1;
    }
    return 0;
}
