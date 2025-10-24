#include <raylib.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    const char *id;
    int x;
    int y;
    int width;
    int height;
} InteractiveRect;
static InteractiveRect *interactive_rects = NULL;

/**
    syncInteractiveRectsToNative(): void
*/
static void sync_interactive_rects(js_State *J) {
    arrfree(interactive_rects);

    js_getglobal(J, "vitadeck");
    js_getproperty(J, -1, "input");
    js_getproperty(J, -1, "interactiveRects");

    int length = js_getlength(J, -1);

    for (int i = 0; i < length; i++) {
        js_getindex(J, -1, i);

        js_getproperty(J, -1, "id");
        const char *id = js_tostring(J, -1);
        js_pop(J, 1);
        
        js_getproperty(J, -1, "x");
        int x = js_tointeger(J, -1);
        js_pop(J, 1);
        
        js_getproperty(J, -1, "y");
        int y = js_tointeger(J, -1);
        js_pop(J, 1);
        
        js_getproperty(J, -1, "width");
        int width = js_tointeger(J, -1);
        js_pop(J, 1);
        
        js_getproperty(J, -1, "height");
        int height = js_tointeger(J, -1);
        js_pop(J, 1);

        // TraceLog(LOG_DEBUG, "Interactive rect %d: %s: x=%d, y=%d, width=%d, height=%d", i, id, x, y, width, height);
        
        InteractiveRect rect = {.id = strdup(id), .x = x, .y = y, .width = width, .height = height};
        arrput(interactive_rects, rect);
        js_pop(J, 1);
    }
}

static const char *top_rect_id_at(int x, int y)
{
    if (!interactive_rects) return NULL;
    int count = arrlen(interactive_rects);
    for (int i = count - 1; i >= 0; i--) {
        InteractiveRect r = interactive_rects[i];
        if (x >= r.x && x < r.x + r.width && y >= r.y && y < r.y + r.height) {
            return r.id;
        }
    }
    return NULL;
}

static bool rect_id_exists(const char *id)
{
    if (!id || !interactive_rects) return false;
    int count = arrlen(interactive_rects);
    for (int i = 0; i < count; i++) {
        if (strcmp(interactive_rects[i].id, id) == 0) return true;
    }
    return false;
}

static void call_input_event_from_native(js_State *J, const char *id, const char *event) {
    js_getglobal(J, "vitadeck");
    js_getproperty(J, -1, "input");
    js_getproperty(J, -1, "onInputEventFromNative");

    js_pushnull(J);

    js_pushstring(J, id);
    js_pushstring(J, event);

    if(js_pcall(J, 2)) {
        TraceLog(LOG_ERROR, "Error calling input event from native: %s", js_tostring(J, -1));
        js_pop(J, 1);
    }
}

void process_mouse_input(js_State *J) {
    static bool prev_is_mouse_down = false;
    static char *hovered_id = NULL;
    static char *mouse_down_id = NULL;
    
    const int x = GetMouseX();
    const int y = GetMouseY();

    const char *top_id = top_rect_id_at(x, y);

    if (hovered_id && !rect_id_exists(hovered_id)) {
        free(hovered_id);
        hovered_id = NULL;
    }

    // Mouse enter/leave
    bool hover_changed = false;
    if ((hovered_id == NULL && top_id != NULL) ||
        (hovered_id != NULL && (top_id == NULL || strcmp(hovered_id, top_id) != 0))) {
        hover_changed = true;
    }

    if (hover_changed) {
        if (hovered_id) {
            TraceLog(LOG_DEBUG, "mouseleave: %s", hovered_id);
            call_input_event_from_native(J, hovered_id, "mouseleave");
            free(hovered_id);
            hovered_id = NULL;
        }
        if (top_id) {
            hovered_id = strdup(top_id);
            TraceLog(LOG_DEBUG, "mouseenter: %s", hovered_id);
            call_input_event_from_native(J, hovered_id, "mouseenter");
        }
    }

    const bool is_mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    const bool just_pressed = is_mouse_down && !prev_is_mouse_down;
    const bool just_released = !is_mouse_down && prev_is_mouse_down;

    // Mouse down
    if (just_pressed) {
        if (top_id) {
            if (mouse_down_id) { free(mouse_down_id); mouse_down_id = NULL; }
            mouse_down_id = strdup(top_id);
            TraceLog(LOG_DEBUG, "mousedown: %s", mouse_down_id);
            call_input_event_from_native(J, mouse_down_id, "mousedown");
        }
    }

    // Mouse up and click
    if (just_released) {
        if (mouse_down_id) {
            TraceLog(LOG_DEBUG, "mouseup: %s", mouse_down_id);
            call_input_event_from_native(J, mouse_down_id, "mouseup");

            if (hovered_id && strcmp(mouse_down_id, hovered_id) == 0) {
                TraceLog(LOG_DEBUG, "click: %s", mouse_down_id);
                call_input_event_from_native(J, mouse_down_id, "click");
            }

            free(mouse_down_id); 
            mouse_down_id = NULL; 
        }
    }

    prev_is_mouse_down = is_mouse_down;
}

void process_touch_input(js_State *J) {
    static bool prev_touch_down = false;
    static char *touch_hovered_id = NULL;
    static char *touch_down_id = NULL;

    const int count = GetTouchPointCount();
    const bool is_down = count > 0;

    int x = 0;
    int y = 0;
    if (is_down) {
        Vector2 pos = GetTouchPosition(0);
        x = (int)pos.x;
        y = (int)pos.y;
    }

    const char *top_id = is_down ? top_rect_id_at(x, y) : NULL;

    if (touch_hovered_id && !rect_id_exists(touch_hovered_id)) {
        free(touch_hovered_id);
        touch_hovered_id = NULL;
    }

    // Hover enter/leave while finger is down
    if (is_down) {
        bool hover_changed = false;
        if ((touch_hovered_id == NULL && top_id != NULL) ||
            (touch_hovered_id != NULL && (top_id == NULL || strcmp(touch_hovered_id, top_id) != 0))) {
            hover_changed = true;
        }

        if (hover_changed) {
            if (touch_hovered_id) {
                call_input_event_from_native(J, touch_hovered_id, "mouseleave");
                free(touch_hovered_id);
                touch_hovered_id = NULL;
            }
            if (top_id) {
                touch_hovered_id = strdup(top_id);
                call_input_event_from_native(J, touch_hovered_id, "mouseenter");
            }
        }
    }

    const bool just_pressed = is_down && !prev_touch_down;
    const bool just_released = !is_down && prev_touch_down;

    // Touch down -> mousedown
    if (just_pressed) {
        if (top_id) {
            if (touch_down_id) { free(touch_down_id); touch_down_id = NULL; }
            touch_down_id = strdup(top_id);
            call_input_event_from_native(J, touch_down_id, "mousedown");
        }
    }

    // Touch up -> mouseup (+ click if released over same target)
    if (just_released) {
        if (touch_down_id) {
            call_input_event_from_native(J, touch_down_id, "mouseup");

            if (touch_hovered_id && strcmp(touch_down_id, touch_hovered_id) == 0) {
                call_input_event_from_native(J, touch_down_id, "click");
            }

        }

        if (touch_hovered_id) {
            call_input_event_from_native(J, touch_hovered_id, "mouseleave");
        }

        if (touch_down_id) { free(touch_down_id); touch_down_id = NULL; }
        if (touch_hovered_id) { free(touch_hovered_id); touch_hovered_id = NULL; }
    }

    prev_touch_down = is_down;
}

void register_js_input(js_State *J) {
    js_newcfunction(J, sync_interactive_rects, "syncInteractiveRectsToNative", 0);
    js_setglobal(J, "syncInteractiveRectsToNative");
}