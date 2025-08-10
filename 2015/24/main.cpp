#include <stdint.h>
#include <stdio.h>
#include <limits.h>

typedef int8_t i8;
typedef uint8_t u8;

typedef int16_t i16;
typedef uint16_t u16;

typedef int32_t i32;
typedef uint32_t u32;

typedef int64_t i64;
typedef uint64_t u64;

typedef i32 b32;

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define Assert(x) if (!(x)) { *(int*)0 = 0; }

#define MaxCount 32

struct set
{
    i32 Items[MaxCount];
    i32 Count;
};

struct subset_iterator
{
    set* Set;
    i32 SubsetCount;
    i32 GroupWeight;
    i32 SubsetIndex;
};

subset_iterator
MakeSubsetIterator(set* Set, i32 GroupWeight)
{
    subset_iterator It;
    It.Set = Set;
    It.SubsetCount = 1 << Set->Count;
    It.GroupWeight = GroupWeight;
    It.SubsetIndex = 0;
    return It;
}

static b32
GetNextSubset(subset_iterator* It, set* Subset)
{
    while (It->SubsetIndex < It->SubsetCount)
    {
        Subset->Count = 0;
        for (i32 BitIndex = 0;
             BitIndex < 32;
             ++BitIndex)
        {
            if ((It->SubsetIndex >> BitIndex) & 1)
            {
                Assert(BitIndex < It->Set->Count);
                Assert(Subset->Count < MaxCount);
                Subset->Items[Subset->Count++] = It->Set->Items[BitIndex];
            }
        }
        i32 Sum = 0;
        for (i32 Index = 0;
             Index < Subset->Count;
             ++Index)
        {
            Sum += Subset->Items[Index];
        }
        ++It->SubsetIndex;
        if (Sum == It->GroupWeight)
        {
            return true;
        }
    }
    return false;
}

set
Subtract(set* S1, set* S2)
{
    set Result;
    Result.Count = 0;
    for (i32 Index1 = 0;
         Index1 < S1->Count;
         ++Index1)
    {
        b32 Keep = true;
        for (i32 Index2 = 0;
             Index2 < S2->Count;
             ++Index2)
        {
            if (S1->Items[Index1] == S2->Items[Index2])
            {
                Keep = false;
                break;
            }
        }
        if (Keep)
        {
            Result.Items[Result.Count++] = S1->Items[Index1];
        }
    }
    return Result;
}

i64
ComputeQE(set* Set)
{
    i64 Result = 1;
    for (i32 Index = 0;
         Index < Set->Count;
         ++Index)
    {
        Result *= Set->Items[Index];
    }
    return Result;
}

struct partition3
{
    set Group1;
    set Group2;
    set Group3;
};

struct partition4
{
    set Group1;
    set Group2;
    set Group3;
    set Group4;
};

partition3
FindBestPartition3(set* Weights)
{
    partition3 Result;
    i32 SumWeights = 0;
    for (i32 I = 0; I < Weights->Count; ++I)
    {
        SumWeights += Weights->Items[I];
    }
    i32 GroupCount = 3;
    Assert(SumWeights % GroupCount == 0);
    i32 GroupWeight = SumWeights / GroupCount;
    subset_iterator It1 = MakeSubsetIterator(Weights, GroupWeight);
    i32 MinGroup1Count = INT_MAX;
    i64 MinQE = LLONG_MAX;
    set Group1;
    while (GetNextSubset(&It1, &Group1))
    {
        if ((Group1.Count < MinGroup1Count) ||
            ((Group1.Count == MinGroup1Count) && (ComputeQE(&Group1) < MinQE)))
        {
            set Weights2 = Subtract(Weights, &Group1);
            subset_iterator It2 = MakeSubsetIterator(&Weights2, GroupWeight);
            set Group2;
            b32 GotGroup2 = GetNextSubset(&It2, &Group2);
            Assert(GotGroup2);
            set Group3 = Subtract(&Weights2, &Group2);
            
            MinGroup1Count = Group1.Count;
            MinQE = ComputeQE(&Group1);
            
            Assert(Group1.Count + Group2.Count + Group3.Count == Weights->Count);
            Result.Group1 = Group1;
            Result.Group2 = Group2;
            Result.Group3 = Group3;
        }
    }
    return Result;
}

partition4
FindBestPartition4(set* Weights)
{
    partition4 Result;
    i32 SumWeights = 0;
    for (i32 I = 0; I < Weights->Count; ++I)
    {
        SumWeights += Weights->Items[I];
    }
    i32 GroupCount = 4;
    Assert(SumWeights % GroupCount == 0);
    i32 GroupWeight = SumWeights / GroupCount;
    subset_iterator It1 = MakeSubsetIterator(Weights, GroupWeight);
    i32 MinGroup1Count = INT_MAX;
    i64 MinQE = LLONG_MAX;
    set Group1;
    while (GetNextSubset(&It1, &Group1))
    {
        if ((Group1.Count < MinGroup1Count) ||
            ((Group1.Count == MinGroup1Count) && (ComputeQE(&Group1) < MinQE)))
        {
            b32 Done = false;
            set Weights2 = Subtract(Weights, &Group1);
            subset_iterator It2 = MakeSubsetIterator(&Weights2, GroupWeight);
            set Group2;
            while (!Done && GetNextSubset(&It2, &Group2))
            {
                set Weights3 = Subtract(&Weights2, &Group2);
                subset_iterator It3 = MakeSubsetIterator(&Weights3, GroupWeight);
                set Group3;
                while (!Done && GetNextSubset(&It3, &Group3))
                {
                    set Group4 = Subtract(&Weights3, &Group3);
                    
                    MinGroup1Count = Group1.Count;
                    MinQE = ComputeQE(&Group1);
                    
                    Done = true;
                    
                    Assert(Group1.Count + Group2.Count + Group3.Count + Group4.Count == Weights->Count);
                    
                    Result.Group1 = Group1;
                    Result.Group2 = Group2;
                    Result.Group3 = Group3;
                    Result.Group4 = Group4;
                }
            }
        }
    }
    return Result;
}

void
PrintGroup(set* Group)
{
    for (i32 Index = 0;
         Index < Group->Count;
         ++Index)
    {
        printf("%d ", Group->Items[Index]);
    }
}

void
PrintPartition3(partition3* Partition)
{
    PrintGroup(&Partition->Group1);
    printf("| ");
    PrintGroup(&Partition->Group2);
    printf("| ");
    PrintGroup(&Partition->Group3);
    printf(" (QE = %lld)\n", ComputeQE(&Partition->Group1));
}

void
PrintPartition4(partition4* Partition)
{
    PrintGroup(&Partition->Group1);
    printf("| ");
    PrintGroup(&Partition->Group2);
    printf("| ");
    PrintGroup(&Partition->Group3);
    printf("| ");
    PrintGroup(&Partition->Group4);
    printf(" (QE = %lld)\n", ComputeQE(&Partition->Group1));
}

i32
main()
{
    set Weights1 {{1,2,3,4,5, 7,8,9,10,11}, 10};
    partition3 Partition1 = FindBestPartition3(&Weights1);
    PrintPartition3(&Partition1);
    
    partition4 Partition1_2 = FindBestPartition4(&Weights1);
    PrintPartition4(&Partition1_2);
    
    set Weights2 {{
            1,
            2,
            3,
            7,
            11,
            13,
            17,
            19,
            23,
            31,
            37,
            41,
            43,
            47,
            53,
            59,
            61,
            67,
            71,
            73,
            79,
            83,
            89,
            97,
            101,
            103,
            107,
            109,
            113,
        }, 29};
    partition3 Partition2 = FindBestPartition3(&Weights2);
    PrintPartition3(&Partition2);
    
    partition4 Partition4 = FindBestPartition4(&Weights2);
    PrintPartition4(&Partition4);
    
    return 0;
}