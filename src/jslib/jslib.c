/*
	This file is the entry point for the JS library for vitadeck.
	It's a single translation unit including all the JS library functions.
	Headers are not included in the js*.c files, they are included here.
*/
#include <stdio.h>
#include <string.h>
#include "quickjs.h"
#include <raylib.h>
#include "stb_ds.h"

#include "jslib.h"
#include "core/event_queue.h"

static void js_set_global_function(JSContext *ctx, const char *name, JSCFunction *func, int length) {
	JSValue global = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global, name, JS_NewCFunction(ctx, func, name, length));
	JS_FreeValue(ctx, global);
}

#include "instance_tree.c"
#include "draw.c"
#include "timeout.c"
#include "input.c"
#include "log.c"

static JSValue js_get_time(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	(void)this_val; (void)argc; (void)argv;
	return JS_NewFloat64(ctx, GetTime());
}

void register_js_lib(JSContext *ctx) {
	register_js_log(ctx);
	register_js_draw(ctx);
	register_js_timeout(ctx);
	register_js_input(ctx);
	register_instance_tree(ctx);

	js_set_global_function(ctx, "getTime", js_get_time, 0);
}