#include <stdint.h>
#include <stdio.h>

typedef int32_t s32;
typedef int32_t b32;

#define internal static

#define Assert(Value) if(!(Value)) { *(int *)0 = 0; }
#define InvalidCodePath Assert(0)
#define InvalidDefaultCase default: { InvalidCodePath; } break;

internal b32
StringsAreEqual(char *A, char *B)
{
    b32 Result;
    while(true)
    {
        if(*A == *B)
        {
            if(*A)
            {
                ++A;
                ++B;
            }
            else
            {
                Result = true;
                break;
            }
        }
        else
        {
            Result = false;
            break;
        }
    }
    return(Result);
}

internal void
FindBathroomCode(char *Instructions, char *OutputBuffer, s32 OutputBufferSize)
{
    char Keypad[3][3] =
    {
        {'1','2','3'},
        {'4','5','6'},
        {'7','8','9'},
    };
    s32 X = 1;
    s32 Y = 1;
    s32 CodeSize = 0;
    char *At = Instructions;
    while(true)
    {
        if(*At == '\n' || *At == 0)
        {
            Assert(CodeSize < OutputBufferSize);
            OutputBuffer[CodeSize++] = Keypad[Y][X];
            if(*At == 0)
            {
                break;
            }
        }
        else
        {
            switch(*At)
            {
                case 'L':
                {
                    --X;
                    if(X < 0)
                    {
                        X = 0;
                    }
                } break;

                case 'R':
                {
                    ++X;
                    if(X > 2)
                    {
                        X = 2;
                    }
                } break;

                case 'U':
                {
                    --Y;
                    if(Y < 0)
                    {
                        Y = 0;
                    }
                } break;

                case 'D':
                {
                    ++Y;
                    if(Y > 2)
                    {
                        Y = 2;
                    }
                } break;

                InvalidDefaultCase;
            }
        }

        ++At;
    }
    Assert(CodeSize < OutputBufferSize);
    OutputBuffer[CodeSize] = 0;
}

internal void
FindBathroomCode2(char *Instructions, char *OutputBuffer, s32 OutputBufferSize)
{
    char Keypad[7][7] =
    {
        {'0','0','0','0','0','0','0'},
        {'0','0','0','1','0','0','0'},
        {'0','0','2','3','4','0','0'},
        {'0','5','6','7','8','9','0'},
        {'0','0','A','B','C','0','0'},
        {'0','0','0','D','0','0','0'},
        {'0','0','0','0','0','0','0'},
    };
    s32 X = 1;
    s32 Y = 3;
    s32 CodeSize = 0;
    char *At = Instructions;
    while(true)
    {
        if(*At == '\n' || *At == 0)
        {
            Assert(CodeSize < OutputBufferSize);
            OutputBuffer[CodeSize++] = Keypad[Y][X];
            if(*At == 0)
            {
                break;
            }
        }
        else
        {
            s32 NewX = X;
            s32 NewY = Y;
            switch(*At)
            {
                case 'L':
                {
                    --NewX;
                } break;

                case 'R':
                {
                    ++NewX;
                } break;

                case 'U':
                {
                    --NewY;
                } break;

                case 'D':
                {
                    ++NewY;
                } break;

                InvalidDefaultCase;
            }
            if(Keypad[NewY][NewX] != '0')
            {
                X = NewX;
                Y = NewY;
            }
        }

        ++At;
    }
    Assert(CodeSize < OutputBufferSize);
    OutputBuffer[CodeSize] = 0;
}

int main(void)
{
    char *TestInput = "ULL\nRRDDD\nLURDL\nUUUUD";
    char *Input =
        "DUURRDRRURUUUDLRUDDLLLURULRRLDULDRDUULULLUUUDRDUDDURRULDRDDDUDDURLDLLDDRRURRUUUDDRUDDLLDDDURLRDDDULRDUDDRDRLRDUULDLDRDLUDDDLRDRLDLUUUDLRDLRUUUDDLUURRLLLUUUUDDLDRRDRDRLDRLUUDUDLDRUDDUDLLUUURUUDLULRDRULURURDLDLLDLLDUDLDRDULLDUDDURRDDLLRLLLLDLDRLDDUULRDRURUDRRRDDDUULRULDDLRLLLLRLLLLRLURRRLRLRDLULRRLDRULDRRLRURDDLDDRLRDLDRLULLRRUDUURRULLLRLRLRRUDLRDDLLRRUDUDUURRRDRDLDRUDLDRDLUUULDLRLLDRULRULLRLRDRRLRLULLRURUULRLLRRRDRLULUDDUUULDULDUDDDUDLRLLRDRDLUDLRLRRDDDURUUUDULDLDDLDRDDDLURLDRLDURUDRURDDDDDDULLDLDLU\n"
        "LURLRUURDDLDDDLDDLULRLUUUDRDUUDDUDLDLDDLLUDURDRDRULULLRLDDUDRRDRUDLRLDDDURDUURLUURRLLDRURDRLDURUDLRLLDDLLRDRRLURLRRUULLLDRLULURULRRDLLLDLDLRDRRURUUUDUDRUULDLUDLURLRDRRLDRUDRUDURLDLDDRUULDURDUURLLUDRUUUUUURRLRULUDRDUDRLLDUDUDUULURUURURULLUUURDRLDDRLUURDLRULDRRRRLRULRDLURRUULURDRRLDLRUURUDRRRDRURRLDDURLUDLDRRLDRLLLLRDUDLULUDRLLLDULUDUULLULLRLURURURDRRDRUURDULRDDLRULLLLLLDLLURLRLLRDLLRLUDLRUDDRLLLDDUDRLDLRLDUDU\n"
        "RRDDLDLRRUULRDLLURLRURDLUURLLLUUDDULLDRURDUDRLRDRDDUUUULDLUDDLRDULDDRDDDDDLRRDDDRUULDLUDUDRRLUUDDRUDLUUDUDLUDURDURDLLLLDUUUUURUUURDURUUUUDDURULLDDLDLDLULUDRULULULLLDRLRRLLDLURULRDLULRLDRRLDDLULDDRDDRURLDLUULULRDRDRDRRLLLURLLDUUUDRRUUURDLLLRUUDDDULRDRRUUDDUUUDLRRURUDDLUDDDUDLRUDRRDLLLURRRURDRLLULDUULLURRULDLURRUURURRLRDULRLULUDUULRRULLLDDDDURLRRRDUDULLRRDURUURUUULUDLDULLUURDRDRRDURDLUDLULRULRLLURULDRUURRRRDUDULLLLLRRLRUDDUDLLURLRDDLLDLLLDDUDDDDRDURRL\n"
        "LLRURUDUULRURRUDURRDLUUUDDDDURUUDLLDLRULRUUDUURRLRRUDLLUDLDURURRDDLLRUDDUDLDUUDDLUUULUUURRURDDLUDDLULRRRUURLDLURDULULRULRLDUDLLLLDLLLLRLDLRLDLUULLDDLDRRRURDDRRDURUURLRLRDUDLLURRLDUULDRURDRRURDDDDUUUDDRDLLDDUDURDLUUDRLRDUDLLDDDDDRRDRDUULDDLLDLRUDULLRRLLDUDRRLRURRRRLRDUDDRRDDUUUDLULLRRRDDRUUUDUUURUULUDURUDLDRDRLDLRLLRLRDRDRULRURLDDULRURLRLDUURLDDLUDRLRUDDURLUDLLULDLDDULDUDDDUDRLRDRUUURDUULLDULUUULLLDLRULDULUDLRRURDLULUDUDLDDRDRUUULDLRURLRUURDLULUDLULLRD\n"
        "UURUDRRDDLRRRLULLDDDRRLDUDLRRULUUDULLDUDURRDLDRRRDLRDUUUDRDRRLLDULRLUDUUULRULULRUDURDRDDLDRULULULLDURULDRUDDDURLLDUDUUUULRUULURDDDUUUURDLDUUURUDDLDRDLLUDDDDULRDLRUDRLRUDDURDLDRLLLLRLULRDDUDLLDRURDDUDRRLRRDLDDUDRRLDLUURLRLLRRRDRLRLLLLLLURULUURRDDRRLRLRUURDLULRUUDRRRLRLRULLLLUDRULLRDDRDDLDLDRRRURLURDDURRLUDDULRRDULRURRRURLUURDDDUDLDUURRRLUDUULULURLRDDRULDLRLLUULRLLRLUUURUUDUURULRRRUULUULRULDDURLDRRULLRDURRDDDLLUDLDRRRRUULDDD";

    char Code[32];

    FindBathroomCode(TestInput, Code, sizeof(Code));
    Assert(StringsAreEqual(Code, "1985"));

    FindBathroomCode2(TestInput, Code, sizeof(Code));
    Assert(StringsAreEqual(Code, "5DB3"));

    FindBathroomCode(Input, Code, sizeof(Code));
    printf("%s\n", Code);

    FindBathroomCode2(Input, Code, sizeof(Code));
    printf("%s\n", Code);

    return(0);
}
