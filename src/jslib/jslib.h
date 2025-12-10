#ifndef JSLIB_H
#define JSLIB_H

#include "quickjs.h"

void register_js_lib(JSContext *ctx);
void process_input_events(JSContext *ctx);
void run_timeouts(JSContext *ctx);
void render_draw_list(void);

#endif /* JSLIB_H */
