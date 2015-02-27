/*
 * Timer.h
 *
 */


#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

typedef void (*TimerCallback)(void);
typedef void (*TimerCallbackWithRefcon)(void *refcon);

typedef struct TimerEntry TimerEntry;

#define Hz(x) (1000 / (x))

// #define DEBUG(xxx) Serial.println(xxx)
#define DEBUG(xxx)

class Timer {
public:
    static void run();

    static long repeat(long d, TimerCallback f, int count=-1);
    static long repeat(long d, TimerCallbackWithRefcon f, void *refcon, int count=-1);

    static long once(long d, TimerCallbackWithRefcon f, void *refcon);
    static long once(long d, TimerCallback f);

    static void clear(long timeout);
};

struct TimerEntry {
    TimerEntry *next;
    TimerCallback callback;
    unsigned long interval;
    unsigned long fireTime;
    void *refcon;
    bool hasRefcon;
    int count;
};

#endif
