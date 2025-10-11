#include <raylib.h>
#include <stdio.h>
#include <mujs.h>

#define FONT_SIZE 30

extern int line_count;

typedef struct {
    const char* func_ref;
    double scheduled_at;
    const char* stack;
} TimeoutItem;
static TimeoutItem* timeout_queue = NULL;

/*
    setTimeout(func: Function, delay: number)
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
        .stack = stack
    };
    
    arrpush(timeout_queue, item);

    TraceLog(LOG_DEBUG, "setTimeout(%s, %d ms) scheduled at %f, queue length: %zu", item.func_ref, delay_in_ms, item.scheduled_at, arrlen(timeout_queue));

	js_pushundefined(J);
}

// TODO: clearTimeout(id: number)

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

static Color parse_color_arg_or_default(js_State *J, int index, Color fallback)
{
    if (js_isundefined(J, index) || !js_isobject(J, index)) {
        return fallback;
    }

    Color c = fallback;

    js_getproperty(J, index, "r");
    if (js_isnumber(J, -1)) c.r = (unsigned char)js_tointeger(J, -1);
    js_pop(J, 1);

    js_getproperty(J, index, "g");
    if (js_isnumber(J, -1)) c.g = (unsigned char)js_tointeger(J, -1);
    js_pop(J, 1);

    js_getproperty(J, index, "b");
    if (js_isnumber(J, -1)) c.b = (unsigned char)js_tointeger(J, -1);
    js_pop(J, 1);

    js_getproperty(J, index, "a");
    if (js_isnumber(J, -1)) c.a = (unsigned char)js_tointeger(J, -1);
    js_pop(J, 1);

    return c;
}

/*
    drawRect(x: number, y: number, width: number, height: number, color?: Color)
*/
void draw_rect(js_State *J) {
    const int x = js_tointeger(J, 1);
    const int y = js_tointeger(J, 2);
    const int w = js_tointeger(J, 3);
    const int h = js_tointeger(J, 4);

    const Color color = parse_color_arg_or_default(J, 5, LIGHTGRAY);

    DrawRectangle(x, y, w, h, color);

    js_pushundefined(J);
}

/*
    drawRectOutline(x: number, y: number, width: number, height: number, color?: Color)
*/
void draw_rect_outline(js_State *J) {
    const int x = js_tointeger(J, 1);
    const int y = js_tointeger(J, 2);
    const int w = js_tointeger(J, 3);
    const int h = js_tointeger(J, 4);

    const Color color = parse_color_arg_or_default(J, 5, DARKGRAY);

    DrawRectangleLines(x, y, w, h, color);

    js_pushundefined(J);
}

/*
    drawText(x: number, y: number, fontSize: number, text: string, color?: Color, border?: boolean)
*/
void draw_text(js_State *J) {
    const int x = js_tointeger(J, 1);
    const int y = js_tointeger(J, 2);
    const int font_size = js_tointeger(J, 3);
    const char *str = js_tostring(J, 4);
    const bool border = js_toboolean(J, 6);
    const Color color = parse_color_arg_or_default(J, 5, BLACK);

    int text_width = MeasureText(str, font_size);
    if (border) {
        int border_padding = 4;
        Rectangle rect = { x - border_padding, y - border_padding, text_width + border_padding * 2, font_size + border_padding * 2 };
        DrawRectangleLinesEx(rect, 2, color);
    }

    DrawText(str, x, y, font_size, color);

    js_pushundefined(J);
}
