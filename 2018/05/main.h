#if !defined(MAIN_H)
#define MAIN_H

struct polymer_unit
{
    char Type;
    b32 Polarity;
    polymer_unit *Prev;
    polymer_unit *Next;
};

struct polymer
{
    polymer_unit *First;
};

#endif
