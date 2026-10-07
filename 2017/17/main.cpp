#include <stdint.h>
#include <stdio.h>

typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t b32;

#define internal static
#define local_persist static

internal void
ReadEntireFileAndNullTerminate(char *FileName, char *Buffer, u32 BufferSize)
{
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        size_t ByteCount = fread(Buffer, 1, BufferSize - 1, File);
        Buffer[ByteCount] = 0;
        fclose(File);
    }
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

internal u32
GetU32(char *Input)
{
    u32 Result = 0;
    for(char *At = Input;
        IsDigit(*At);
        ++At)
    {
        Result = 10*Result + (*At - '0');
    }
    return(Result);
}

int
main(int ArgCount, char **Args)
{
    local_persist char Input[64];
    ReadEntireFileAndNullTerminate((ArgCount > 1) ? Args[1] : "input.txt", Input, sizeof(Input));
    u32 StepsPerInsert = GetU32(Input);

    local_persist u16 Buffer[2018];
    Buffer[0] = 0;
    u32 CurrentIndex = 0;
    for(u16 Value = 1;
        Value < 2018;
        ++Value)
    {
        CurrentIndex = ((CurrentIndex + StepsPerInsert) % Value) + 1;
        for(u32 Index = Value - 1;
            Index >= CurrentIndex;
            --Index)
        {
            Buffer[Index + 1] = Buffer[Index];
        }
        Buffer[CurrentIndex] = Value;
    }
    printf("%u\n", Buffer[CurrentIndex + 1]);
    return(0);
}
