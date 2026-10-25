#ifndef JS_RUNTIME_H
#define JS_RUNTIME_H

#include <stdbool.h>
#include <stdio.h>

#define VD_FAILURE_MESSAGE_MAX 512
#define VD_FAILURE_STACK_MAX 1024

typedef struct {
    void *thread;
    volatile bool stop_requested;
    volatile bool ready;
    volatile bool failed;
    char failure_message[VD_FAILURE_MESSAGE_MAX];
    char failure_stack[VD_FAILURE_STACK_MAX];
} VdJsRuntime;

void js_runtime_init(VdJsRuntime *runtime);
bool js_runtime_start(VdJsRuntime *runtime);
void js_runtime_stop(VdJsRuntime *runtime);
bool js_runtime_is_ready(const VdJsRuntime *runtime);
bool js_runtime_failed(const VdJsRuntime *runtime);

// Inline so jslib can report failures without linking the runtime thread (timer_reload_harness).
// Keeps the first failure until the next start.
static inline void js_runtime_fail(VdJsRuntime *runtime, const char *message, const char *stack)
{
    if (runtime->failed) return;
    snprintf(runtime->failure_message, sizeof(runtime->failure_message), "%s", message ? message : "");
    snprintf(runtime->failure_stack, sizeof(runtime->failure_stack), "%s", stack ? stack : "");
    runtime->failed = true;
}

#endif /* JS_RUNTIME_H */
