#include "jslib_internal.h"
#include "core/js_runtime.h"

static void report_failure(JSContext *ctx, const char *source, const char *message, const char *stack)
{
    TraceLog(LOG_ERROR, "[%s] %s", source, message);
    if (stack && stack[0]) TraceLog(LOG_ERROR, "%s", stack);

    VdJsRuntime *runtime = JS_GetContextOpaque(ctx);
    if (runtime) js_runtime_fail(runtime, message, stack);
}

static void report_value(JSContext *ctx, const char *source, JSValueConst value)
{
    const char *message = JS_ToCString(ctx, value);
    if (!message) JS_FreeValue(ctx, JS_GetException(ctx));

    const char *stack = NULL;
    if (JS_IsError(ctx, value)) {
        JSValue stack_value = JS_GetPropertyStr(ctx, value, "stack");
        if (JS_IsString(stack_value)) stack = JS_ToCString(ctx, stack_value);
        JS_FreeValue(ctx, stack_value);
    }

    report_failure(ctx, source, message ? message : "unknown error", stack);
    JS_FreeCString(ctx, stack);
    JS_FreeCString(ctx, message);
}

void js_report_exception(JSContext *ctx, const char *source)
{
    JSValue exc = JS_GetException(ctx);
    report_value(ctx, source, exc);
    JS_FreeValue(ctx, exc);
}

void js_report_rejection(JSContext *ctx, JSValueConst reason)
{
    report_value(ctx, "unhandled rejection", reason);
}

static JSValue js_native_report_deck_app_error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    const char *message = argc > 0 ? JS_ToCString(ctx, argv[0]) : NULL;
    const char *stack = argc > 1 ? JS_ToCString(ctx, argv[1]) : NULL;
    report_failure(ctx, "render", message ? message : "unknown error", stack);
    JS_FreeCString(ctx, stack);
    JS_FreeCString(ctx, message);
    return JS_UNDEFINED;
}

void register_js_error(JSContext *ctx)
{
    js_set_global_function(ctx, "nativeReportDeckAppError", js_native_report_deck_app_error, 2);
}
