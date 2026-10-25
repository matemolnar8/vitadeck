#ifndef JSLIB_H
#define JSLIB_H

#include "quickjs.h"

void register_js_lib(JSContext *ctx);
// Takes the pending exception, logs it, and puts the Active Deck App into the Deck App Failure Screen.
void js_report_exception(JSContext *ctx, const char *source);
void process_input_events(JSContext *ctx);
void run_timeouts(JSContext *ctx);
void timeout_shutdown(JSContext *ctx);
void run_fetch(JSContext *ctx);
void fetch_shutdown(JSContext *ctx);

#endif /* JSLIB_H */
