#include "jslib_internal.h"

void js_set_global_function(JSContext *ctx, const char *name, JSCFunction *func, int length)
{
	JSValue global = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global, name, JS_NewCFunction(ctx, func, name, length));
	JS_FreeValue(ctx, global);
}
