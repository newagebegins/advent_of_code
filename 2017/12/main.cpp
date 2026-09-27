#include <stdint.h>
#include <stdio.h>

#define ArrayCount(A) (sizeof(A)/sizeof((A)[0]))
#define Assert(C) if(!(C)) { *(int *)0 = 0; }

static void
ReadEntireFileAndNullTerminate(char *Buffer, size_t BufferSize, char *Filename)
{
    FILE *File = fopen(Filename, "rb");
    Assert(File);
    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    Assert(FileSize + 1 <= BufferSize);
    fseek(File, 0, SEEK_SET);
    fread(Buffer, FileSize, 1, File);
    Buffer[FileSize] = 0;
}

#define MAX_PROGRAM_COUNT 2000

typedef uint16_t program_id;

struct connection_list
{
    program_id IDs[8];
    uint8_t Count;
};

static uint32_t
CountProgramsConnectedTo0(connection_list *ConnectionLists)
{
    uint32_t Result = 0;

    static program_id ToVisit[MAX_PROGRAM_COUNT];
    ToVisit[0] = 0;
    uint32_t ToVisitCount = 1;

    static uint8_t Visited[MAX_PROGRAM_COUNT];
    for(program_id ProgramID = 0;
        ProgramID < ArrayCount(Visited);
        ++ProgramID)
    {
        Visited[ProgramID] = false;
    }

    while(ToVisitCount)
    {
        program_id ProgramID = ToVisit[--ToVisitCount];
        if(!Visited[ProgramID])
        {
            Visited[ProgramID] = true;
            ++Result;
            connection_list *ConnectionList = ConnectionLists + ProgramID;
            for(uint8_t IDIndex = 0;
                IDIndex < ConnectionList->Count;
                ++IDIndex)
            {
                program_id ConnectedID = ConnectionList->IDs[IDIndex];
                if(!Visited[ConnectedID])
                {
                    ToVisit[ToVisitCount++] = ConnectedID;
                }
            }
        }
    }

    return(Result);
}

static void
VisitProgramsConnectedTo(connection_list *ConnectionLists, uint8_t *Visited, program_id StartID)
{
    static program_id ToVisit[MAX_PROGRAM_COUNT];
    ToVisit[0] = StartID;
    uint32_t ToVisitCount = 1;

    while(ToVisitCount)
    {
        program_id ProgramID = ToVisit[--ToVisitCount];
        if(!Visited[ProgramID])
        {
            Visited[ProgramID] = true;
            connection_list *ConnectionList = ConnectionLists + ProgramID;
            for(uint8_t IDIndex = 0;
                IDIndex < ConnectionList->Count;
                ++IDIndex)
            {
                program_id ConnectedID = ConnectionList->IDs[IDIndex];
                if(!Visited[ConnectedID])
                {
                    ToVisit[ToVisitCount++] = ConnectedID;
                }
            }
        }
    }
}

static uint32_t
CountConnectedGroups(connection_list *ConnectionLists, uint32_t ProgramCount)
{
    uint32_t Result = 0;

    static uint8_t Visited[MAX_PROGRAM_COUNT];
    for(program_id ProgramID = 0;
        ProgramID < ProgramCount;
        ++ProgramID)
    {
        Visited[ProgramID] = false;
    }

    for(program_id ProgramID = 0;
        ProgramID < ProgramCount;
        ++ProgramID)
    {
        if(!Visited[ProgramID])
        {
            ++Result;
            VisitProgramsConnectedTo(ConnectionLists, Visited, ProgramID);
        }
    }

    return(Result);
}

struct parser
{
    char *At;
};

inline bool
IsDigit(char C)
{
    bool Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

static program_id
ParseProgramID(parser *Parser)
{
    program_id ID = 0;
    while(IsDigit(Parser->At[0]))
    {
        ID = 10*ID + (Parser->At[0] - '0');
        ++Parser->At;
    }
    return(ID);
}

inline void
SkipString(parser *Parser, char *String)
{
    for(char *At = String;
        *At;
        ++At, ++Parser->At)
    {
        Assert(At[0] == Parser->At[0]);
    }
}

inline bool
IsWhitespace(char C)
{
    bool Result;
    switch(C)
    {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        {
            Result = true;
        } break;

        default:
        {
            Result = false;
        } break;
    }
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

static uint32_t
ParseInput(char *Input, connection_list *ConnectionLists)
{
    uint32_t ProgramCount = 0;
    parser Parser;
    Parser.At = Input;
    while(Parser.At[0])
    {
        program_id ID = ParseProgramID(&Parser);
        Assert(ID < MAX_PROGRAM_COUNT);
        ++ProgramCount;
        connection_list *ConnectionList = ConnectionLists + ID;
        ConnectionList->Count = 0;
        SkipString(&Parser, " <-> ");
        for(;;)
        {
            program_id ConnectedID = ParseProgramID(&Parser);
            Assert(ConnectedID < MAX_PROGRAM_COUNT);
            Assert(ConnectionList->Count < ArrayCount(ConnectionList->IDs));
            ConnectionList->IDs[ConnectionList->Count++] = ConnectedID;
            if(Parser.At[0] == ',')
            {
                SkipString(&Parser, ", ");
            }
            else
            {
                SkipWhitespace(&Parser);
                break;
            }
        }
    }
    Assert(ProgramCount <= MAX_PROGRAM_COUNT);
    return(ProgramCount);
}

static void
TestCountProgramsConnectedTo0(char *Filename, uint32_t ExpectedCount0, uint32_t ExpectedGroupCount)
{
    static char Input[40000];
    ReadEntireFileAndNullTerminate(Input, sizeof(Input), Filename);

    static connection_list ConnectionLists[MAX_PROGRAM_COUNT];
    uint32_t ProgramCount = ParseInput(Input, ConnectionLists);

    uint32_t Count0 = CountProgramsConnectedTo0(ConnectionLists);
    Assert(Count0 == ExpectedCount0);

    uint32_t GroupCount = CountConnectedGroups(ConnectionLists, ProgramCount);
    Assert(GroupCount == ExpectedGroupCount);
}

int
main(void)
{
    TestCountProgramsConnectedTo0("test_input.txt", 6, 2);
    TestCountProgramsConnectedTo0("input.txt", 175, 213);

    return(0);
}
