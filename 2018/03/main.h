#if !defined(MAIN_H)
#define MAIN_H

struct claimed_fabric
{
    u8 *ClaimCounts;
    u32 Width;
    u32 Height;
};

struct claim_rect
{
    u32 X;
    u32 Y;
    u32 Width;
    u32 Height;
};

struct claim_rects
{
    u32 Count;
    claim_rect *Rects;
};

#endif
