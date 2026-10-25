#include "js_runtime.h"

#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include "quickjs.h"
#include "stb_ds.h"
#include "core/event_queue.h"
#include "core/util.h"
#include "jslib/jslib.h"
#include "platform/thread.h"
#include "ui/input.h"
#include "ui/instance_tree.h"

// Longest idle wait between JS thread polls; bounds input latency well under a 60 FPS frame.
#define JS_IDLE_SLEEP_MAX_MS 4
// Stack left below QuickJS's limit for the native frames under the deepest JS call
// (QuickJS internals, jslib functions, TraceLog), so deep recursion throws instead of crashing.
#define JS_STACK_HEADROOM (64 * 1024)
// A Vita app gets roughly 256 MB of main memory, shared with raylib/vitaGL, fonts, images and curl.
// A quarter of it is ample for a React Deck App and stops a runaway one from starving the host.
#define JS_MEMORY_LIMIT (64 * 1024 * 1024)

static int run_function(JSContext *ctx, const char *func_name)
{
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue vitadeck = JS_GetPropertyStr(ctx, global, "vitadeck");
    JSValue func = JS_GetPropertyStr(ctx, vitadeck, func_name);

    JSValue result = JS_Call(ctx, func, vitadeck, 0, NULL);

    int ret = 0;
    if (JS_IsException(result)) {
        js_report_exception(ctx, func_name);
        ret = -1;
    }

    JS_FreeValue(ctx, result);
    JS_FreeValue(ctx, func);
    JS_FreeValue(ctx, vitadeck);
    JS_FreeValue(ctx, global);

    return ret;
}

// QuickJS reports a rejection with no handler (is_handled false), then again with is_handled true if a
// handler attaches later, so rejections are held until the microtask queue drains.
static void track_rejection(JSContext *ctx, JSValueConst promise, JSValueConst reason, JS_BOOL is_handled, void *opaque)
{
    (void)reason;
    JSValue **pending = opaque;
    if (!is_handled) {
        arrput(*pending, JS_DupValue(ctx, promise));
        return;
    }
    for (ptrdiff_t i = 0; i < arrlen(*pending); i++) {
        if (JS_VALUE_GET_PTR((*pending)[i]) == JS_VALUE_GET_PTR(promise)) {
            JS_FreeValue(ctx, (*pending)[i]);
            arrdelswap(*pending, i);
            return;
        }
    }
}

static void clear_rejections(JSContext *ctx, JSValue **pending, bool report)
{
    for (ptrdiff_t i = 0; i < arrlen(*pending); i++) {
        if (report) {
            JSValue reason = JS_PromiseResult(ctx, (*pending)[i]);
            js_report_rejection(ctx, reason);
            JS_FreeValue(ctx, reason);
        }
        JS_FreeValue(ctx, (*pending)[i]);
    }
    arrfree(*pending);
}

static void drain_microtasks(JSContext *ctx, JSValue **pending_rejections)
{
    JSContext *pctx;
    int err;
    while ((err = JS_ExecutePendingJob(JS_GetRuntime(ctx), &pctx)) != 0) {
        if (err < 0) js_report_exception(pctx, "microtask");
    }
    clear_rejections(ctx, pending_rejections, true);
}

// Sleeps until the next timer is due, capped so queued input is still picked up promptly.
static int idle_sleep_ms(void)
{
    double next_due = timeouts_next_due();
    if (next_due < 0.0) return JS_IDLE_SLEEP_MAX_MS;
    double until_due_ms = ceil((next_due - GetTime()) * 1000.0);
    if (until_due_ms <= 0.0) return 0;
    return until_due_ms < JS_IDLE_SLEEP_MAX_MS ? (int)until_due_ms : JS_IDLE_SLEEP_MAX_MS;
}

static void *js_thread_func(void *arg)
{
    VdJsRuntime *runtime = arg;
    JSRuntime *rt = NULL;
    JSContext *ctx = NULL;
    JSValue *pending_rejections = NULL;
    void *result = NULL;

    rt = JS_NewRuntime();
    if (!rt) {
        TraceLog(LOG_ERROR, "Could not initialize QuickJS runtime.");
        js_runtime_fail(runtime, "Could not initialize QuickJS runtime.", NULL);
        return NULL;
    }
    JS_SetMemoryLimit(rt, JS_MEMORY_LIMIT);
    JS_SetMaxStackSize(rt, VD_THREAD_STACK_SIZE - JS_STACK_HEADROOM);
    JS_SetHostPromiseRejectionTracker(rt, track_rejection, &pending_rejections);

    ctx = JS_NewContext(rt);
    if (!ctx) {
        TraceLog(LOG_ERROR, "Could not initialize QuickJS context.");
        js_runtime_fail(runtime, "Could not initialize QuickJS context.", NULL);
        goto defer;
    }

    JS_SetContextOpaque(ctx, runtime);
    register_js_lib(ctx);

    size_t len = 0;
    char *code = vd_read_file("js/runtime.js", &len);
    if (!code) {
        TraceLog(LOG_ERROR, "Could not load runtime.js.");
        js_runtime_fail(runtime, "Could not load runtime.js.", NULL);
        goto defer;
    }

    JSValue eval_result = JS_Eval(ctx, code, len, "runtime.js", JS_EVAL_TYPE_GLOBAL);
    free(code);

    if (JS_IsException(eval_result)) {
        js_report_exception(ctx, "runtime.js");
        JS_FreeValue(ctx, eval_result);
        goto defer;
    }
    JS_FreeValue(ctx, eval_result);

    if (run_function(ctx, "updateContainer") != 0) goto defer;

    drain_microtasks(ctx, &pending_rejections);
    instance_tree_swap();
    runtime->ready = true;

    while (!runtime->stop_requested && !runtime->failed && !event_queue_is_shutdown()) {
        bool busy = process_input_events(ctx);
        busy |= run_timeouts(ctx);
        busy |= run_fetch(ctx);
        drain_microtasks(ctx, &pending_rejections);
        busy |= instance_tree_swap();
        int sleep_ms = busy ? 0 : idle_sleep_ms();
        if (sleep_ms > 0) {
            vd_sleep_ms(sleep_ms);
        } else {
            vd_thread_yield();
        }
    }

defer:
    if (ctx) clear_rejections(ctx, &pending_rejections, false);
    if (ctx) timeout_shutdown(ctx);
    if (ctx) fetch_shutdown(ctx);
    if (ctx) JS_FreeContext(ctx);
    if (rt) JS_FreeRuntime(rt);
    return result;
}

static void clear_failure(VdJsRuntime *runtime)
{
    runtime->failed = false;
    runtime->failure_message[0] = '\0';
    runtime->failure_stack[0] = '\0';
}

void js_runtime_init(VdJsRuntime *runtime)
{
    runtime->thread = NULL;
    runtime->stop_requested = false;
    runtime->ready = false;
    clear_failure(runtime);
}

bool js_runtime_start(VdJsRuntime *runtime)
{
    if (runtime->thread) return true;
    runtime->stop_requested = false;
    runtime->ready = false;
    clear_failure(runtime);
    event_queue_clear();
    input_clear_focus();
    runtime->thread = vd_thread_create(js_thread_func, runtime);
    return runtime->thread != NULL;
}

void js_runtime_stop(VdJsRuntime *runtime)
{
    if (!runtime->thread) {
        clear_failure(runtime);
        return;
    }
    runtime->stop_requested = true;
    vd_thread_join((vd_thread *)runtime->thread);
    vd_thread_destroy((vd_thread *)runtime->thread);
    runtime->thread = NULL;
    runtime->ready = false;
    clear_failure(runtime);
    input_clear_focus();
    instance_tree_clear();
    event_queue_clear();
}

bool js_runtime_is_ready(const VdJsRuntime *runtime)
{
    return runtime->ready;
}

bool js_runtime_failed(const VdJsRuntime *runtime)
{
    return runtime->failed;
}
