/*
	This file is the entry point for the JS library for vitadeck.
	It's a single translation unit including all the JS library functions.
	Headers are not included in the *.c files, they are included here.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "quickjs.h"
#include <raylib.h>
#include "stb_ds.h"

#include "jslib.h"
#include "ui/instance_tree.h"
#include "core/event_queue.h"

static void js_set_global_function(JSContext *ctx, const char *name, JSCFunction *func, int length) {
	JSValue global = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global, name, JS_NewCFunction(ctx, func, name, length));
	JS_FreeValue(ctx, global);
}

#include "instance.c"
#include "colors.c"
#include "timeout.c"
#include "log.c"

static JSValue js_get_time(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	(void)this_val; (void)argc; (void)argv;
	return JS_NewFloat64(ctx, GetTime());
}

// Called from JS thread to dispatch event to JS
static void call_input_event_from_native(JSContext *ctx, const char *id, const char *event) {
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue vitadeck = JS_GetPropertyStr(ctx, global, "vitadeck");
    JSValue input = JS_GetPropertyStr(ctx, vitadeck, "input");
    JSValue fn = JS_GetPropertyStr(ctx, input, "onInputEventFromNative");

    JSValue args[2] = {
        JS_NewString(ctx, id),
        JS_NewString(ctx, event)
    };

    JSValue result = JS_Call(ctx, fn, input, 2, args);
    
    if (JS_IsException(result)) {
        JSValue exc = JS_GetException(ctx);
        const char *str = JS_ToCString(ctx, exc);
        TraceLog(LOG_ERROR, "Error calling input event from native: %s", str ? str : "unknown");
        JS_FreeCString(ctx, str);
        JS_FreeValue(ctx, exc);
    }

    JS_FreeValue(ctx, result);
    JS_FreeValue(ctx, args[0]);
    JS_FreeValue(ctx, args[1]);
    JS_FreeValue(ctx, fn);
    JS_FreeValue(ctx, input);
    JS_FreeValue(ctx, vitadeck);
    JS_FreeValue(ctx, global);
}

// Process events from queue (called from JS thread)
void process_input_events(JSContext *ctx) {
    InputEvent evt;
    while (event_queue_pop(&evt)) {
        if (evt.type == EVT_INPUT) {
            call_input_event_from_native(ctx, evt.id, evt.event_name);
        }
    }
}

void register_js_lib(JSContext *ctx) {
	register_js_log(ctx);
	register_js_colors(ctx);
	register_js_timeout(ctx);
	register_instance_tree(ctx);

	js_set_global_function(ctx, "getTime", js_get_time, 0);
}
