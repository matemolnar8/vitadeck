
typedef struct {
    const char* func_ref;
    double next_scheduled_at;
    const char* stack;
    unsigned int id;
    int interval_ms;
} TimeoutItem;

static struct { unsigned int key; TimeoutItem value; }* timeout_hm = NULL;
static struct { const char* key; int value; }* ref_count_hm = NULL;
static unsigned int timeout_id_counter = 0;

/*
    Get a function reference, keep track of the number of references to the function.
*/
const char* get_func_ref(js_State *J) {
    const char* func_ref = js_ref(J);
    int ref_count = hmget(ref_count_hm, func_ref);

    if(ref_count < 0) {
        hmput(ref_count_hm, func_ref, 1);
        TraceLog(LOG_DEBUG, "[get_func_ref] Function reference: %s, ref count set to: %d", func_ref, 1);
    } else {
        hmput(ref_count_hm, func_ref, ref_count + 1);
        TraceLog(LOG_DEBUG, "[get_func_ref] Function reference: %s, ref count incremented to: %d", func_ref, ref_count + 1);
    }

    return func_ref;
}

/*
    Free a function reference, unref if there's no more references to it
*/
void free_func_ref(js_State *J, const char* func_ref) {
    int ref_count = hmget(ref_count_hm, func_ref);
    if(ref_count < 0) {
        TraceLog(LOG_WARNING, "[free_func_ref] Function reference not found: %s", func_ref);
        return;
    }

    if(ref_count - 1 == 0) {
        hmdel(ref_count_hm, func_ref);
        js_unref(J, func_ref);
        TraceLog(LOG_DEBUG, "[free_func_ref] Function reference: %s, ref count 0, unrefing", func_ref);
        return;
    }

    hmput(ref_count_hm, func_ref, ref_count - 1);
    TraceLog(LOG_DEBUG, "[free_func_ref] Function reference: %s, ref count decremented to: %d, not unrefing", func_ref, ref_count - 1);
}

void set_timeout_impl(js_State *J, bool is_interval) {
    if(js_isundefined(J, 1)) {
        TraceLog(LOG_WARNING, "set_timeout: Function is undefined");
        js_dostring(J, "console.error(new Error().stack);");
        js_pushundefined(J);
        return;
    }

    // Copy function reference to top of stack for js_ref
    js_copy(J, 1);
    const char* func_ref = get_func_ref(J);

    const int delay_in_ms = js_tointeger(J, 2);
    const double delay_in_seconds = delay_in_ms / 1000.0;

    // Store stack trace for debugging
    js_getglobal(J, "Error");
    js_construct(J, 0);
    js_getproperty(J, -1, "stack");
    const char* stack = js_tostring(J, -1);

    // Prevent duplicate IDs
    while(hmgeti(timeout_hm, timeout_id_counter) >= 0) {
        timeout_id_counter++;
    }

    TimeoutItem item = {
        .func_ref = func_ref,
        .next_scheduled_at = GetTime() + delay_in_seconds,
        .stack = stack,
        .id = timeout_id_counter,
        .interval_ms = is_interval ? delay_in_ms : -1
    };
    
    hmput(timeout_hm, item.id, item);

    TraceLog(LOG_DEBUG, 
        "[%s] (%s, %d ms) ID: %zu, scheduled at %f, queue length: %zu, interval ms: %d",
        is_interval ? "setInterval" : "setTimeout",
        item.func_ref,
        delay_in_ms, 
        item.id, 
        item.next_scheduled_at, 
        hmlen(timeout_hm),
        item.interval_ms
    );

    // return value is the timeout ID
	js_pushnumber(J, item.id);
}

/*
    setTimeout(func: Function, delay: number): number
*/
void set_timeout(js_State *J) {
    set_timeout_impl(J, false);
}

/*
    setInterval(func: Function, delay: number): number
*/
void set_interval(js_State *J) {
    set_timeout_impl(J, true);
}

/*
    clearTimeout(id: number): void
*/
void clear_timeout(js_State *J) {
    const int id = js_tointeger(J, 1);

    if(hmgeti(timeout_hm, id) < 0) {
        TraceLog(LOG_DEBUG, "[clearTimeout/clearInterval] Timeout with ID: %zu not found, ignoring", id);
        js_pushundefined(J);
        return;
    }

    TimeoutItem item = hmget(timeout_hm, id);
    hmdel(timeout_hm, id);
    free_func_ref(J, item.func_ref);

    TraceLog(LOG_DEBUG, "[clearTimeout/clearInterval] Cleared timeout with ID: %zu", id);
    js_pushundefined(J);
}

static unsigned int tick_count = 0;
void run_timeouts(js_State *J) {
    double current_time = GetTime();
    tick_count++;

    // Collect expired timeout IDs
    unsigned int *expired_ids = NULL;
    for (int i = 0; i < hmlen(timeout_hm); i++) {
        TimeoutItem item = timeout_hm[i].value;
        if (item.next_scheduled_at <= current_time) {
            arrput(expired_ids, item.id);
        }
    }

    // Execute and remove expired timeouts
    for (size_t j = 0; j < arrlen(expired_ids); j++) {
        unsigned int id = expired_ids[j];
        TimeoutItem item = hmget(timeout_hm, id);

        js_getregistry(J, item.func_ref);
        if(js_isundefined(J, -1)) {
            TraceLog(LOG_WARNING, "[run_timeouts] %d ID: %zu Function is undefined: %s", tick_count, id, item.func_ref);
            TraceLog(LOG_WARNING, "[run_timeouts] Creation stack: %s", item.stack);
        } else {
            js_pushnull(J);

            if(js_try(J)) {
                TraceLog(LOG_ERROR, "[run_timeouts] %d ID: %zu Error calling function: %s", tick_count, id, js_trystring(J, -1, "Unknown error"));
                TraceLog(LOG_ERROR, "[run_timeouts] Creation stack: %s", item.stack);
            } else {
                TraceLog(LOG_DEBUG, "[run_timeouts] %d ID: %zu Calling function: %s Scheduled at: %f, current time: %f", tick_count, id, item.func_ref, item.next_scheduled_at, current_time);
                js_call(J, 0);
            }
            js_endtry(J);
            js_pop(J, 1);
        }

        // reschedule interval
        if(item.interval_ms >= 0) {
            item.next_scheduled_at = GetTime() + item.interval_ms / 1000.0;
            hmput(timeout_hm, item.id, item);
        } else {
            hmdel(timeout_hm, item.id);
            free_func_ref(J, item.func_ref);
        }
    }

    arrfree(expired_ids);
}

void register_js_timeout(js_State *J) {
    js_newcfunction(J, set_timeout, "setTimeout", 0);
    js_setglobal(J, "setTimeout");
    js_newcfunction(J, clear_timeout, "clearTimeout", 0);
    js_setglobal(J, "clearTimeout");
    js_newcfunction(J, set_interval, "setInterval", 0);
    js_setglobal(J, "setInterval");
    // clearInterval is the same as clearTimeout, ids are shared
    js_newcfunction(J, clear_timeout, "clearInterval", 0);
    js_setglobal(J, "clearInterval");
}