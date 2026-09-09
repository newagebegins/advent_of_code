#if !defined(LIB_H)
#define LIB_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define global_variable static
#define internal static
#define local_persist static

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef int32_t b32;

typedef float r32;
typedef double r64;

typedef size_t memory_index;

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

internal char *
ReadEntireFileAndNullTerminate(char *FileName)
{
    char *Result = 0;
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        long FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);
        Result = (char *)malloc(FileSize + 1);
        if(Result)
        {
            fread(Result, FileSize, 1, File);
            Result[FileSize] = 0;
        }
        fclose(File);
    }
    return(Result);
}

#endif
