#ifndef JS_RUNTIME_H
#define JS_RUNTIME_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    void *thread;
    volatile bool stop_requested;
    volatile bool ready;
    volatile bool failed;
} VdJsRuntime;

void js_runtime_init(VdJsRuntime *runtime);
bool js_runtime_start(VdJsRuntime *runtime);
void js_runtime_stop(VdJsRuntime *runtime);
bool js_runtime_restart(VdJsRuntime *runtime);
bool js_runtime_is_ready(const VdJsRuntime *runtime);
bool js_runtime_failed(const VdJsRuntime *runtime);

#endif /* JS_RUNTIME_H */
