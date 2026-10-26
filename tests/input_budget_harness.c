// Checks that process_input_events stops after its time budget when handlers are slow, so the JS loop reaches
// the snapshot swap during a backlog, and still handles a burst of cheap events in one call.
#include <raylib.h>
#include <stdio.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/event_queue.h"
#include "jslib/jslib.h"
#include "quickjs.h"

#define EVENTS 8

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "input_budget_harness: %s\n", message);
    failures++;
}

static int32_t get_int(JSContext *ctx, const char *name)
{
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue value = JS_GetPropertyStr(ctx, global, name);
    int32_t result = -1;
    JS_ToInt32(ctx, &result, value);
    JS_FreeValue(ctx, value);
    JS_FreeValue(ctx, global);
    return result;
}

// Queues EVENTS Pad Button events whose handler busy-waits `delay_ms`, then returns how many one call handled.
static int32_t handled_by_one_call(JSContext *ctx, int delay_ms)
{
    char source[256];
    snprintf(source, sizeof(source),
             "globalThis.handled = 0;"
             "globalThis.vitadeck = { input: { onPadButtonFromNative() {"
             "  const end = Date.now() + %d; while (Date.now() < end) {} handled++;"
             "} } };",
             delay_ms);
    JS_FreeValue(ctx, JS_Eval(ctx, source, strlen(source), "input-budget-harness.js", JS_EVAL_TYPE_GLOBAL));

    for (int i = 0; i < EVENTS; i++) {
        InputEvent evt = {.type = EVT_PAD_BUTTON};
        snprintf(evt.id, sizeof(evt.id), "down");
        snprintf(evt.event_name, sizeof(evt.event_name), i % 2 ? "pressend" : "pressstart");
        event_queue_push(&evt);
    }
    expect(process_input_events(ctx), "a call with queued events reports that it ran JS");
    const int32_t first = get_int(ctx, "handled");

    while (process_input_events(ctx)) {
    }
    expect(get_int(ctx, "handled") == EVENTS, "later calls handle the rest of the queue");
    return first;
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    if (!event_queue_init()) return 2;
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);
    if (!rt || !ctx) return 2;

    const int32_t slow = handled_by_one_call(ctx, 10);
    expect(slow >= 1 && slow < EVENTS, "slow handlers leave the rest of the queue for the next iteration");

    expect(handled_by_one_call(ctx, 0) == EVENTS, "cheap handlers all run in one call");

    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    event_queue_destroy();
    return failures ? 1 : 0;
}
