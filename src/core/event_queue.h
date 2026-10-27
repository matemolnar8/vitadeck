#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum { EVT_INPUT, EVT_PAD_BUTTON, EVT_TEXT_INPUT } EventType;

// EVT_INPUT: id is the instance, event_name is focus, blur, pressstart, pressend or press.
// EVT_PAD_BUTTON: id is the Pad Button name, event_name is pressstart or pressend.
// EVT_TEXT_INPUT: id is the Text Input instance, text its confirmed UTF-8 value.
// text is heap-allocated or NULL; push takes ownership and pop hands it to the caller, who frees it.
// push stamps queued_us with vd_now_us(), for Benchmark Mode's input latency.
typedef struct {
    EventType type;
    char id[64];
    char event_name[32];
    char *text;
    uint64_t queued_us;
} InputEvent;

bool event_queue_init(void);
void event_queue_destroy(void);
void event_queue_push(const InputEvent *evt);
bool event_queue_pop(InputEvent *evt);
// For harnesses: true once the JS thread has taken every queued event.
bool event_queue_is_empty(void);
void event_queue_shutdown(void);
bool event_queue_is_shutdown(void);
void event_queue_clear(void);

#endif /* EVENT_QUEUE_H */
