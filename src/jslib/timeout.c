
typedef struct {
    JSValue func;
    double next_scheduled_at;
    unsigned int id;
    int interval_ms;
} TimeoutItem;

static struct { unsigned int key; TimeoutItem value; }* timeout_hm = NULL;
static unsigned int timeout_id_counter = 0;
static JSContext *timeout_ctx = NULL;

static JSValue set_timeout_impl(JSContext *ctx, int argc, JSValueConst *argv, bool is_interval) {
    if (argc < 1 || JS_IsUndefined(argv[0])) {
        TraceLog(LOG_WARNING, "set_timeout: Function is undefined");
        return JS_UNDEFINED;
    }

    JSValue func = JS_DupValue(ctx, argv[0]);
    
    int32_t delay_in_ms = 0;
    if (argc > 1) {
        JS_ToInt32(ctx, &delay_in_ms, argv[1]);
    }
    double delay_in_seconds = delay_in_ms / 1000.0;

    while (hmgeti(timeout_hm, timeout_id_counter) >= 0) {
        timeout_id_counter++;
    }

    TimeoutItem item = {
        .func = func,
        .next_scheduled_at = GetTime() + delay_in_seconds,
        .id = timeout_id_counter,
        .interval_ms = is_interval ? delay_in_ms : -1
    };
    
    hmput(timeout_hm, item.id, item);

    TraceLog(LOG_DEBUG, 
        "[%s] ID: %u, delay: %d ms, scheduled at %f, queue length: %td",
        is_interval ? "setInterval" : "setTimeout",
        item.id, 
        delay_in_ms,
        item.next_scheduled_at, 
        hmlen(timeout_hm)
    );

    return JS_NewInt32(ctx, item.id);
}

static JSValue js_set_timeout(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    (void)this_val;
    return set_timeout_impl(ctx, argc, argv, false);
}

static JSValue js_set_interval(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    (void)this_val;
    return set_timeout_impl(ctx, argc, argv, true);
}

static JSValue js_clear_timeout(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    (void)this_val;
    if (argc < 1) return JS_UNDEFINED;

    int32_t id;
    JS_ToInt32(ctx, &id, argv[0]);

    if (hmgeti(timeout_hm, id) < 0) {
        TraceLog(LOG_DEBUG, "[clearTimeout/clearInterval] Timeout with ID: %d not found, ignoring", id);
        return JS_UNDEFINED;
    }

    TimeoutItem item = hmget(timeout_hm, id);
    hmdel(timeout_hm, id);
    JS_FreeValue(ctx, item.func);

    TraceLog(LOG_DEBUG, "[clearTimeout/clearInterval] Cleared timeout with ID: %d", id);
    return JS_UNDEFINED;
}

void run_timeouts(JSContext *ctx) {
    static unsigned int tick_count = 0;
    
    double current_time = GetTime();
    tick_count++;

    unsigned int *expired_ids = NULL;
    for (int i = 0; i < hmlen(timeout_hm); i++) {
        TimeoutItem item = timeout_hm[i].value;
        if (item.next_scheduled_at <= current_time) {
            arrput(expired_ids, item.id);
        }
    }

    for (size_t j = 0; j < arrlen(expired_ids); j++) {
        unsigned int id = expired_ids[j];
        int idx = hmgeti(timeout_hm, id);
        if (idx < 0) continue;
        
        TimeoutItem item = timeout_hm[idx].value;

        JSValue global = JS_GetGlobalObject(ctx);
        JSValue result = JS_Call(ctx, item.func, global, 0, NULL);
        JS_FreeValue(ctx, global);

        if (JS_IsException(result)) {
            JSValue exc = JS_GetException(ctx);
            const char *str = JS_ToCString(ctx, exc);
            TraceLog(LOG_ERROR, "[run_timeouts] %u ID: %u Error: %s", tick_count, id, str ? str : "unknown");
            JS_FreeCString(ctx, str);
            JS_FreeValue(ctx, exc);
        } else {
            TraceLog(LOG_DEBUG, "[run_timeouts] %u ID: %u executed", tick_count, id);
        }
        JS_FreeValue(ctx, result);

        if (item.interval_ms >= 0) {
            item.next_scheduled_at = GetTime() + item.interval_ms / 1000.0;
            hmput(timeout_hm, item.id, item);
        } else {
            hmdel(timeout_hm, item.id);
            JS_FreeValue(ctx, item.func);
        }
    }

    arrfree(expired_ids);
}

void register_js_timeout(JSContext *ctx) {
    timeout_ctx = ctx;
    js_set_global_function(ctx, "setTimeout", js_set_timeout, 2);
    js_set_global_function(ctx, "clearTimeout", js_clear_timeout, 1);
    js_set_global_function(ctx, "setInterval", js_set_interval, 2);
    js_set_global_function(ctx, "clearInterval", js_clear_timeout, 1);
}