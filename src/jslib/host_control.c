#include "jslib_internal.h"
#include "host_control/link.h"

// Read live on every Host Control call, so a link made while a Deck App runs takes effect at once.
static JSValue js_native_get_host_control_links(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    (void)argc;
    (void)argv;
    char *json = host_control_links_json();
    if (!json) return JS_ThrowOutOfMemory(ctx);
    JSValue result = JS_NewString(ctx, json);
    free(json);
    return result;
}

void register_js_host_control(JSContext *ctx)
{
    js_set_global_function(ctx, "nativeGetHostControlLinks", js_native_get_host_control_links, 0);
}
