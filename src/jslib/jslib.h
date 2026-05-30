#ifndef JSLIB_H
#define JSLIB_H

#include "quickjs.h"

void register_js_lib(JSContext *ctx);
void process_input_events(JSContext *ctx);
void run_timeouts(JSContext *ctx);
void run_fetch(JSContext *ctx);
void fetch_shutdown(JSContext *ctx);

#endif /* JSLIB_H */
