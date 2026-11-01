#if !defined(MAIN_H)
#define MAIN_H

enum event
{
    Event_BeginsShift,
    Event_FallsAsleep,
    Event_WakesUp,
};

#define ONE_PAST_MAX_GUARD_ID 4096

struct record
{
    event Event;
    u16 GuardID;
    u16 Year;
    u8 Month;
    u8 Day;
    u8 Hour;
    u8 Minute;
};

struct records
{
    u32 Count;
    record *Records;
};

struct guard
{
    u32 TotalMinutesAsleep;
    u8 AsleepCountForMinute[60];
};

#endif
