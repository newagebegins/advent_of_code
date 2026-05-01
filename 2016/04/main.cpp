#include <windows.h>
#include <stdint.h>

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

inline b32
IsLetter(char C)
{
    b32 Result = ('a' <= C && C <= 'z');
    return(Result);
}

inline b32
IsDigit(char C)
{
    b32 Result = ('0' <= C && C <= '9');
    return(Result);
}

struct str
{
    char *Data;
    u32 Length;
};

inline u32
GetStringLength(char *Str)
{
    u32 Result = 0;
    while(*Str++)
    {
        ++Result;
    }
    return(Result);
}

inline str
ToStr(char *Data)
{
    str Result = { Data, GetStringLength(Data) };
    return(Result);
}

#define SECTOR_LENGTH 3
#define CHECKSUM_LENGTH 5

struct parsed_room
{
    str Name;
    str Sector;
    str Checksum;
};

internal parsed_room
ParseRoom(str Description)
{
    parsed_room Result = {};

    char *At = Description.Data;

    Assert(IsLetter(*At));
    Result.Name.Data = At;
    while(IsLetter(*At) || (*At == '-'))
    {
        ++At;
    }
    Result.Name.Length = (u32)(At - Result.Name.Data);

    Assert(IsDigit(*At));
    Result.Sector.Data = At;
    while(IsDigit(*At))
    {
        ++At;
    }
    Result.Sector.Length = (u32)(At - Result.Sector.Data);
    Assert(Result.Sector.Length == SECTOR_LENGTH);

    Assert(*At == '[');
    ++At;

    Assert(IsLetter(*At));
    Result.Checksum.Data = At;
    while(IsLetter(*At))
    {
        ++At;
    }
    Result.Checksum.Length = (u32)(At - Result.Checksum.Data);
    Assert(Result.Checksum.Length == CHECKSUM_LENGTH);

    Assert(*At == ']');
    ++At;

    Assert((At - Description.Data) == Description.Length);

    return(Result);
}

struct heap_element
{
    char Letter;
    u32 Count;
};

#define ALPHABET_SIZE ('z' - 'a' + 1)

struct heap
{
    // NOTE(slava): Indexing is 1-based for ease of calculation
    heap_element Elements[ALPHABET_SIZE + 1];
    u32 ElementCount;
};

internal void
Initialize(heap *Heap)
{
    Heap->ElementCount = 1;
}

inline b32
IsGreater(heap *Heap, u32 IndexA, u32 IndexB)
{
    Assert(IndexA < Heap->ElementCount);
    Assert(IndexB < Heap->ElementCount);
    heap_element A = Heap->Elements[IndexA];
    heap_element B = Heap->Elements[IndexB];
    b32 Result;
    if(A.Count == B.Count)
    {
        Result = (A.Letter < B.Letter);
    }
    else
    {
        Result = (A.Count > B.Count);
    }
    return(Result);
}

inline void
Swap(heap *Heap, u32 IndexA, u32 IndexB)
{
    heap_element Temp = Heap->Elements[IndexA];
    Heap->Elements[IndexA] = Heap->Elements[IndexB];
    Heap->Elements[IndexB] = Temp;
}

internal void
Insert(heap *Heap, char Letter, u32 Count)
{
    u32 NewIndex = Heap->ElementCount++;
    Assert(NewIndex < ArrayCount(Heap->Elements));
    Heap->Elements[NewIndex] = { Letter, Count };
    u32 ParentIndex;
    while((ParentIndex = NewIndex / 2) &&
          IsGreater(Heap, NewIndex, ParentIndex))
    {
        Swap(Heap, NewIndex, ParentIndex);
        NewIndex = ParentIndex;
    }
}

inline u32
GetGreaterChildIndex(heap *Heap, u32 ParentIndex)
{
    u32 LeftChildIndex = 2*ParentIndex;
    u32 RightChildIndex = 2*ParentIndex + 1;
    u32 Result = LeftChildIndex;
    if((RightChildIndex < Heap->ElementCount) &&
       IsGreater(Heap, RightChildIndex, LeftChildIndex))
    {
        Result = RightChildIndex;
    }
    return(Result);
}

internal char
ExtractMax(heap *Heap)
{
    Assert(Heap->ElementCount > 1);
    char Result = Heap->Elements[1].Letter;
    Heap->Elements[1] = Heap->Elements[--Heap->ElementCount];
    u32 ParentIndex = 1;
    u32 GreaterChildIndex;
    while(((GreaterChildIndex = GetGreaterChildIndex(Heap, ParentIndex)) < Heap->ElementCount) &&
          IsGreater(Heap, GreaterChildIndex, ParentIndex))
    {
        Swap(Heap, GreaterChildIndex, ParentIndex);
        ParentIndex = GreaterChildIndex;
    }
    return(Result);
}

internal b32
IsRealRoom(parsed_room Room)
{
    u8 LetterCounts[128] = {};

    Assert(Room.Name.Length > 0);
    for(u32 CharIndex = 0;
        CharIndex < Room.Name.Length;
        ++CharIndex)
    {
        char C = Room.Name.Data[CharIndex];
        if(C == '-')
        {
            // NOTE(slava): Skip
        }
        else
        {
            Assert(IsLetter(C));
            Assert(LetterCounts[C] < 0xFF);
            ++LetterCounts[C];
        }
    }

    heap Heap;
    Initialize(&Heap);

    for(char Letter = 'a';
        Letter <= 'z';
        ++Letter)
    {
        u32 Count = LetterCounts[Letter];
        if(Count)
        {
            Insert(&Heap, Letter, Count);
        }
    }

    b32 Result = true;
    Assert(Room.Checksum.Length == CHECKSUM_LENGTH);
    for(u32 LetterIndex = 0;
        LetterIndex < Room.Checksum.Length;
        ++LetterIndex)
    {
        char C = Room.Checksum.Data[LetterIndex];
        Assert(IsLetter(C));
        if(ExtractMax(&Heap) == C)
        {
            // NOTE(slava): Do nothing
        }
        else
        {
            Result = false;
            break;
        }
    }

    return(Result);
}

internal b32
IsRealRoom(char *Description)
{
    parsed_room ParsedRoom = ParseRoom(ToStr(Description));
    b32 Result = IsRealRoom(ParsedRoom);
    return(Result);
}

internal u32
StringToU32(str Str)
{
    u32 Result = 0;
    for(u32 Index = 0;
        Index < Str.Length;
        ++Index)
    {
        char C = Str.Data[Index];
        Result = (10*Result) + (C - '0');
    }
    return(Result);
}

inline char
DecryptLetter(char EncryptedLetter, u32 SectorId)
{
    char Result;
    if(EncryptedLetter == '-')
    {
        Result = ' ';
    }
    else
    {
        Result = (((EncryptedLetter - 'a') + SectorId) % ALPHABET_SIZE) + 'a';
    }
    return(Result);
}

internal void
DecryptName(str EncryptedName, u32 SectorId, char *Output, u32 OutputSize)
{
    Assert(EncryptedName.Length < OutputSize);
    for(u32 Index = 0;
        Index < EncryptedName.Length;
        ++Index)
    {
        Output[Index] = DecryptLetter(EncryptedName.Data[Index], SectorId);
    }
    Output[EncryptedName.Length] = 0;
}

inline void
DecryptName(char *EncryptedName, u32 SectorId, char *Output, u32 OutputSize)
{
    DecryptName(ToStr(EncryptedName), SectorId, Output, OutputSize);
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

struct solution
{
    u32 SectorSum;
    u32 StorageSectorId;
};

internal solution
GetSolution(char *Input)
{
    solution Result = {};
    char *At = Input;
    while(*At)
    {
        str RoomDescription = { At, 0 };

        while(*At && (*At != '\n'))
        {
            ++At;
        }

        RoomDescription.Length = (u32)(At - RoomDescription.Data);
        Assert(RoomDescription.Length);

        while(*At && (*At == '\n'))
        {
            ++At;
        }

        parsed_room ParsedRoom = ParseRoom(RoomDescription);
        if(IsRealRoom(ParsedRoom))
        {
            u32 Sector = StringToU32(ParsedRoom.Sector);
            Assert(Result.SectorSum <= (0xFFFFFFFF - Sector));
            Result.SectorSum += Sector;
            
            char DecryptedName[64];
            DecryptName(ParsedRoom.Name, Sector, DecryptedName, sizeof(DecryptedName));
            if(StringsAreEqual(DecryptedName, "northpole object storage "))
            {
                Result.StorageSectorId = Sector;
            }
        }
    }
    return(Result);
}

int main(void)
{
    Assert(IsRealRoom("aaaaa-bbb-z-y-x-123[abxyz]"));
    Assert(IsRealRoom("a-b-c-d-e-f-g-h-987[abcde]"));
    Assert(IsRealRoom("not-a-real-room-404[oarel]"));
    Assert(!IsRealRoom("totally-real-room-200[decoy]"));

    char *TestInput =
        "aaaaa-bbb-z-y-x-123[abxyz]\n"
        "a-b-c-d-e-f-g-h-987[abcde]\n"
        "not-a-real-room-404[oarel]\n"
        "totally-real-room-200[decoy]";
    solution TestSolution = GetSolution(TestInput);
    Assert(TestSolution.SectorSum == 1514);

    char DecryptedName[64];
    DecryptName("qzmt-zixmtkozy-ivhz", 343, DecryptedName, sizeof(DecryptedName));
    Assert(StringsAreEqual(DecryptedName, "very encrypted name"));

    entire_file InputFile = ReadEntireFile("input.txt");
    solution Solution = GetSolution((char *)InputFile.Contents);
    Assert(Solution.SectorSum == 245102);
    Assert(Solution.StorageSectorId == 324);

    return(0);
}
