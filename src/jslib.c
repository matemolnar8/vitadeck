#include <raylib.h>
#include <stdio.h>
#include <mujs.h>

#define FONT_SIZE 30

extern int line_count;

typedef struct {
    const char* func_ref;
    double scheduled_at;
} TimeoutItem;
static TimeoutItem* timeout_queue = NULL;

/*
    print(message: string)
*/
void print(js_State *J) {
	const char *str = js_tostring(J, 1);

	DrawText(str, 10, 10 + line_count * FONT_SIZE, FONT_SIZE, RED);
	line_count++;

	js_pushundefined(J);
}

/*
    debug(...args: any[])
*/
void debug(js_State *J) {
	int i, top = js_gettop(J);
	for (i = 1; i < top; ++i) {
		const char *s = js_tostring(J, i);
		if (i > 1) putchar(' ');
		fputs(s, stderr);
	}
	putchar('\n');
	js_pushundefined(J);
}

/*
    setTimeout(func: Function, delay: number)
*/
void set_timeout(js_State *J) {
    if(js_isundefined(J, 1)) {
        printf("set_timeout: Function is undefined");
        js_dostring(J, "debug(new Error().stack);");
        js_pushundefined(J);
        return;
    }

    js_copy(J, 1);
    const char* func_ref = js_ref(J);

    const int delay_in_ms = js_tointeger(J, 2);
    const double delay_in_seconds = delay_in_ms / 1000.0;

    TimeoutItem item = {
        .func_ref = func_ref,
        .scheduled_at = GetTime() + delay_in_seconds
    };
    
    arrpush(timeout_queue, item);

    printf("setTimeout(%s, %d ms) scheduled at %f, queue length: %zu\n", item.func_ref, delay_in_ms, item.scheduled_at, arrlen(timeout_queue));

	js_pushundefined(J);
}

void run_timeout_queue(js_State *J) {
    double current_time = GetTime();

    for (int i = 0; i < arrlen(timeout_queue); i++) {
        TimeoutItem item = timeout_queue[i];
        if (item.scheduled_at <= current_time) {
            js_getregistry(J, item.func_ref);
	        js_pushnull(J);
            
            if(js_try(J)) {
                printf("Error calling function: %s\n", js_trystring(J, -1, "Unknown error"));
                js_pop(J, 1);
                arrdel(timeout_queue, i);
                i--;
                continue;
            }
                printf("Calling function: %s\n", item.func_ref);
                js_call(J, 0);
            js_endtry(J);

            arrdel(timeout_queue, i);
            i--;
            js_unref(J, item.func_ref);
        }
    }
}