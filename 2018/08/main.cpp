#include "lib.h"
#include "main.h"

struct numbers
{
    u32 Count;
    u32 *Numbers;
};

internal numbers
ParseNumbers(char *Input)
{
    numbers Result = {};
    Result.Count = CountSpaces(Input) + 1;
    Result.Numbers = (u32 *)malloc(Result.Count*sizeof(u32));

    parser Parser = MakeParser(Input);
    u32 NumberIndex = 0;

    while(StillParsing(&Parser))
    {
        Result.Numbers[NumberIndex++] = ParseU32(&Parser);
        SkipWhitespace(&Parser);
    }

    Assert(NumberIndex == Result.Count);

    return(Result);
}

struct sum_result
{
    u32 Sum;
    u32 *Node;
};

internal sum_result
SumMetadataEntries(u32 *Node)
{
    sum_result Result = {};
    Result.Node = Node;
    u32 ChildCount = *Result.Node++;
    u32 MetaCount = *Result.Node++;
    for(u32 ChildIndex = 0;
        ChildIndex < ChildCount;
        ++ChildIndex)
    {
        sum_result ChildResult = SumMetadataEntries(Result.Node);
        Result.Sum += ChildResult.Sum;
        Result.Node = ChildResult.Node;
    }
    for(u32 MetaIndex = 0;
        MetaIndex < MetaCount;
        ++MetaIndex)
    {
        Result.Sum += *Result.Node++;
    }
    return(Result);
}

struct create_node_tree_result
{
    node Node;
    u32 *NumbersAt;
};

internal create_node_tree_result
CreateNodeTree(memory_arena *Arena, u32 *Numbers)
{
    create_node_tree_result Result = {};
    Result.NumbersAt = Numbers;
    node *Node = &Result.Node;
    Node->ChildCount = *Result.NumbersAt++;
    Node->MetaCount = *Result.NumbersAt++;
    if(Node->ChildCount == 0)
    {
        Node->Children = 0;
    }
    else
    {
        Node->Children = PushArray(Arena, Node->ChildCount, node);
        for(u32 ChildIndex = 0;
            ChildIndex < Node->ChildCount;
            ++ChildIndex)
        {
            create_node_tree_result ChildResult = CreateNodeTree(Arena, Result.NumbersAt);
            Node->Children[ChildIndex] = ChildResult.Node;
            Result.NumbersAt = ChildResult.NumbersAt;
        }
    }
    if(Node->MetaCount == 0)
    {
        Node->Meta = 0;
    }
    else
    {
        Node->Meta = PushArray(Arena, Node->MetaCount, u32);
        for(u32 MetaIndex = 0;
            MetaIndex < Node->MetaCount;
            ++MetaIndex)
        {
            Node->Meta[MetaIndex] = *Result.NumbersAt++;
        }
    }
    return(Result);
}

internal u32
GetNodeValue(node Node)
{
    u32 Result = 0;
    if(Node.ChildCount == 0)
    {
        for(u32 MetaIndex = 0;
            MetaIndex < Node.MetaCount;
            ++MetaIndex)
        {
            Result += Node.Meta[MetaIndex];
        }
    }
    else
    {
        for(u32 MetaIndex = 0;
            MetaIndex < Node.MetaCount;
            ++MetaIndex)
        {
            u32 ChildOneBasedIndex = Node.Meta[MetaIndex];
            if((ChildOneBasedIndex != 0) && (ChildOneBasedIndex <= Node.ChildCount))
            {
                Result += GetNodeValue(Node.Children[ChildOneBasedIndex - 1]);
            }
        }
    }
    return(Result);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");
    char *Input = ReadEntireFileAndNullTerminate(InputFileName);
    if(Input)
    {
        memory_arena Arena;
        memory_index ArenaSize = Megabytes(16);
        void *ArenaBase = malloc(ArenaSize);
        InitializeArena(&Arena, ArenaSize, ArenaBase);

        numbers Numbers = ParseNumbers(Input);
        sum_result SumResult = SumMetadataEntries(Numbers.Numbers);
        printf("%u\n", SumResult.Sum);

        create_node_tree_result TreeResult = CreateNodeTree(&Arena, Numbers.Numbers);
        u32 Value = GetNodeValue(TreeResult.Node);
        printf("%u\n", Value);
    }
    else
    {
        fprintf(stderr, "Failed to read %s\n", InputFileName);
    }
    return(0);
}
