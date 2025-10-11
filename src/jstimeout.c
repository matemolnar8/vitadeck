
typedef struct {
    const char* func_ref;
    double scheduled_at;
    const char* stack;
    ptrdiff_t id;
} TimeoutItem;
static TimeoutItem* timeout_queue = NULL;

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

    js_copy(J, 1);
    const char* func_ref = js_ref(J);

    const int delay_in_ms = js_tointeger(J, 2);
    const double delay_in_seconds = delay_in_ms / 1000.0;

    js_getglobal(J, "Error");
    js_construct(J, 0);
    js_getproperty(J, -1, "stack");
    const char* stack = js_tostring(J, -1);
    js_pop(J, 1);

    TimeoutItem item = {
        .func_ref = func_ref,
        .scheduled_at = GetTime() + delay_in_seconds,
        .stack = stack,
        .id = arrlen(timeout_queue)
    };
    
    arrpush(timeout_queue, item);

    TraceLog(LOG_DEBUG, "setTimeout(%s, %d ms) scheduled at %f, queue length: %zu", item.func_ref, delay_in_ms, item.scheduled_at, arrlen(timeout_queue));

	js_pushnumber(J, item.id);
}

/*
    clearTimeout(id: number): void
*/
void clear_timeout(js_State *J) {
    const int id = js_tointeger(J, 1);
    if(id < 0) {
        TraceLog(LOG_WARNING, "clearTimeout: Invalid ID: %d", id);
        js_pushundefined(J);
        return;
    }
    
    for(int i = 0; i < arrlen(timeout_queue); i++) {
        if(timeout_queue[i].id == id) {
            arrdel(timeout_queue, i);
            js_unref(J, timeout_queue[i].func_ref);
            i--;
            TraceLog(LOG_DEBUG, "clearTimeout: Cleared timeout with ID: %d", id);
            break;
        }
    }
    
    js_pushundefined(J);
}

void run_timeout_queue(js_State *J) {
    double current_time = GetTime();

    for (int i = 0; i < arrlen(timeout_queue); i++) {
        TimeoutItem item = timeout_queue[i];
        if (item.scheduled_at <= current_time) {
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
                    arrdel(timeout_queue, i);
                    i--;
                    continue;
                }
                    TraceLog(LOG_DEBUG, "Calling function: %s", item.func_ref);
                    js_call(J, 0);
                js_endtry(J);
            }

            arrdel(timeout_queue, i);
            i--;
            js_unref(J, item.func_ref);
        }
    }
}

void register_js_timeout(js_State *J) {
    js_newcfunction(J, set_timeout, "setTimeout", 0);
    js_setglobal(J, "setTimeout");
    js_newcfunction(J, clear_timeout, "clearTimeout", 0);
    js_setglobal(J, "clearTimeout");
}