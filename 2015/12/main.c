#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

struct Arena
{
    char* base;
    char* top;
    char* topMax;
};

#define TOKEN_TYPE_OPENING_BRACKET 1 // [
#define TOKEN_TYPE_CLOSING_BRACKET 2 // ]
#define TOKEN_TYPE_OPENING_BRACE   3 // {
#define TOKEN_TYPE_CLOSING_BRACE   4 // }
#define TOKEN_TYPE_COMMA           5 // ,
#define TOKEN_TYPE_COLON           6 // :
#define TOKEN_TYPE_INTEGER         7 // 23
#define TOKEN_TYPE_STRING          8 // "foo"

struct Token
{
    int8_t type;
    int16_t dataIndex;
};

struct TokenizerContext
{
    struct Token tokens[32768];
    int16_t tokensCount;
    int16_t integers[512];
    int16_t integersCount;
    char* strings[32];
    int16_t stringsCount;
    char stringData[128];
    int16_t stringDataSize;
};

#define JSON_NODE_TYPE_ARRAY    1
#define JSON_NODE_TYPE_OBJECT   2
#define JSON_NODE_TYPE_STRING   3
#define JSON_NODE_TYPE_INTEGER  4

struct JsonNode
{
    int8_t type;

    union
    {
        // Array
        struct
        {
            struct JsonNode* nodes;
            int16_t nodesCount;
        };

        // Object
        struct
        {
            struct JsonObjectProperty* properties;
            int16_t propertiesCount;
        };

        int16_t integer;
        char* string;
    };
};

struct JsonObjectProperty
{
    char* name;
    struct JsonNode value;
};

static void debugPrintToken(struct TokenizerContext* context, int tokenIndex)
{
    struct Token token = context->tokens[tokenIndex];

    switch (token.type)
    {
    case TOKEN_TYPE_OPENING_BRACKET:
        printf("[");
        break;
    case TOKEN_TYPE_CLOSING_BRACKET:
        printf("]");
        break;
    case TOKEN_TYPE_OPENING_BRACE:
        printf("{");
        break;
    case TOKEN_TYPE_CLOSING_BRACE:
        printf("}");
        break;
    case TOKEN_TYPE_COMMA:
        printf(",");
        break;
    case TOKEN_TYPE_COLON:
        printf(":");
        break;
    case TOKEN_TYPE_INTEGER:
        printf("%d", context->integers[token.dataIndex]);
        break;
    case TOKEN_TYPE_STRING:
        printf("\"%s\"", context->strings[token.dataIndex]);
        break;
    default:
        assert(0);
    }
}

static void debugPrintTokens(struct TokenizerContext* context)
{
    for (int i = 0; i < context->tokensCount; ++i)
    {
        debugPrintToken(context, i);
    }
}

static char* readEntireFile(char* path)
{
    char* result = NULL;
    FILE* file = fopen(path, "r");
    if (file)
    {
        if (fseek(file, 0, SEEK_END) == 0)
        {
            long fileSize = ftell(file);
            if (fileSize > 0)
            {
                rewind(file);
                char* buffer = malloc(fileSize+1);
                if (buffer)
                {
                    if (fread(buffer, 1, fileSize, file) == fileSize)
                    {
                        buffer[fileSize] = 0;
                        result = buffer;
                    }
                    else
                    {
                        assert(0);
                    }
                }
                else
                {
                    assert(0);
                }
            }
            else
            {
                assert(0);
            }
        }
        else
        {
            assert(0);
        }
        fclose(file);
    }
    else
    {
        assert(0);
    }
    return result;
}

inline bool isDigit(char c)
{
    return '0' <= c && c <= '9';
}

static int16_t extractInteger(char** text)
{
    int16_t result = 0;
    int16_t sign = 1;
    if (**text == '-')
    {
        sign = -1;
        ++(*text);
    }
    while (true)
    {
        char c = **text;
        if (isDigit(c))
        {
            result = result * 10 + (c - '0');
            ++(*text);
        }
        else
        {
            break;
        }
    }
    result *= sign;
    return result;
}

static int extractString(char** jsonDoc, char* buffer, int bufferSize)
{
    assert(**jsonDoc == '"');
    (*jsonDoc)++;
    int size = 0;

    while (**jsonDoc != '"')
    {
        assert(**jsonDoc);
        assert(size < bufferSize);
        buffer[size++] = *(*jsonDoc)++;
    }

    (*jsonDoc)++;
    buffer[size++] = 0;

    return size;
}

static int sumNumbersInText(char* text)
{
    int sum = 0;
    while (*text)
    {
        if (*text == '-')
        {
            if (isDigit(*(text + 1)))
            {
                sum += extractInteger(&text);
            }
            else
            {
                assert(0);
            }
        }
        else if (isDigit(*text))
        {
            sum += extractInteger(&text);
        }
        else
        {
            ++text;
        }
    }
    return sum;
}

static bool stringsEqual(char* s1, char* s2)
{
    while (true)
    {
        if (*s1 == *s2)
        {
            if (*s1 == 0)
            {
                return true;
            }
            else
            {
                ++s1;
                ++s2;
            }
        }
        else
        {
            return false;
        }
    }
}

static void tokenizeJsonDoc(struct TokenizerContext* context, char* jsonDoc)
{
    while (*jsonDoc)
    {
        int8_t type = -1;
        int16_t dataIndex = -1;

        switch (*jsonDoc)
        {
        case '[':
            type = TOKEN_TYPE_OPENING_BRACKET;
            ++jsonDoc;
            break;
        case ']':
            type = TOKEN_TYPE_CLOSING_BRACKET;
            ++jsonDoc;
            break;
        case '{':
            type = TOKEN_TYPE_OPENING_BRACE;
            ++jsonDoc;
            break;
        case '}':
            type = TOKEN_TYPE_CLOSING_BRACE;
            ++jsonDoc;
            break;
        case ',':
            type = TOKEN_TYPE_COMMA;
            ++jsonDoc;
            break;
        case ':':
            type = TOKEN_TYPE_COLON;
            ++jsonDoc;
            break;
        case '-':
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
        {
            type = TOKEN_TYPE_INTEGER;
            int16_t integer = extractInteger(&jsonDoc);
            bool found = false;
            for (int16_t i = 0; i < context->integersCount; ++i)
            {
                if (context->integers[i] == integer)
                {
                    found = true;
                    dataIndex = i;
                    break;
                }
            }
            if (!found)
            {
                dataIndex = context->integersCount++;
                assert(dataIndex < ARRAY_LENGTH(context->integers));
                context->integers[dataIndex] = integer;
            }
            break;
        }
        case '"':
        {
            type = TOKEN_TYPE_STRING;
            char buffer[64];
            int stringSize = extractString(&jsonDoc, buffer, ARRAY_LENGTH(buffer));
            bool found = false;
            for (int16_t i = 0; i < context->stringsCount; ++i)
            {
                if (stringsEqual(buffer, context->strings[i]))
                {
                    found = true;
                    dataIndex = i;
                    break;
                }
            }
            if (!found)
            {
                dataIndex = context->stringsCount++;
                assert(dataIndex < ARRAY_LENGTH(context->strings));
                context->strings[dataIndex] = &context->stringData[context->stringDataSize];

                assert(context->stringDataSize + stringSize <= ARRAY_LENGTH(context->stringData));
                for (int i = 0; i < stringSize; ++i)
                {
                    context->stringData[context->stringDataSize++] = buffer[i];
                }
            }
            break;
        }
        case '\n':
            ++jsonDoc;
            break;
        default:
            assert(0);
        }

        if (type != -1)
        {
            assert(context->tokensCount < ARRAY_LENGTH(context->tokens));
            context->tokens[context->tokensCount++] = (struct Token){ type, dataIndex };
        }
    }
}

static int16_t skipJsonArray(struct TokenizerContext* context, int16_t tokenIndex);
static int16_t skipJsonObject(struct TokenizerContext* context, int16_t tokenIndex);

static int16_t skipJsonNode(struct TokenizerContext* context, int16_t tokenIndex)
{
    switch (context->tokens[tokenIndex].type)
    {
    case TOKEN_TYPE_OPENING_BRACKET:
        tokenIndex = skipJsonArray(context, tokenIndex);
        break;
    case TOKEN_TYPE_OPENING_BRACE:
        tokenIndex = skipJsonObject(context, tokenIndex);
        break;
    case TOKEN_TYPE_STRING:
        ++tokenIndex;
        break;
    case TOKEN_TYPE_INTEGER:
        ++tokenIndex;
        break;
    default:
        assert(0);
    }

    return tokenIndex;
}

static int16_t skipJsonArray(struct TokenizerContext* context, int16_t tokenIndex)
{
    assert(context->tokens[tokenIndex].type == TOKEN_TYPE_OPENING_BRACKET);
    ++tokenIndex;

    bool reachedEndOfArray = false;

    while (!reachedEndOfArray)
    {
        assert(tokenIndex < context->tokensCount);
        struct Token token = context->tokens[tokenIndex];

        switch (token.type)
        {
        case TOKEN_TYPE_CLOSING_BRACKET:
            ++tokenIndex;
            reachedEndOfArray = true;
            break;
        case TOKEN_TYPE_COMMA:
            ++tokenIndex;
            break;
        default:
            tokenIndex = skipJsonNode(context, tokenIndex);
            break;
        }
    }

    return tokenIndex;
}

static int16_t skipJsonObjectProperty(struct TokenizerContext* context, int16_t tokenIndex)
{
    assert(context->tokens[tokenIndex].type == TOKEN_TYPE_STRING);
    ++tokenIndex;

    assert(context->tokens[tokenIndex].type == TOKEN_TYPE_COLON);
    ++tokenIndex;

    return skipJsonNode(context, tokenIndex);
}

static int16_t skipJsonObject(struct TokenizerContext* context, int16_t tokenIndex)
{
    assert(context->tokens[tokenIndex].type == TOKEN_TYPE_OPENING_BRACE);
    ++tokenIndex;

    bool reachedEndOfObject = false;

    while (!reachedEndOfObject)
    {
        assert(tokenIndex < context->tokensCount);
        struct Token token = context->tokens[tokenIndex];

        switch (token.type)
        {
        case TOKEN_TYPE_CLOSING_BRACE:
            ++tokenIndex;
            reachedEndOfObject = true;
            break;
        case TOKEN_TYPE_COMMA:
            ++tokenIndex;
            break;
        case TOKEN_TYPE_STRING:
            tokenIndex = skipJsonObjectProperty(context, tokenIndex);
            break;
        default:
            assert(0);
        }
    }

    return tokenIndex;
}

static int16_t countJsonArrayNodes(struct TokenizerContext* context, int16_t tokenIndex)
{
    assert(context->tokens[tokenIndex].type == TOKEN_TYPE_OPENING_BRACKET);
    ++tokenIndex;

    int16_t nodesCount = 0;
    bool reachedEndOfArray = false;

    while (!reachedEndOfArray)
    {
        assert(tokenIndex < context->tokensCount);
        struct Token token = context->tokens[tokenIndex];

        switch (token.type)
        {
        case TOKEN_TYPE_CLOSING_BRACKET:
            reachedEndOfArray = true;
            break;
        case TOKEN_TYPE_COMMA:
            ++tokenIndex;
            break;
        default:
            ++nodesCount;
            tokenIndex = skipJsonNode(context, tokenIndex);
            break;
        }
    }

    return nodesCount;
}

inline struct Token getToken(struct TokenizerContext* context, int16_t index)
{
    assert(index < context->tokensCount);
    return context->tokens[index];
}

static int16_t countJsonObjectProperties(struct TokenizerContext* context, int16_t tokenIndex)
{
    assert(getToken(context, tokenIndex).type == TOKEN_TYPE_OPENING_BRACE);
    ++tokenIndex;

    int16_t propertiesCount = 0;
    bool reachedEndOfObject = false;

    while (!reachedEndOfObject)
    {
        struct Token token;
        token = getToken(context, tokenIndex);

        if (token.type == TOKEN_TYPE_CLOSING_BRACE)
        {
            reachedEndOfObject = true;
        }
        else
        {
            ++propertiesCount;

            // Skip property name
            assert(token.type == TOKEN_TYPE_STRING);
            ++tokenIndex;

            // Skip colon
            token = getToken(context, tokenIndex);
            assert(token.type == TOKEN_TYPE_COLON);
            ++tokenIndex;

            // Skip property value
            tokenIndex = skipJsonNode(context, tokenIndex);

            // Skip comma
            token = getToken(context, tokenIndex);
            if (token.type == TOKEN_TYPE_COMMA)
            {
                ++tokenIndex;
            }
        }
    }

    return propertiesCount;
}

static struct JsonNode parseJsonInteger(struct TokenizerContext* context, int16_t* tokenIndex)
{
    assert(context->tokens[*tokenIndex].type == TOKEN_TYPE_INTEGER);
    struct Token token = context->tokens[(*tokenIndex)++];
    int16_t integer = context->integers[token.dataIndex];
    struct JsonNode jsonInteger = { JSON_NODE_TYPE_INTEGER, .integer = integer };
    return jsonInteger;
}

static struct JsonNode parseJsonString(struct TokenizerContext* context, int16_t* tokenIndex)
{
    assert(context->tokens[*tokenIndex].type == TOKEN_TYPE_STRING);
    struct Token token = context->tokens[(*tokenIndex)++];
    char* string = context->strings[token.dataIndex];
    struct JsonNode jsonString = { JSON_NODE_TYPE_STRING, .string = string };
    return jsonString;
}

static struct JsonNode parseJsonArray(struct TokenizerContext* context, int16_t* tokenIndex, struct Arena* arena);
static struct JsonNode parseJsonObject(struct TokenizerContext* context, int16_t* tokenIndex, struct Arena* arena);

static struct JsonNode parseJsonNode(struct TokenizerContext* context, int16_t* tokenIndex, struct Arena* arena)
{
    struct JsonNode result;

    switch (context->tokens[*tokenIndex].type)
    {
    case TOKEN_TYPE_OPENING_BRACKET:
        result = parseJsonArray(context, tokenIndex, arena);
        break;
    case TOKEN_TYPE_OPENING_BRACE:
        result = parseJsonObject(context, tokenIndex, arena);
        break;
    case TOKEN_TYPE_STRING:
        result = parseJsonString(context, tokenIndex);
        break;
    case TOKEN_TYPE_INTEGER:
        result = parseJsonInteger(context, tokenIndex);
        break;
    default:
        assert(0);
    }

    return result;
}

static struct JsonObjectProperty parseJsonObjectProperty(struct TokenizerContext* context, int16_t* tokenIndex, struct Arena* arena)
{
    struct JsonObjectProperty property;

    assert(context->tokens[*tokenIndex].type == TOKEN_TYPE_STRING);
    property.name = context->strings[context->tokens[*tokenIndex].dataIndex];
    ++(*tokenIndex);

    assert(context->tokens[*tokenIndex].type == TOKEN_TYPE_COLON);
    ++(*tokenIndex);

    property.value = parseJsonNode(context, tokenIndex, arena);

    return property;
}

static struct JsonNode parseJsonObject(struct TokenizerContext* context, int16_t* tokenIndex, struct Arena* arena)
{
    assert(context->tokens[*tokenIndex].type == TOKEN_TYPE_OPENING_BRACE);

    struct JsonNode objectNode = { JSON_NODE_TYPE_OBJECT };
    objectNode.propertiesCount = countJsonObjectProperties(context, *tokenIndex);
    objectNode.properties = (struct JsonObjectProperty*)arena->top;
    arena->top += objectNode.propertiesCount * sizeof(objectNode.properties[0]);
    assert(arena->top <= arena->topMax);

    ++(*tokenIndex);

    bool reachedEndOfObject = false;
    int propertyIndex = 0;

    while (!reachedEndOfObject)
    {
        assert(*tokenIndex < context->tokensCount);
        struct Token token = context->tokens[*tokenIndex];

        switch (token.type)
        {
        case TOKEN_TYPE_CLOSING_BRACE:
            ++(*tokenIndex);
            reachedEndOfObject = true;
            break;
        case TOKEN_TYPE_COMMA:
            ++(*tokenIndex);
            break;
        case TOKEN_TYPE_STRING:
            objectNode.properties[propertyIndex++] = parseJsonObjectProperty(context, tokenIndex, arena);
            break;
        default:
            assert(0);
        }
    }

    assert(propertyIndex == objectNode.propertiesCount);

    return objectNode;
}

static struct JsonNode parseJsonArray(struct TokenizerContext* context, int16_t* tokenIndex, struct Arena* arena)
{
    assert(context->tokens[*tokenIndex].type == TOKEN_TYPE_OPENING_BRACKET);

    struct JsonNode arrayNode = { JSON_NODE_TYPE_ARRAY };
    arrayNode.nodesCount = countJsonArrayNodes(context, *tokenIndex);
    arrayNode.nodes = (struct JsonNode*)arena->top;
    arena->top += arrayNode.nodesCount * sizeof(arrayNode.nodes[0]);
    assert(arena->top <= arena->topMax);

    ++(*tokenIndex);

    bool reachedEndOfArray = false;
    int childIndex = 0;

    while (!reachedEndOfArray)
    {
        assert(*tokenIndex < context->tokensCount);
        struct Token token = context->tokens[*tokenIndex];

        switch (token.type)
        {
        case TOKEN_TYPE_CLOSING_BRACKET:
            ++(*tokenIndex);
            reachedEndOfArray = true;
            break;
        case TOKEN_TYPE_COMMA:
            ++(*tokenIndex);
            break;
        default:
            arrayNode.nodes[childIndex++] = parseJsonNode(context, tokenIndex, arena);
            break;
        }
    }

    assert(childIndex == arrayNode.nodesCount);

    return arrayNode;
}

static int sumNumbersInJsonNode(struct JsonNode node)
{
    int result = 0;
    switch (node.type)
    {
    case JSON_NODE_TYPE_ARRAY:
        for (int i = 0; i < node.nodesCount; ++i)
        {
            result += sumNumbersInJsonNode(node.nodes[i]);
        }
        break;
    case JSON_NODE_TYPE_OBJECT:
    {
        bool skipObject = false;
        for (int i = 0; i < node.propertiesCount; ++i)
        {
            struct JsonNode propertyValue = node.properties[i].value;
            if (propertyValue.type == JSON_NODE_TYPE_STRING && stringsEqual(propertyValue.string, "red"))
            {
                skipObject = true;
                break;
            }
        }
        if (!skipObject)
        {
            for (int i = 0; i < node.propertiesCount; ++i)
            {
                result += sumNumbersInJsonNode(node.properties[i].value);
            }
        }
        break;
    }
    case JSON_NODE_TYPE_STRING:
        break;
    case JSON_NODE_TYPE_INTEGER:
        result += node.integer;
        break;
    default:
        assert(0);
    }
    return result;
}

int main(void)
{
    assert(sumNumbersInText("[1,2,3]") == 6);
    assert(sumNumbersInText("{\"a\":2,\"b\":4}") == 6);
    assert(sumNumbersInText("[[[3]]]") == 3);
    assert(sumNumbersInText("{\"a\":{\"b\":4},\"c\":-1}") == 3);
    assert(sumNumbersInText("{\"a\":[-1,1]}") == 0);
    assert(sumNumbersInText("[-1,{\"a\":1}]") == 0);
    assert(sumNumbersInText("[]") == 0);
    assert(sumNumbersInText("{}") == 0);

    char* input = readEntireFile("input.txt");
    assert(input);
    int answer1 = sumNumbersInText(input);
    assert(answer1 == 191164);
    printf("%d\n", answer1);

    struct TokenizerContext* tokenizerContext = calloc(1, sizeof(*tokenizerContext));
    assert(tokenizerContext);
    tokenizeJsonDoc(tokenizerContext, input);

#if 0
    debugPrintTokens(tokenizerContext);
#endif

    int arenaSize = 4 * 1024 * 1024;
    struct Arena arena;
    arena.base = malloc(arenaSize);
    assert(arena.base);
    arena.top = arena.base;
    arena.topMax = arena.base + arenaSize;

    int16_t tokenIndex = 0;
    struct JsonNode rootNode = parseJsonNode(tokenizerContext, &tokenIndex, &arena);
    assert(tokenIndex == tokenizerContext->tokensCount);
    int answer2 = sumNumbersInJsonNode(rootNode);
    printf("%d\n", answer2);

    return 0;
}
