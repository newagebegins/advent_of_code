#if !defined(MAIN_H)
#define MAIN_H

#include <stdint.h>
#include <stdio.h>

#define internal static
#define local_persist static

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t b32;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#define Assert(Condition) if(!(Condition)) { *(int *)0 = 0; }
#define InvalidDefaultCase default: { Assert(!"Invalid default case"); } break

struct parser
{
    char *At;
};

struct pattern
{
    u8 Pixels[16];
};

struct parsed_pattern
{
    u32 PixelsPerSide;
    pattern Pattern;
};

struct conversion
{
    parsed_pattern From;
    parsed_pattern To;
};

struct conversion_table
{
#if 0
    pattern Patterns[16 + 512];
#else    
    u32 ConversionCount;
    conversion Conversions[128];
#endif
};

struct grid
{
    u32 PixelsPerSide;
    u8 Pixels[10000000];
};

#endif
