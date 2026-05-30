#ifndef VD_HOST_CONTROL_H
#define VD_HOST_CONTROL_H

#include "quickjs.h"

struct JSContext;
struct JSRuntime;

void host_control_init(void);
void host_control_shutdown(struct JSContext *ctx);
void host_control_drain_completions(struct JSContext *ctx);

JSValue native_host_control_fetch(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue native_get_host_control_base_url(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

#endif /* VD_HOST_CONTROL_H */
