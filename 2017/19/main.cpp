#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef int32_t s32;
typedef int32_t b32;

#define Assert(Condition) if(!(Condition)) { *(int *)0 = 0; }

static char *
ReadEntireFileAndNullTerminate(char *FileName)
{
    char *Result = 0;
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        if(fseek(File, 0, SEEK_END) == 0)
        {
            long FileSize = ftell(File);
            if(FileSize >= 0)
            {
                if(fseek(File, 0, SEEK_SET) == 0)
                {
                    Result = (char *)malloc(FileSize + 1);
                    if(Result)
                    {
                        size_t ReadCount = fread(Result, 1, FileSize, File);
                        if(ReadCount == FileSize)
                        {
                            Result[FileSize] = 0;
                        }
                        else
                        {
                            free(Result);
                            Result = 0;
                        }
                    }
                }
            }
        }
        fclose(File);
    }
    return(Result);
}

struct map
{
    char *Text;
    s32 Width;
};

static map
MakeMap(char *Input)
{
    map Map = {};
    Map.Text = Input;

    char *At = Input;
    while(*At && (*At != '\n'))
    {
        ++At;
    }
    Assert(*At == '\n');

    Map.Width = (s32)(At - Map.Text) + 1;

    return(Map);
}

inline char
GetCell(map Map, s32 X, s32 Y)
{
    char Result = Map.Text[Y*Map.Width + X];
    return(Result);
}

inline b32
PathExists(map Map, s32 X, s32 Y)
{
    char Cell = GetCell(Map, X, Y);
    b32 Result = (Cell != ' ');
    return(Result);
}

static void
SolvePart1(char *Input)
{
    map Map = MakeMap(Input);

    s32 X = 0;
    s32 Y = 0;
    s32 DirX = 0;
    s32 DirY = 1;

    for(;
        X < Map.Width;
        ++X)
    {
        if(GetCell(Map, X, Y) == '|')
        {
            break;
        }
    }

    b32 Completed = false;

    while(!Completed)
    {
        char Cell = GetCell(Map, X, Y);

        switch(Cell)
        {
            case ' ':
            {
                Completed = true;
            } break;

            case '+':
            {
                // NOTE(slava): Change direction
                if(DirX == 0)
                {
                    DirY = 0;
                    if(PathExists(Map, X + 1, Y))
                    {
                        ++X;
                        DirX = 1;
                    }
                    else
                    {
                        Assert(PathExists(Map, X - 1, Y));
                        --X;
                        DirX = -1;
                    }
                }
                else
                {
                    Assert(DirY == 0);
                    DirX = 0;
                    if(PathExists(Map, X, Y + 1))
                    {
                        ++Y;
                        DirY = 1;
                    }
                    else
                    {
                        Assert(PathExists(Map, X, Y - 1));
                        --Y;
                        DirY = -1;
                    }
                }
            } break;

            default:
            {
                // NOTE(slava): Continue walking in the current direction
                if((Cell >= 'A') && (Cell <= 'Z'))
                {
                    printf("%c", Cell);
                }
                else
                {
                    Assert((Cell == '|') || (Cell == '-'));
                }
                X += DirX;
                Y += DirY;
            } break;
        }
    }
    printf("\n");
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = "input.txt";
    if(ArgCount > 1)
    {
        InputFileName = Args[1];
    }
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        SolvePart1(Input);
        free(Input);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
