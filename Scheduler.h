#include <Arduino.h>

#define MAX_TASKS 10

class Task {

public:
    Task() {
        free = true;
    }

    boolean free;
    boolean repeat;
    unsigned long interval;
    unsigned long scheduledAt;
    void (*task)();

    boolean isDue(unsigned long now) {
        return (now - scheduledAt) >= interval;
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
        return schedule(interval, task, false);
    }

    boolean every(unsigned long interval, void (*task)()) {
        return schedule(interval, task, true);
    }

    void run() {
        Task *t = nextToRun();
        t->task();
        t->free = true;
        if (t->repeat) {
            every(t->interval, t->task);
        }
    }

private:
    Task noop;

    Task tasks[MAX_TASKS];

    boolean schedule(unsigned long interval, void (*task)(), boolean repeat) {
        Task *t = nextFree();
        if (!t) {
            return false;
        }
        t->task = task;
        t->free = false;
        t->repeat = repeat;
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