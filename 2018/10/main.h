#if !defined(MAIN_H)
#define MAIN_H

struct v2i
{
    s32 x, y;
};

struct point
{
    v2i P, V;
};

struct points
{
    u32 Count;
    point *Points;
};

#endif
