
typedef struct {
    const char* func_ref;
    double scheduled_at;
    const char* stack;
    unsigned int id;
} TimeoutItem;

static struct { unsigned int key; TimeoutItem value; }* timeout_queue_hm = NULL;

unsigned int timeout_id_counter = 0;

/*
    setTimeout(func: Function, delay: number): number
*/
void set_timeout(js_State *J) {
    if(js_isundefined(J, 1)) {
        TraceLog(LOG_WARNING, "set_timeout: Function is undefined");
        js_dostring(J, "console.error(new Error().stack);");
        js_pushundefined(J);
        return;
    }

    // Copy function reference to top of stack for js_ref
    js_copy(J, 1);
    const char* func_ref = js_ref(J);

    const int delay_in_ms = js_tointeger(J, 2);
    const double delay_in_seconds = delay_in_ms / 1000.0;

    // Store stack trace for debugging
    js_getglobal(J, "Error");
    js_construct(J, 0);
    js_getproperty(J, -1, "stack");
    const char* stack = js_tostring(J, -1);

    // Prevent duplicate IDs
    while(hmgeti(timeout_queue_hm, timeout_id_counter) >= 0) {
        timeout_id_counter++;
    }

    TimeoutItem item = {
        .func_ref = func_ref,
        .scheduled_at = GetTime() + delay_in_seconds,
        .stack = stack,
        .id = timeout_id_counter
    };
    
    hmput(timeout_queue_hm, item.id, item);

    TraceLog(LOG_DEBUG, "setTimeout(%s, %d ms) ID: %zu, scheduled at %f, queue length: %zu", item.func_ref, delay_in_ms, item.id, item.scheduled_at, hmlen(timeout_queue_hm));

    // return value is the timeout ID
	js_pushnumber(J, item.id);
}

/*
    clearTimeout(id: number): void
*/
void clear_timeout(js_State *J) {
    const int id = js_tointeger(J, 1);

    if(hmgeti(timeout_queue_hm, id) < 0) {
        TraceLog(LOG_DEBUG, "clearTimeout: Timeout with ID: %zu not found, ignoring", id);
        js_pushundefined(J);
        return;
    }

    TimeoutItem item = hmget(timeout_queue_hm, id);
    hmdel(timeout_queue_hm, id);
    js_unref(J, item.func_ref);

    TraceLog(LOG_DEBUG, "clearTimeout: Cleared timeout with ID: %zu", id);
    js_pushundefined(J);
}

void run_timeout_queue(js_State *J) {
    double current_time = GetTime();

    // Collect expired timeout IDs
    unsigned int *expired_ids = NULL;
    for (int i = 0; i < hmlen(timeout_queue_hm); i++) {
        TimeoutItem item = timeout_queue_hm[i].value;
        if (item.scheduled_at <= current_time) {
            arrput(expired_ids, item.id);
        }
    }

    // Execute and remove expired timeouts
    for (size_t j = 0; j < arrlen(expired_ids); j++) {
        unsigned int id = expired_ids[j];

        TimeoutItem item = hmget(timeout_queue_hm, id);

        js_getregistry(J, item.func_ref);
        if(js_isundefined(J, -1)) {
            TraceLog(LOG_WARNING, "Function is undefined: %s", item.func_ref);
            TraceLog(LOG_WARNING, "Creation stack: %s", item.stack);
        } else {
            js_pushnull(J);

            if(js_try(J)) {
                TraceLog(LOG_ERROR, "Error calling function: %s", js_trystring(J, -1, "Unknown error"));
                TraceLog(LOG_ERROR, "Creation stack: %s", item.stack);
                js_pop(J, 1);
            } else {
                TraceLog(LOG_DEBUG, "Calling function: %s", item.func_ref);
                js_call(J, 0);
            }
            js_endtry(J);
        }

        // Clean up the timeout regardless of execution result
        hmdel(timeout_queue_hm, item.id);
        js_unref(J, item.func_ref);
    }

    arrfree(expired_ids);
}

void register_js_timeout(js_State *J) {
    js_newcfunction(J, set_timeout, "setTimeout", 0);
    js_setglobal(J, "setTimeout");
    js_newcfunction(J, clear_timeout, "clearTimeout", 0);
    js_setglobal(J, "clearTimeout");
}