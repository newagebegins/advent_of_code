#include <stdio.h>
#include <stdint.h>

int main()
{
    uint64_t p = 20151125;
    int row_start = 2;
    int row = row_start;
    int col = 1;
    while (true)
    {
        uint64_t c = (p * 252533) % 33554393;
        if (row == 2978 && col == 3083)
        {
            printf("%llu\n", c);
            break;
        }
        p = c;
        ++col;
        --row;
        if (row == 0)
        {
            ++row_start;
            row = row_start;
            col = 1;
        }
    }
    return 0;
}