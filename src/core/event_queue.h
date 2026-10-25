#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stdbool.h>

typedef enum { EVT_INPUT, EVT_PAD_BUTTON } EventType;

// EVT_INPUT: id is the instance, event_name is focus, blur, pressstart, pressend or press.
// EVT_PAD_BUTTON: id is the Pad Button name, event_name is pressstart or pressend.
typedef struct {
    EventType type;
    char id[64];
    char event_name[32];
} InputEvent;

bool event_queue_init(void);
void event_queue_destroy(void);
void event_queue_push(const InputEvent *evt);
bool event_queue_pop(InputEvent *evt);
void event_queue_shutdown(void);
bool event_queue_is_shutdown(void);
void event_queue_clear(void);

#endif /* EVENT_QUEUE_H */
