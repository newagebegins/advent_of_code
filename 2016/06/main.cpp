#include <windows.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint32_t b32;

#define internal static
#define Assert(Value) if(!(Value)) { *(int *)0 = 0; }
#define ArrayCount(A) (sizeof(A)/sizeof(A[0]))

struct entire_file
{
    void *Contents;
    u32 ContentsSize;
};

internal entire_file
ReadEntireFile(char *Filename)
{
    entire_file Result;
    BOOL Success;

    HANDLE File = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    Assert(File != INVALID_HANDLE_VALUE);

    LARGE_INTEGER FileSize;
    Success = GetFileSizeEx(File, &FileSize);
    Assert(Success);
    Assert(FileSize.HighPart == 0);
    Result.ContentsSize = FileSize.LowPart;

    Result.Contents = VirtualAlloc(0, Result.ContentsSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    Assert(Result.Contents);

    DWORD BytesRead;
    Success = ReadFile(File, Result.Contents, Result.ContentsSize, &BytesRead, 0);
    Assert(Success);
    Assert(BytesRead == Result.ContentsSize);

    Success = CloseHandle(File);
    Assert(Success);

    return(Result);
}

internal b32
StringsAreEqual(char *A, char *B)
{
    b32 Result = false;
    while(*A == *B)
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
    return(Result);
}

#define ALPHABET_SIZE ('z' - 'a' + 1)

struct letter_counts
{
    u32 Counts[ALPHABET_SIZE];
};

#define MAX_MESSAGE_LENGTH 8

internal void
DecodeMessage(char *Input, char *Output)
{
    letter_counts LetterCounts[MAX_MESSAGE_LENGTH] = {};

    u32 MessageLength = 0;
    for(char *At = Input;
        *At && *At != '\n';
        ++At)
    {
        ++MessageLength;
    }
    Assert(MessageLength > 0);
    Assert(MessageLength <= MAX_MESSAGE_LENGTH);

    {
        u32 Position = 0;
        for(char *At = Input;
            *At;
            ++At)
        {
            if(*At == '\n')
            {
                Position = 0;
            }
            else
            {
                Assert(Position < MessageLength);
                u32 LetterIndex = *At - 'a';
                Assert(LetterIndex < ALPHABET_SIZE);
                LetterCounts[Position++].Counts[LetterIndex]++;
            }
        }
    }

    for(u32 Position = 0;
        Position < MessageLength;
        ++Position)
    {
        letter_counts *Counts = LetterCounts + Position;
        u32 MaxCount = 0;
        u32 MaxLetterIndex = 0;
        for(u32 LetterIndex = 0;
            LetterIndex < ALPHABET_SIZE;
            ++LetterIndex)
        {
            u32 Count = Counts->Counts[LetterIndex];
            if(Count > MaxCount)
            {
                MaxCount = Count;
                MaxLetterIndex = LetterIndex;
            }
        }
        char MaxLetter = (char)('a' + MaxLetterIndex);
        Output[Position] = MaxLetter;
    }
    Output[MessageLength] = 0;
}

internal void
DecodeMessage2(char *Input, char *Output)
{
    letter_counts LetterCounts[MAX_MESSAGE_LENGTH] = {};

    u32 MessageLength = 0;
    for(char *At = Input;
        *At && *At != '\n';
        ++At)
    {
        ++MessageLength;
    }
    Assert(MessageLength > 0);
    Assert(MessageLength <= MAX_MESSAGE_LENGTH);

    {
        u32 Position = 0;
        for(char *At = Input;
            *At;
            ++At)
        {
            if(*At == '\n')
            {
                Position = 0;
            }
            else
            {
                Assert(Position < MessageLength);
                u32 LetterIndex = *At - 'a';
                Assert(LetterIndex < ALPHABET_SIZE);
                LetterCounts[Position++].Counts[LetterIndex]++;
            }
        }
    }

    for(u32 Position = 0;
        Position < MessageLength;
        ++Position)
    {
        letter_counts *Counts = LetterCounts + Position;
        u32 MinCount = INT_MAX;
        u32 MinLetterIndex = 0;
        for(u32 LetterIndex = 0;
            LetterIndex < ALPHABET_SIZE;
            ++LetterIndex)
        {
            u32 Count = Counts->Counts[LetterIndex];
            if(Count && Count < MinCount)
            {
                MinCount = Count;
                MinLetterIndex = LetterIndex;
            }
        }
        char MinLetter = (char)('a' + MinLetterIndex);
        Output[Position] = MinLetter;
    }
    Output[MessageLength] = 0;
}

int main(void)
{
    char *TestInput =
        "eedadn\n"
        "drvtee\n"
        "eandsr\n"
        "raavrd\n"
        "atevrs\n"
        "tsrnev\n"
        "sdttsa\n"
        "rasrtv\n"
        "nssdts\n"
        "ntnada\n"
        "svetve\n"
        "tesnvt\n"
        "vntsnd\n"
        "vrdear\n"
        "dvrsen\n"
        "enarar";

    char Message[MAX_MESSAGE_LENGTH + 1];

    DecodeMessage(TestInput, Message);
    Assert(StringsAreEqual(Message, "easter"));

    DecodeMessage2(TestInput, Message);
    Assert(StringsAreEqual(Message, "advent"));

    entire_file EntireFile = ReadEntireFile("input.txt");
    DecodeMessage((char *)EntireFile.Contents, Message);
    Assert(StringsAreEqual(Message, "cyxeoccr"));

    DecodeMessage2((char *)EntireFile.Contents, Message);
    Assert(StringsAreEqual(Message, "batwpask"));

    //printf("%s\n", Message);

    return(0);
}
