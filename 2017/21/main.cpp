#include "main.h"

inline void
Copy(u32 Size, void *SrcInit, void *DstInit)
{
    u8 *Src = (u8 *)SrcInit;
    u8 *Dst = (u8 *)DstInit;
    for(u32 ByteIndex = 0;
        ByteIndex < Size;
        ++ByteIndex)
    {
        Dst[ByteIndex] = Src[ByteIndex];
    }
}

internal void
ReadEntireFileAndNullTerminate(char *FileName, u32 BufferSize, char *Buffer)
{
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    size_t ReadCount = fread(Buffer, 1, BufferSize - 1, File);
    Assert(ReadCount < BufferSize);
    Buffer[ReadCount] = 0;
    fclose(File);
}

internal u16
HashSquare(u32 PixelsPerSquareSide, u32 Pitch, u8 *PixelsStart)
{
    u16 Result = 0;
    u8 *Row = PixelsStart;
    u32 BitIndex = 0;
    for(u32 Y = 0;
        Y < PixelsPerSquareSide;
        ++Y)
    {
        u8 *Pixels = Row;
        for(u32 X = 0;
            X < PixelsPerSquareSide;
            ++X)
        {
            Result |= (*Pixels++ << BitIndex++);
        }
        Row += Pitch;
    }
    if(PixelsPerSquareSide == 2)
    {
        Result |= (1 << 16);
    }
    else
    {
        Assert(PixelsPerSquareSide == 3);
    }
    return(Result);
}

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') || (C == '\t') || (C == '\r') || (C == '\n'));
    return(Result);
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

inline void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

inline void
SkipChar(parser *Parser, char C)
{
    Assert(Parser->At[0] == C);
    ++Parser->At;
}

inline void
SkipString(parser *Parser, char *String)
{
    for(char *At = String;
        At[0];
        ++At)
    {
        SkipChar(Parser, At[0]);
    }
}

internal parsed_pattern
ParsePattern(parser *Parser)
{
    parsed_pattern Result = {};
    u32 PixelIndex = 0;
    while(Parser->At[0] && !IsWhitespace(Parser->At[0]))
    {
        switch(Parser->At[0])
        {
            case '.':
            {
                Assert(PixelIndex < ArrayCount(Result.Pattern.Pixels));
                Result.Pattern.Pixels[PixelIndex++] = 0;
            } break;

            case '#':
            {
                Assert(PixelIndex < ArrayCount(Result.Pattern.Pixels));
                Result.Pattern.Pixels[PixelIndex++] = 1;
            } break;

            case '/':
            {
                if(Result.PixelsPerSide == 0)
                {
                    Result.PixelsPerSide = PixelIndex;
                }
            } break;

            InvalidDefaultCase;
        }
        ++Parser->At;
    }
    return(Result);
}

#if 0
internal void
SetConversion(conversion_table *Table, parsed_pattern PatternFrom, parsed_pattern PatternTo)
{
    u16 HashIndex = HashSquare(PatternFrom.PixelsPerSide, PatternFrom.PixelsPerSide, PatternFrom.Pattern.Pixels);
    Assert(HashIndex < ArrayCount(Table->Patterns));
    Table->Patterns[HashIndex] = PatternTo.Pattern;
}
#endif

internal parsed_pattern
FlipX(parsed_pattern Pattern)
{
    parsed_pattern Result = {};
    Result.PixelsPerSide = Pattern.PixelsPerSide;
    u8 *SrcRow = Pattern.Pattern.Pixels;
    u8 *DstRow = Result.Pattern.Pixels;
    for(u32 Y = 0;
        Y < Pattern.PixelsPerSide;
        ++Y)
    {
        u8 *Src = SrcRow;
        u8 *Dst = DstRow + Pattern.PixelsPerSide - 1;
        for(u32 X = 0;
            X < Pattern.PixelsPerSide;
            ++X)
        {
            *Dst-- = *Src++;
        }
        SrcRow += Pattern.PixelsPerSide;
        DstRow += Pattern.PixelsPerSide;
    }
    return(Result);
}

internal parsed_pattern
FlipY(parsed_pattern Pattern)
{
    parsed_pattern Result = {};
    Result.PixelsPerSide = Pattern.PixelsPerSide;
    u8 *SrcRow = Pattern.Pattern.Pixels;
    u8 *DstRow = Result.Pattern.Pixels + Pattern.PixelsPerSide*(Pattern.PixelsPerSide - 1);
    for(u32 Y = 0;
        Y < Pattern.PixelsPerSide;
        ++Y)
    {
        u8 *Src = SrcRow;
        u8 *Dst = DstRow;
        for(u32 X = 0;
            X < Pattern.PixelsPerSide;
            ++X)
        {
            *Dst++ = *Src++;
        }
        SrcRow += Pattern.PixelsPerSide;
        DstRow -= Pattern.PixelsPerSide;
    }
    return(Result);
}

inline u8
GetPixel(parsed_pattern *Pattern, u32 X, u32 Y)
{
    u8 Result = Pattern->Pattern.Pixels[Y*Pattern->PixelsPerSide + X];
    return(Result);
}

inline void
SetPixel(parsed_pattern *Pattern, u32 X, u32 Y, u8 Pixel)
{
    Pattern->Pattern.Pixels[Y*Pattern->PixelsPerSide + X] = Pixel;
}

internal parsed_pattern
Rotate90(parsed_pattern Pattern)
{
    parsed_pattern Result = {};
    Result.PixelsPerSide = Pattern.PixelsPerSide;
    for(u32 Y = 0;
        Y < Pattern.PixelsPerSide;
        ++Y)
    {
        for(u32 X = 0;
            X < Pattern.PixelsPerSide;
            ++X)
        {
            u8 SrcPixel = GetPixel(&Pattern, X, Y);
            SetPixel(&Result, Y, X, SrcPixel);
        }
    }
    return(Result);
}

internal void
FillConversionTable(conversion_table *Table, char *Input)
{
    parser Parser;
    Parser.At = Input;

    Table->ConversionCount = 0;

    SkipWhitespace(&Parser);

    while(Parser.At[0])
    {
        Assert(Table->ConversionCount < ArrayCount(Table->Conversions));
        conversion *Conversion = Table->Conversions + Table->ConversionCount++;

        parsed_pattern PatternFrom = ParsePattern(&Parser);
        SkipString(&Parser, " => ");
        parsed_pattern PatternTo = ParsePattern(&Parser);

        Conversion->From = PatternFrom;
        Conversion->To = PatternTo;

#if 0
        SetConversion(Table, PatternFrom, PatternTo);
        PatternFrom = FlipX(PatternFrom);
        SetConversion(Table, PatternFrom, PatternTo);
        PatternFrom = FlipY(PatternFrom);
        SetConversion(Table, PatternFrom, PatternTo);
        PatternFrom = FlipX(PatternFrom);
        SetConversion(Table, PatternFrom, PatternTo);
        PatternFrom = Rotate90(PatternFrom);
        SetConversion(Table, PatternFrom, PatternTo);
        PatternFrom = FlipY(PatternFrom);
        SetConversion(Table, PatternFrom, PatternTo);
        PatternFrom = FlipX(PatternFrom);
        SetConversion(Table, PatternFrom, PatternTo);
        PatternFrom = FlipY(PatternFrom);
        SetConversion(Table, PatternFrom, PatternTo);
#endif

        SkipWhitespace(&Parser);
    }
}

internal parsed_pattern
MakePattern(u32 PixelsPerSquareSide, u32 Pitch, u8 *Square)
{
    parsed_pattern Result;
    Result.PixelsPerSide = PixelsPerSquareSide;

    u8 *SrcRow = Square;
    u8 *DstPixel = Result.Pattern.Pixels;

    for(u32 Y = 0;
        Y < PixelsPerSquareSide;
        ++Y)
    {
        u8 *SrcPixel = SrcRow;
        for(u32 X = 0;
            X < PixelsPerSquareSide;
            ++X)
        {
            *DstPixel++ = *SrcPixel++;
        }
        SrcRow += Pitch;
    }

    return(Result);
}

internal u32
GetDerivedPatterns(u32 MaxCount, parsed_pattern *Patterns, parsed_pattern Base)
{
    u32 Count = 0;

    parsed_pattern Pattern = Base;

    Patterns[Count++] = Pattern;
    Pattern = FlipX(Pattern);
    Patterns[Count++] = Pattern;
    Pattern = FlipY(Pattern);
    Patterns[Count++] = Pattern;
    Pattern = FlipX(Pattern);
    Patterns[Count++] = Pattern;
    Pattern = Rotate90(Pattern);
    Patterns[Count++] = Pattern;
    Pattern = FlipY(Pattern);
    Patterns[Count++] = Pattern;
    Pattern = FlipX(Pattern);
    Patterns[Count++] = Pattern;
    Pattern = FlipY(Pattern);
    Patterns[Count++] = Pattern;

    Assert(Count <= MaxCount);

    return(Count);
}

internal b32
PatternsAreEqual(parsed_pattern A, parsed_pattern B)
{
    b32 Result = false;
    if(A.PixelsPerSide == B.PixelsPerSide)
    {
        Result = true;
        u32 PixelCount = A.PixelsPerSide*A.PixelsPerSide;
        for(u32 PixelIndex = 0;
            PixelIndex < PixelCount;
            ++PixelIndex)
        {
            if(A.Pattern.Pixels[PixelIndex] != B.Pattern.Pixels[PixelIndex])
            {
                Result = false;
                break;
            }
        }
    }
    return(Result);
}

internal parsed_pattern
FindConversion(conversion_table *Table, parsed_pattern PatternFrom)
{
    parsed_pattern Result = {};
    u32 Found = false;
    for(u32 ConversionIndex = 0;
        (ConversionIndex < Table->ConversionCount) && !Found;
        ++ConversionIndex)
    {
        conversion *Conversion = Table->Conversions + ConversionIndex;

        parsed_pattern DerivedPatterns[8];
        u32 DerivedPatternCount = GetDerivedPatterns(ArrayCount(DerivedPatterns), DerivedPatterns, PatternFrom);

        for(u32 PatternIndex = 0;
            PatternIndex < DerivedPatternCount;
            ++PatternIndex)
        {
            Assert(PatternIndex < ArrayCount(DerivedPatterns));
            parsed_pattern Pattern = DerivedPatterns[PatternIndex];
            if(PatternsAreEqual(Conversion->From, Pattern))
            {
                Found = true;
                Result = Conversion->To;
                break;
            }
        }
    }
    Assert(Found);
    return(Result);
}

internal void
ConvertSquare(conversion_table *Table,
              u32 SrcPixelsPerSquareSide, u32 SrcPitch, u8 *SrcSquare,
              u32 DstPixelsPerSquareSide, u32 DstPitch, u8 *DstSquare)
{
#if 0
    u16 SrcHash = HashSquare(SrcPixelsPerSquareSide, SrcPitch, SrcSquare);
    Assert(SrcHash < ArrayCount(Table->Patterns));
    pattern *Pattern = Table->Patterns + SrcHash;
#else
    parsed_pattern PatternFrom = MakePattern(SrcPixelsPerSquareSide, SrcPitch, SrcSquare);
    parsed_pattern PatternTo = FindConversion(Table, PatternFrom);
    pattern *Pattern = &PatternTo.Pattern;
#endif

    u8 *Src = Pattern->Pixels;
    u8 *DstRow = DstSquare;

    Assert((DstPixelsPerSquareSide*DstPixelsPerSquareSide) <= ArrayCount(Pattern->Pixels));

    for(u32 Y = 0;
        Y < DstPixelsPerSquareSide;
        ++Y)
    {
        u8 *DstPixels = DstRow;
        for(u32 X = 0;
            X < DstPixelsPerSquareSide;
            ++X)
        {
            *DstPixels++ = *Src++;
        }
        DstRow += DstPitch;
    }
}

internal void
DoArtGenerationStep(grid *Src, grid *Dst, conversion_table *ConversionTable)
{
    u32 SrcPixelsPerSquareSide;

    if((Src->PixelsPerSide % 2) == 0)
    {
        SrcPixelsPerSquareSide = 2;
    }
    else
    {
        Assert((Src->PixelsPerSide % 3) == 0);
        SrcPixelsPerSquareSide = 3;
    }

    u32 DstPixelsPerSquareSide = SrcPixelsPerSquareSide + 1;
    u32 SquaresPerSide = Src->PixelsPerSide / SrcPixelsPerSquareSide;
    Dst->PixelsPerSide = SquaresPerSide * DstPixelsPerSquareSide;

    Assert((Dst->PixelsPerSide * Dst->PixelsPerSide) <= ArrayCount(Dst->Pixels));

    for(u32 SquareY = 0;
        SquareY < SquaresPerSide;
        ++SquareY)
    {
        for(u32 SquareX = 0;
            SquareX < SquaresPerSide;
            ++SquareX)
        {
            u8 *SrcSquare = (Src->Pixels +
                             SquareY*Src->PixelsPerSide*SrcPixelsPerSquareSide +
                             SquareX*SrcPixelsPerSquareSide);
            u8 *DstSquare = (Dst->Pixels +
                             SquareY*Dst->PixelsPerSide*DstPixelsPerSquareSide +
                             SquareX*DstPixelsPerSquareSide);
            ConvertSquare(ConversionTable,
                          SrcPixelsPerSquareSide, Src->PixelsPerSide, SrcSquare,
                          DstPixelsPerSquareSide, Dst->PixelsPerSide, DstSquare);
        }
    }
}

internal void
Print(grid *Grid)
{
    u8 *Pixel = Grid->Pixels;
    for(u32 Y = 0;
        Y < Grid->PixelsPerSide;
        ++Y)
    {
        for(u32 X = 0;
            X < Grid->PixelsPerSide;
            ++X)
        {
            if(*Pixel++)
            {
                printf("#");
            }
            else
            {
                printf(".");
            }
        }
        printf("\n");
    }
    printf("\n");
}

internal void
Print(parsed_pattern *Pattern)
{
    u8 *Pixel = Pattern->Pattern.Pixels;
    for(u32 Y = 0;
        Y < Pattern->PixelsPerSide;
        ++Y)
    {
        for(u32 X = 0;
            X < Pattern->PixelsPerSide;
            ++X)
        {
            if(*Pixel++)
            {
                printf("#");
            }
            else
            {
                printf(".");
            }
        }
        printf("\n");
    }
    printf("\n");
}

internal u32
ParseU32(char *String)
{
    u32 Result = 0;
    while(IsDigit(*String))
    {
        Result = 10*Result + (*String - '0');
        ++String;
    }
    return(Result);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    u32 IterationCount = ((ArgCount > 2) ? ParseU32(Args[2]) : 5);

    local_persist char Input[4*1024];
    ReadEntireFileAndNullTerminate(InputFileName, sizeof(Input), Input);

    local_persist conversion_table ConversionTable;
    FillConversionTable(&ConversionTable, Input);

    u8 StartGrid[] =
    {
        0,1,0,
        0,0,1,
        1,1,1,
    };

#if 0
    {
        parsed_pattern Pattern;
        Pattern.PixelsPerSide = 3;
        Copy(sizeof(StartGrid), StartGrid, Pattern.Pattern.Pixels);

        Print(&Pattern);
        Pattern = FlipX(Pattern);
        Print(&Pattern);
        Pattern = FlipY(Pattern);
        Print(&Pattern);
        Pattern = FlipX(Pattern);
        Print(&Pattern);
        Pattern = Rotate90(Pattern);
        Print(&Pattern);
        Pattern = FlipY(Pattern);
        Print(&Pattern);
        Pattern = FlipX(Pattern);
        Print(&Pattern);
        Pattern = FlipY(Pattern);
        Print(&Pattern);
    }
#endif

    local_persist grid Grids[2];
    Grids[0].PixelsPerSide = 3;
    Copy(sizeof(StartGrid), StartGrid, Grids[0].Pixels);
    Grids[1].PixelsPerSide = 0;

    grid *Src = Grids + 0;
    grid *Dst = Grids + 1;

    //Print(Src);

    for(u32 Iteration = 0;
        Iteration < IterationCount;
        ++Iteration)
    {
        DoArtGenerationStep(Src, Dst, &ConversionTable);
        grid *Tmp = Src;
        Src = Dst;
        Dst = Tmp;
        //Print(Src);
    }

    u32 OnPixelCount = 0;
    u8 *Pixel = Src->Pixels;

    for(u32 Y = 0;
        Y < Src->PixelsPerSide;
        ++Y)
    {
        for(u32 X = 0;
            X < Src->PixelsPerSide;
            ++X)
        {
            if(*Pixel++)
            {
                ++OnPixelCount;
            }
        }
    }

    printf("%u\n", OnPixelCount);
    // 180 - too high

    return(0);
}
