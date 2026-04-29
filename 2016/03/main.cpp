#include <stdio.h>
#include <assert.h>

bool IsTrianglePossible(int A, int B, int C)
{
    bool Result = (A + B > C) && (A + C > B) && (B + C > A);
    return(Result);
}

static void
Part1()
{
    FILE *File = fopen("input.txt", "rt");
    assert(File);
    int A, B, C;
    int PossibleCount = 0;
    while(fscanf(File, "%d %d %d ", &A, &B, &C) != EOF)
    {
        if(IsTrianglePossible(A, B, C))
        {
            ++PossibleCount;
        }
    }
    fclose(File);
    printf("%d\n", PossibleCount);
}

static void
Part2()
{
    FILE *File = fopen("input.txt", "rt");
    assert(File);
    int A1, B1, C1;
    int A2, B2, C2;
    int A3, B3, C3;
    int PossibleCount = 0;
    while(fscanf(File, "%d %d %d ", &A1, &B1, &C1) != EOF &&
          fscanf(File, "%d %d %d ", &A2, &B2, &C2) != EOF &&
          fscanf(File, "%d %d %d ", &A3, &B3, &C3) != EOF)
    {
        if(IsTrianglePossible(A1, A2, A3))
        {
            ++PossibleCount;
        }
        if(IsTrianglePossible(B1, B2, B3))
        {
            ++PossibleCount;
        }
        if(IsTrianglePossible(C1, C2, C3))
        {
            ++PossibleCount;
        }
    }
    fclose(File);
    printf("%d\n", PossibleCount);
}

int main(void)
{
    Part1();
    Part2();
    return(0);
}
