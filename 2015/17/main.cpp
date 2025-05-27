#include <limits.h>
#include <stdio.h>

#define ARRAY_COUNT(a) (sizeof(a)/sizeof((a)[0]))

int power(int x, int n)
{
    int result = 1;
    for (int i = 0; i < n; ++i)
    {
        result *= x;
    }
    return result;
}

int getCombinationsCount(int* containers, int containersCount, int targetVolume)
{
    int combinationsCount = 0;
    int subsetsCount = power(2, containersCount);
    for (int subset = 1; subset < subsetsCount; ++subset)
    {
        int volume = 0;
        for (int containerIndex = 0; containerIndex < containersCount; ++containerIndex)
        {
            bool containerInSubset = (subset >> containerIndex) & 1;
            if (containerInSubset)
            {
                volume += containers[containerIndex];
            }
        }
        if (volume == targetVolume)
        {
            ++combinationsCount;
        }
    }
    return combinationsCount;
}

int getCombinationsCount2(int* containers, int containersCount, int targetVolume)
{
    int subsetsCount = power(2, containersCount);

    int minSubsetLen = INT_MAX;
    for (int subset = 1; subset < subsetsCount; ++subset)
    {
        int volume = 0;
        int subsetLen = 0;
        for (int containerIndex = 0; containerIndex < containersCount; ++containerIndex)
        {
            bool containerInSubset = (subset >> containerIndex) & 1;
            if (containerInSubset)
            {
                volume += containers[containerIndex];
                ++subsetLen;
            }
        }
        if (volume == targetVolume && subsetLen < minSubsetLen)
        {
            minSubsetLen = subsetLen;
        }
    }

    int combinationsCount = 0;
    for (int subset = 1; subset < subsetsCount; ++subset)
    {
        int volume = 0;
        int subsetLen = 0;
        for (int containerIndex = 0; containerIndex < containersCount; ++containerIndex)
        {
            bool containerInSubset = (subset >> containerIndex) & 1;
            if (containerInSubset)
            {
                volume += containers[containerIndex];
                ++subsetLen;
            }
        }
        if (volume == targetVolume && subsetLen == minSubsetLen)
        {
            ++combinationsCount;
        }
    }

    return combinationsCount;
}

int main()
{
    int exampleContainers[] = { 20, 15, 10, 5, 5 };
    printf("%d\n", getCombinationsCount(exampleContainers, ARRAY_COUNT(exampleContainers), 25));

    int containers[] = {
        43,
        3,
        4,
        10,
        21,
        44,
        4,
        6,
        47,
        41,
        34,
        17,
        17,
        44,
        36,
        31,
        46,
        9,
        27,
        38,
    };
    printf("%d\n", getCombinationsCount(containers, ARRAY_COUNT(containers), 150));
    printf("%d\n", getCombinationsCount2(containers, ARRAY_COUNT(containers), 150));

    return 0;
}
