#include "core/event_queue.h"
#include "platform/thread.h"

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 256
#define PRODUCED 20000

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "event_queue_harness: %s\n", message);
    failures++;
}

static void push_numbered(int n)
{
    InputEvent evt = {.type = EVT_INPUT};
    snprintf(evt.id, sizeof(evt.id), "%d", n);
    snprintf(evt.event_name, sizeof(evt.event_name), "click");
    event_queue_push(&evt);
}

static int pop_numbered(void)
{
    InputEvent evt;
    if (!event_queue_pop(&evt)) return -1;
    return atoi(evt.id);
}

static void test_fifo_and_wraparound(void)
{
    expect(pop_numbered() == -1, "a fresh queue is empty");

    // Three passes of 200 move head and tail across the end of the ring.
    int next = 0;
    for (int pass = 0; pass < 3; pass++) {
        for (int i = 0; i < 200; i++)
            push_numbered(next + i);
        for (int i = 0; i < 200; i++) {
            if (pop_numbered() != next + i) {
                expect(false, "events pop in push order across the ring boundary");
                return;
            }
        }
        next += 200;
    }
    expect(pop_numbered() == -1, "queue is empty after popping everything");

    InputEvent long_evt = {.type = EVT_INPUT};
    memset(long_evt.id, 'x', sizeof(long_evt.id) - 1);
    snprintf(long_evt.event_name, sizeof(long_evt.event_name), "mouseenter");
    event_queue_push(&long_evt);
    InputEvent out;
    expect(event_queue_pop(&out) && memcmp(&out, &long_evt, sizeof(out)) == 0, "event round-trips byte for byte");
}

// The queue silently drops the newest event when full; push has no return value to report it.
static void test_full_queue_drops_newest(void)
{
    for (int i = 0; i < CAPACITY + 44; i++)
        push_numbered(i);

    int popped = 0;
    bool in_order = true;
    for (int n; (n = pop_numbered()) >= 0; popped++) {
        if (n != popped) in_order = false;
    }
    expect(popped == CAPACITY, "a full queue holds exactly 256 events");
    expect(in_order, "a full queue keeps the oldest 256 events and drops the newest");

    push_numbered(7);
    expect(pop_numbered() == 7, "the queue accepts events again once drained");
}

static void test_clear_and_shutdown(void)
{
    for (int i = 0; i < 10; i++)
        push_numbered(i);
    event_queue_clear();
    expect(pop_numbered() == -1, "clear drops every pending event");
    push_numbered(1);
    expect(pop_numbered() == 1, "the queue works after clear");

    expect(!event_queue_is_shutdown(), "a fresh queue is not shut down");
    event_queue_shutdown();
    expect(event_queue_is_shutdown(), "shutdown is visible to readers");
    push_numbered(2);
    expect(pop_numbered() == 2, "shutdown is only a flag; push and pop still work");
}

static atomic_bool producer_done;

static void *producer(void *arg)
{
    (void)arg;
    for (int i = 0; i < PRODUCED; i++) {
        push_numbered(i);
        if (i % 64 == 0) vd_thread_yield();
    }
    atomic_store(&producer_done, true);
    return NULL;
}

static void test_concurrent_producer(void)
{
    atomic_store(&producer_done, false);
    vd_thread *thread = vd_thread_create(producer, NULL);
    expect(thread != NULL, "producer thread starts");
    if (!thread) return;

    int last = -1;
    int received = 0;
    bool increasing = true;
    while (true) {
        bool done = atomic_load(&producer_done);
        int n = pop_numbered();
        if (n < 0) {
            if (done) break;
            vd_thread_yield();
            continue;
        }
        if (n <= last) increasing = false;
        last = n;
        received++;
    }
    vd_thread_join(thread);
    vd_thread_destroy(thread);

    expect(increasing, "a concurrent consumer sees events in push order (drops only, no reordering)");
    expect(received > 0 && received <= PRODUCED, "a concurrent consumer receives at most what was pushed");
}

int main(void)
{
    if (!event_queue_init()) {
        fprintf(stderr, "event_queue_harness: could not create the queue mutex\n");
        return 1;
    }

    test_fifo_and_wraparound();
    test_full_queue_drops_newest();
    test_clear_and_shutdown();

    event_queue_destroy();
    expect(event_queue_init(), "the queue re-initializes after destroy");
    expect(!event_queue_is_shutdown(), "init clears the shutdown flag");
    test_concurrent_producer();
    event_queue_destroy();

    if (failures) {
        fprintf(stderr, "event_queue_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("event_queue_harness: ok\n");
    return 0;
}
