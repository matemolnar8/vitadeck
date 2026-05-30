#ifndef JSLIB_INTERNAL_H
#define JSLIB_INTERNAL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "quickjs.h"
#include <raylib.h>
#include "stb_ds.h"

static inline void js_set_global_function(JSContext *ctx, const char *name, JSCFunction *func, int length) {
	JSValue global = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global, name, JS_NewCFunction(ctx, func, name, length));
	JS_FreeValue(ctx, global);
}

void register_js_log(JSContext *ctx);
void register_js_colors(JSContext *ctx);
void register_js_timeout(JSContext *ctx);
void register_js_fetch(JSContext *ctx);
void register_instance_tree(JSContext *ctx);

#endif /* JSLIB_INTERNAL_H */
