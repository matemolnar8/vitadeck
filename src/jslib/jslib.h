#ifndef JSLIB_H
#define JSLIB_H

#include "quickjs.h"

void register_js_lib(JSContext *ctx);
void process_input_events(JSContext *ctx);
void run_timeouts(JSContext *ctx);

#endif /* JSLIB_H */
