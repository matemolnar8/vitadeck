#ifndef HOST_CONTROL_H
#define HOST_CONTROL_H

#include "quickjs.h"

void register_js_host_control(JSContext *ctx);
void host_control_drain_completions(JSContext *ctx, JSRuntime *rt);
void host_control_shutdown(JSContext *ctx);

#endif /* HOST_CONTROL_H */
