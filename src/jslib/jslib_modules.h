#ifndef JSLIB_MODULES_H
#define JSLIB_MODULES_H

#include "quickjs.h"

void register_js_log(JSContext *ctx);
void register_js_colors(JSContext *ctx);
void register_js_timeout(JSContext *ctx);
void register_js_host_control(JSContext *ctx);
void register_js_instance_tree(JSContext *ctx);

#endif
