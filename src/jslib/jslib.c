#include "jslib_internal.h"
#include "core/bench.h"
#include "core/event_queue.h"
#include "core/package_library.h"
#include "core/package_manifest.h"
#include "core/util.h"
#include "platform/thread.h"

static JSValue js_get_time(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    (void)argc;
    (void)argv;
    return JS_NewFloat64(ctx, GetTime());
}

// The reconciler brackets each React commit with this, false at prepareForCommit and true at resetAfterCommit.
static JSValue js_native_bench_commit(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    bench_commit(argc > 0 && JS_ToBool(ctx, argv[0]));
    return JS_UNDEFINED;
}

// Evaluates the Active Deck App's Package Entry; its manifest was validated before the runtime started.
static JSValue js_native_eval_active_deck_app_entry(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    (void)argc;
    (void)argv;
    char package_path[VD_PATH_MAX];
    char entry_path[VD_PATH_MAX];
    package_library_active_package_path(package_path, sizeof(package_path));
    if (package_path[0] == '\0' || !vd_join_path(entry_path, sizeof(entry_path), package_path, VD_PACKAGE_ENTRY)) {
        return JS_ThrowReferenceError(ctx, "No Active Deck App");
    }

    size_t len = 0;
    char *code = vd_read_file(entry_path, &len);
    if (!code) return JS_ThrowReferenceError(ctx, "Could not read Deck App Package Entry");

    JSValue result = JS_Eval(ctx, code, len, entry_path, JS_EVAL_TYPE_GLOBAL);
    free(code);
    return result;
}

// Calls vitadeck.input[fn_name](a, b), the runtime's entry points for native input.
static void call_runtime_input(JSContext *ctx, const char *fn_name, const char *a, const char *b)
{
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue vitadeck = JS_GetPropertyStr(ctx, global, "vitadeck");
    JSValue input = JS_GetPropertyStr(ctx, vitadeck, "input");
    JSValue fn = JS_GetPropertyStr(ctx, input, fn_name);

    JSValue args[2] = {JS_NewString(ctx, a), JS_NewString(ctx, b)};

    JSValue result = JS_Call(ctx, fn, input, 2, args);

    if (JS_IsException(result)) js_report_exception(ctx, "input");

    JS_FreeValue(ctx, result);
    JS_FreeValue(ctx, args[0]);
    JS_FreeValue(ctx, args[1]);
    JS_FreeValue(ctx, fn);
    JS_FreeValue(ctx, input);
    JS_FreeValue(ctx, vitadeck);
    JS_FreeValue(ctx, global);
}

bool process_input_events(JSContext *ctx)
{
    bool ran = false;
    InputEvent evt;
    while (event_queue_pop(&evt)) {
        const uint64_t started_us = vd_now_us();
        switch (evt.type) {
        case EVT_INPUT:
            call_runtime_input(ctx, "onInputEventFromNative", evt.id, evt.event_name);
            break;
        case EVT_PAD_BUTTON:
            call_runtime_input(ctx, "onPadButtonFromNative", evt.id, evt.event_name);
            break;
        case EVT_TEXT_INPUT:
            call_runtime_input(ctx, "onTextInputFromNative", evt.id, evt.text ? evt.text : "");
            break;
        }
        free(evt.text);
        bench_js_input_handled(JS_GetRuntime(ctx), evt.queued_us, started_us);
        ran = true;
    }
    return ran;
}

void register_js_lib(JSContext *ctx)
{
    register_js_log(ctx);
    register_js_error(ctx);
    register_js_colors(ctx);
    register_js_timeout(ctx);
    register_js_fetch(ctx);
    register_instance_tree(ctx);
    register_js_storage(ctx);

    js_set_global_function(ctx, "getTime", js_get_time, 0);
    js_set_global_function(ctx, "nativeEvalActiveDeckAppEntry", js_native_eval_active_deck_app_entry, 0);
    js_set_global_function(ctx, "nativeBenchCommit", js_native_bench_commit, 1);
}
