#if !defined(MAIN_H)
#define MAIN_H

struct node
{
    u32 ChildCount;
    u32 MetaCount;
    node *Children;
    u32 *Meta;
};

#endif
