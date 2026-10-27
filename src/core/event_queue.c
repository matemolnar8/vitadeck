#include "event_queue.h"
#include "platform/thread.h"
#include <raylib.h>
#include <string.h>
#include <stdlib.h>

#define QUEUE_CAPACITY 256

typedef struct {
    InputEvent events[QUEUE_CAPACITY];
    int head;
    int tail;
    int count;
    bool shutdown;
    vd_mutex *mutex;
} EventQueue;

static EventQueue queue;

bool event_queue_init(void)
{
    queue.head = 0;
    queue.tail = 0;
    queue.count = 0;
    queue.shutdown = false;
    queue.mutex = vd_mutex_create();
    return queue.mutex != NULL;
}

void event_queue_destroy(void)
{
    if (queue.mutex) {
        vd_mutex_destroy(queue.mutex);
        queue.mutex = NULL;
    }
}

void event_queue_push(const InputEvent *evt)
{
    vd_mutex_lock(queue.mutex);

    bool full = queue.count == QUEUE_CAPACITY;
    if (!full) {
        memcpy(&queue.events[queue.tail], evt, sizeof(InputEvent));
        queue.events[queue.tail].queued_us = vd_now_us();
        queue.tail = (queue.tail + 1) % QUEUE_CAPACITY;
        queue.count++;
    } else {
        free(evt->text);
    }

    vd_mutex_unlock(queue.mutex);

    if (full) TraceLog(LOG_WARNING, "Event queue full, dropped %s on %s.", evt->event_name, evt->id);
}

bool event_queue_pop(InputEvent *evt)
{
    bool got_event = false;

    vd_mutex_lock(queue.mutex);

    if (queue.count > 0) {
        memcpy(evt, &queue.events[queue.head], sizeof(InputEvent));
        queue.head = (queue.head + 1) % QUEUE_CAPACITY;
        queue.count--;
        got_event = true;
    }

    vd_mutex_unlock(queue.mutex);

    return got_event;
}

bool event_queue_is_empty(void)
{
    vd_mutex_lock(queue.mutex);
    bool empty = queue.count == 0;
    vd_mutex_unlock(queue.mutex);
    return empty;
}

void event_queue_shutdown(void)
{
    vd_mutex_lock(queue.mutex);
    queue.shutdown = true;
    vd_mutex_unlock(queue.mutex);
}

bool event_queue_is_shutdown(void)
{
    bool shutdown;
    vd_mutex_lock(queue.mutex);
    shutdown = queue.shutdown;
    vd_mutex_unlock(queue.mutex);
    return shutdown;
}

void event_queue_clear(void)
{
    vd_mutex_lock(queue.mutex);
    for (int i = 0; i < queue.count; i++)
        free(queue.events[(queue.head + i) % QUEUE_CAPACITY].text);
    queue.head = 0;
    queue.tail = 0;
    queue.count = 0;
    vd_mutex_unlock(queue.mutex);
}
