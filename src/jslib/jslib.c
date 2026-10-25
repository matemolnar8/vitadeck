#include "jslib_internal.h"
#include "core/event_queue.h"
#include "core/package_library.h"
#include "core/util.h"

static JSValue js_get_time(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    (void)argc;
    (void)argv;
    return JS_NewFloat64(ctx, GetTime());
}

static JSValue js_native_read_text_file(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 1) return JS_ThrowTypeError(ctx, "nativeReadTextFile requires a path");

    const char *filename = JS_ToCString(ctx, argv[0]);
    if (!filename) return JS_EXCEPTION;

    size_t len = 0;
    char *contents = vd_read_file(filename, &len);
    if (!contents) {
        JS_FreeCString(ctx, filename);
        return JS_ThrowReferenceError(ctx, "Could not read file");
    }

    JSValue result = JS_NewStringLen(ctx, contents, len);
    free(contents);
    JS_FreeCString(ctx, filename);
    return result;
}

static JSValue js_native_eval_file(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 1) return JS_ThrowTypeError(ctx, "nativeEvalFile requires a path");

    const char *filename = JS_ToCString(ctx, argv[0]);
    if (!filename) return JS_EXCEPTION;

    size_t len = 0;
    char *code = vd_read_file(filename, &len);
    if (!code) {
        JS_FreeCString(ctx, filename);
        return JS_ThrowReferenceError(ctx, "Could not read file");
    }

    JSValue result = JS_Eval(ctx, code, len, filename, JS_EVAL_TYPE_GLOBAL);
    free(code);
    JS_FreeCString(ctx, filename);
    return result;
}

static JSValue js_native_get_active_deck_app_path(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    (void)argc;
    (void)argv;
    char path[VD_PATH_MAX];
    package_library_active_package_path(path, sizeof(path));
    return JS_NewString(ctx, path);
}

static void call_input_event_from_native(JSContext *ctx, const char *id, const char *event)
{
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue vitadeck = JS_GetPropertyStr(ctx, global, "vitadeck");
    JSValue input = JS_GetPropertyStr(ctx, vitadeck, "input");
    JSValue fn = JS_GetPropertyStr(ctx, input, "onInputEventFromNative");

    JSValue args[2] = {JS_NewString(ctx, id), JS_NewString(ctx, event)};

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
        if (evt.type == EVT_INPUT) {
            call_input_event_from_native(ctx, evt.id, evt.event_name);
            ran = true;
        }
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

    js_set_global_function(ctx, "getTime", js_get_time, 0);
    js_set_global_function(ctx, "nativeReadTextFile", js_native_read_text_file, 1);
    js_set_global_function(ctx, "nativeEvalFile", js_native_eval_file, 1);
    js_set_global_function(ctx, "nativeGetActiveDeckAppPath", js_native_get_active_deck_app_path, 0);
}
