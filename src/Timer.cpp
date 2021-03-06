#include "Timer.h"


static TimerEntry *active = NULL;
static TimerEntry *pool = NULL;


static void appendToActive(TimerEntry *entry, unsigned long current);
static void appendToPool(TimerEntry *entry);
static TimerEntry *findFreeEntry(TimerCallback f, long d, int count);


static unsigned long _now = 0L;


void Timer::run() {
    // get current time
    _now = millis();

    while (active && active->fireTime <= _now) {
        TimerEntry *entry = active;
        active = entry->next;

        if (entry->count == 0) {
            appendToPool(entry);
        } else {
            appendToActive(entry, _now);
        }

        if (entry->hasRefcon)
            ((TimerCallbackWithRefcon) entry->callback)(entry->refcon);
        else
            entry->callback();
    }
}


unsigned long Timer::now()
{
    return _now;
}


long Timer::repeat(long d, TimerCallback f, int count)
{
    TimerEntry *entry = findFreeEntry(f, d, count);
    appendToActive(entry, now());
    return (long) entry;
}


long Timer::repeat(long d, TimerCallbackWithRefcon f, void *refcon, int count)
{
    TimerEntry *entry = (TimerEntry *) repeat(d, (TimerCallback) f, count);
    entry->refcon = refcon;
    entry->hasRefcon = true;
    return (long) entry;
}


long Timer::once(long d, TimerCallback f)
{
    return repeat(d, f, 1);
}


long Timer::once(long d, TimerCallbackWithRefcon f, void *refcon)
{
    return repeat(d, f, refcon, 1);
}


void Timer::clear(long timer)
{
    TimerEntry *target = (TimerEntry *) timer;

    if (target) {

        if (active == target) {
            active = target->next;
            appendToPool(target);
        } else {
            TimerEntry *entry = active;

            while (entry) {
                if (entry->next == target) {
                    entry->next = target->next;
                    appendToPool(target);
                    break;
                }
                entry = entry->next;
            }
            if (entry == NULL) {
            }
        }
    }
}


static TimerEntry *findFreeEntry(TimerCallback f, long d, int count)
{
    TimerEntry *entry;

    if (pool) {
        entry = pool;
        pool = entry->next;
    } else {
        entry = (TimerEntry *) malloc(sizeof(TimerEntry));
    }
    entry->callback = f;
    entry->count = count;
    entry->interval = d > 0 ? d : 1;
    entry->hasRefcon = false;

    return entry;
}


static void appendToActive(TimerEntry *entry, unsigned long current)
{
    if (entry->count > 0) entry->count -= 1;

    entry->fireTime = current + entry->interval;

    if (active == NULL || active->fireTime > entry->fireTime) {
        entry->next = active;
        active = entry;
    } else {
        TimerEntry *e = active->next, *prev = active;;
        while (true) {
            if (e == NULL || e->fireTime > entry->fireTime) {
                entry->next = e;
                prev->next = entry;
                break;
            }

            prev = e;
            e = e->next;
        }
    }
}


static void appendToPool(TimerEntry *entry)
{
    entry->next = NULL;

    if (pool == NULL) {
        pool = entry;
    } else {
        TimerEntry *e = pool;
        while (e->next) {
            e = e->next;
        }
        e->next = entry;
    }
}


