#include "event_queue.h"
#include "platform/thread.h"
#include <string.h>
#include <stdlib.h>

#define QUEUE_CAPACITY 256

typedef struct {
    InputEvent events[QUEUE_CAPACITY];
    int head;
    int tail;
    int count;
    bool shutdown;
    vd_mutex* mutex;
} EventQueue;

static EventQueue queue;

void event_queue_init(void) {
    queue.head = 0;
    queue.tail = 0;
    queue.count = 0;
    queue.shutdown = false;
    queue.mutex = vd_mutex_create();
}

void event_queue_destroy(void) {
    if (queue.mutex) {
        vd_mutex_destroy(queue.mutex);
        queue.mutex = NULL;
    }
}

void event_queue_push(const InputEvent* evt) {
    vd_mutex_lock(queue.mutex);
    
    if (queue.count < QUEUE_CAPACITY) {
        memcpy(&queue.events[queue.tail], evt, sizeof(InputEvent));
        queue.tail = (queue.tail + 1) % QUEUE_CAPACITY;
        queue.count++;
    }
    
    vd_mutex_unlock(queue.mutex);
}

bool event_queue_pop(InputEvent* evt) {
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

void event_queue_shutdown(void) {
    vd_mutex_lock(queue.mutex);
    queue.shutdown = true;
    vd_mutex_unlock(queue.mutex);
}

bool event_queue_is_shutdown(void) {
    bool shutdown;
    vd_mutex_lock(queue.mutex);
    shutdown = queue.shutdown;
    vd_mutex_unlock(queue.mutex);
    return shutdown;
}
