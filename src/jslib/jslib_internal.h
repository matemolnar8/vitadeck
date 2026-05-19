#ifndef JSLIB_INTERNAL_H
#define JSLIB_INTERNAL_H

#include "quickjs.h"

void js_set_global_function(JSContext *ctx, const char *name, JSCFunction *func, int length);

#endif
