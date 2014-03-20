#ifndef scheduler_h
#define scheduler_h

#include <Arduino.h>

#define MAX_TASKS 10

class Task {

public:
    Task() {
        free = true;
    }

    boolean free;
    boolean repeat;
    boolean firstTime;
    unsigned long warmup;
    unsigned long interval;
    unsigned long scheduledAt;
    void (*task)();

    boolean isDue(unsigned long now) {
        return (now - scheduledAt) >= (firstTime ? warmup : interval);
    }
};

static void _noop() {
    delay(10);
}

class Scheduler {

public:
    Scheduler() {
        noop.task = _noop;
    }

    boolean after(unsigned long interval, void (*task)()) {
        return schedule(interval, interval, task, false);
    }

    boolean every(unsigned long interval, void (*task)()) {
        return schedule(interval, interval, task, true);
    }

    boolean every(unsigned long interval, void (*task)(), unsigned long warmup) {
        return schedule(interval, warmup, task, true);
    }
    
    void cancel(void (*task)()) {
        for (int i = 0; i < MAX_TASKS; i++) {
            Task *t = &tasks[i];
            if (t->task == task) {
                t->free = true;
            }
        }
    }

    void run() {
        Task *t = nextToRun();
        t->task();
        if (t->repeat) {
            t->scheduledAt = millis();
            t->firstTime = false;
        } else {
            t->free = true;
        }
    }

private:
    Task noop;

    Task tasks[MAX_TASKS];

    boolean schedule(unsigned long interval, unsigned long warmup, void (*task)(), boolean repeat) {
        Task *t = nextFree();
        if (!t) {
            return false;
        }
        t->task = task;
        t->free = false;
        t->repeat = repeat;
        t->firstTime = true;
        t->warmup = warmup;
        t->interval = interval;
        t->scheduledAt = millis();
        return true;
    }

    Task *nextToRun() {
        unsigned long now = millis();

        Task *next = &noop;
        for (int i = 0; i < MAX_TASKS; i++) {
            Task *t = &tasks[i];
            if (!t->free && t->isDue(now) && (next->free || t->scheduledAt < next->scheduledAt)) {
                next = t;
            }
        }
        return next;
    }

    Task *nextFree() {
        for (int i = 0; i < MAX_TASKS; i++) {
            Task *t = &tasks[i];
            if (t->free) {
                return t;
            }
        }
        return NULL;
    }
};

#endif
