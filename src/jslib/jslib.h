#ifndef JSLIB_H
#define JSLIB_H

#include <stdbool.h>
#include "quickjs.h"

void register_js_lib(JSContext *ctx);
// Takes the pending exception, logs it, and puts the Active Deck App into the Deck App Failure Screen.
void js_report_exception(JSContext *ctx, const char *source);
// These return whether they ran any JS, so the JS thread knows when it is idle.
bool process_input_events(JSContext *ctx);
bool run_timeouts(JSContext *ctx);
// GetTime() of the earliest pending timer, or a negative value when none is pending.
double timeouts_next_due(void);
void timeout_shutdown(JSContext *ctx);
bool run_fetch(JSContext *ctx);
void fetch_shutdown(JSContext *ctx);

#endif /* JSLIB_H */
