#define STB_DS_IMPLEMENTATION
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jslib/jslib.h"
#include "jslib/jslib_internal.h"
#include "quickjs.h"

static void eval_or_die(JSContext *ctx, const char *source)
{
    JSValue result = JS_Eval(ctx, source, strlen(source), "timer-reload-harness.js", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        JSValue exc = JS_GetException(ctx);
        const char *message = JS_ToCString(ctx, exc);
        fprintf(stderr, "eval failed: %s\n", message ? message : "unknown");
        JS_FreeCString(ctx, message);
        JS_FreeValue(ctx, exc);
        exit(2);
    }
    JS_FreeValue(ctx, result);
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);

    JSRuntime *rt1 = JS_NewRuntime();
    JSContext *ctx1 = JS_NewContext(rt1);
    if (!rt1 || !ctx1) return 2;
    register_js_timeout(ctx1);
    eval_or_die(ctx1,
                "globalThis.ticks = 0;"
                "globalThis.id = setInterval(() => { globalThis.ticks += 1; }, 0);");

    timeout_shutdown(ctx1);
    JS_FreeContext(ctx1);
    JS_FreeRuntime(rt1);

    JSRuntime *rt2 = JS_NewRuntime();
    JSContext *ctx2 = JS_NewContext(rt2);
    if (!rt2 || !ctx2) return 2;
    register_js_timeout(ctx2);
    eval_or_die(ctx2,
                "globalThis.ticks = 0;"
                "globalThis.id = setTimeout(() => { globalThis.ticks += 1; }, 0);");

    JSValue global = JS_GetGlobalObject(ctx2);
    JSValue id_value = JS_GetPropertyStr(ctx2, global, "id");
    int32_t id = 0;
    JS_ToInt32(ctx2, &id, id_value);
    JS_FreeValue(ctx2, id_value);
    if (id <= 0) {
        fprintf(stderr, "expected positive timer id, got %d\n", id);
        JS_FreeValue(ctx2, global);
        return 1;
    }

    run_timeouts(ctx2);

    JSValue ticks_value = JS_GetPropertyStr(ctx2, global, "ticks");
    int32_t ticks = 0;
    JS_ToInt32(ctx2, &ticks, ticks_value);
    JS_FreeValue(ctx2, ticks_value);
    JS_FreeValue(ctx2, global);
    if (ticks != 1) {
        fprintf(stderr, "expected only fresh timeout to fire once, got %d\n", ticks);
        return 1;
    }

    timeout_shutdown(ctx2);
    JS_FreeContext(ctx2);
    JS_FreeRuntime(rt2);
    return 0;
}
