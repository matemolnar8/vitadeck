
#include "core/event_queue.h"
#include <string.h>

// Mouse state (file-scope to share with JS getters)
static bool prev_is_mouse_down = false;
static char *hovered_id = NULL;
static char *mouse_down_id = NULL;

// Touch state (file-scope)
static bool prev_touch_down = false;
static char *touch_hovered_id = NULL;
static char *touch_down_id = NULL;

// Use instance tree for hit testing
static const char *top_rect_id_at(int x, int y)
{
    return instance_hit_test(x, y);
}

// Check if instance still exists in tree
static bool rect_id_exists(const char *id)
{
    return instance_exists(id);
}

// Push an input event to the queue (called from UI thread)
static void push_input_event(const char *id, const char *event) {
    InputEvent evt;
    evt.type = EVT_INPUT;
    strncpy(evt.id, id, sizeof(evt.id) - 1);
    evt.id[sizeof(evt.id) - 1] = '\0';
    strncpy(evt.event_name, event, sizeof(evt.event_name) - 1);
    evt.event_name[sizeof(evt.event_name) - 1] = '\0';
    event_queue_push(&evt);
}

// Called from JS thread to dispatch event to JS
static void call_input_event_from_native(JSContext *ctx, const char *id, const char *event) {
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue vitadeck = JS_GetPropertyStr(ctx, global, "vitadeck");
    JSValue input = JS_GetPropertyStr(ctx, vitadeck, "input");
    JSValue fn = JS_GetPropertyStr(ctx, input, "onInputEventFromNative");

    JSValue args[2] = {
        JS_NewString(ctx, id),
        JS_NewString(ctx, event)
    };

    JSValue result = JS_Call(ctx, fn, input, 2, args);
    
    if (JS_IsException(result)) {
        JSValue exc = JS_GetException(ctx);
        const char *str = JS_ToCString(ctx, exc);
        TraceLog(LOG_ERROR, "Error calling input event from native: %s", str ? str : "unknown");
        JS_FreeCString(ctx, str);
        JS_FreeValue(ctx, exc);
    }

    JS_FreeValue(ctx, result);
    JS_FreeValue(ctx, args[0]);
    JS_FreeValue(ctx, args[1]);
    JS_FreeValue(ctx, fn);
    JS_FreeValue(ctx, input);
    JS_FreeValue(ctx, vitadeck);
    JS_FreeValue(ctx, global);
}

// Process events from queue (called from JS thread)
void process_input_events(JSContext *ctx) {
    InputEvent evt;
    while (event_queue_pop(&evt)) {
        if (evt.type == EVT_INPUT) {
            call_input_event_from_native(ctx, evt.id, evt.event_name);
        }
    }
}

// Poll mouse input and push events to queue (called from UI thread)
void poll_mouse_input(void) {
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
            push_input_event(hovered_id, "mouseleave");
            free(hovered_id);
            hovered_id = NULL;
        }
        if (top_id) {
            hovered_id = strdup(top_id);
            TraceLog(LOG_DEBUG, "mouseenter: %s", hovered_id);
            push_input_event(hovered_id, "mouseenter");
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
            push_input_event(mouse_down_id, "mousedown");
        }
    }

    // Mouse up and click
    if (just_released) {
        if (mouse_down_id) {
            TraceLog(LOG_DEBUG, "mouseup: %s", mouse_down_id);
            push_input_event(mouse_down_id, "mouseup");

            if (hovered_id && strcmp(mouse_down_id, hovered_id) == 0) {
                TraceLog(LOG_DEBUG, "click: %s", mouse_down_id);
                push_input_event(mouse_down_id, "click");
            }

            free(mouse_down_id); 
            mouse_down_id = NULL; 
        }
    }

    prev_is_mouse_down = is_mouse_down;
}

// Poll touch input and push events to queue (called from UI thread)
void poll_touch_input(void) {
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
                push_input_event(touch_hovered_id, "mouseleave");
                free(touch_hovered_id);
                touch_hovered_id = NULL;
            }
            if (top_id) {
                touch_hovered_id = strdup(top_id);
                push_input_event(touch_hovered_id, "mouseenter");
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
            push_input_event(touch_down_id, "mousedown");
        }
    }

    // Touch up -> mouseup (+ click if released over same target)
    if (just_released) {
        if (touch_down_id) {
            push_input_event(touch_down_id, "mouseup");

            if (touch_hovered_id && strcmp(touch_down_id, touch_hovered_id) == 0) {
                push_input_event(touch_down_id, "click");
            }

        }

        if (touch_hovered_id) {
            push_input_event(touch_hovered_id, "mouseleave");
        }

        if (touch_down_id) { free(touch_down_id); touch_down_id = NULL; }
        if (touch_hovered_id) { free(touch_hovered_id); touch_hovered_id = NULL; }
    }

    prev_touch_down = is_down;
}

// Legacy functions that call JS directly (kept for compatibility during transition)
void process_mouse_input(JSContext *ctx) {
    (void)ctx;
    poll_mouse_input();
}

void process_touch_input(JSContext *ctx) {
    (void)ctx;
    poll_touch_input();
}

static JSValue js_get_interactive_state(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 1) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    JSValue obj = JS_NewObject(ctx);
    
    bool is_hovered = false;
    if (hovered_id && strcmp(hovered_id, id) == 0) is_hovered = true;
    if (touch_hovered_id && strcmp(touch_hovered_id, id) == 0) is_hovered = true;
    JS_SetPropertyStr(ctx, obj, "hovered", JS_NewBool(ctx, is_hovered));
    
    bool is_pressed = false;
    if (mouse_down_id && strcmp(mouse_down_id, id) == 0) is_pressed = true;
    if (touch_down_id && strcmp(touch_down_id, id) == 0) is_pressed = true;
    JS_SetPropertyStr(ctx, obj, "pressed", JS_NewBool(ctx, is_pressed));

    JS_FreeCString(ctx, id);
    return obj;
}

void register_js_input(JSContext *ctx) {
    js_set_global_function(ctx, "getInteractiveState", js_get_interactive_state, 1);
}

bool input_is_hovered(const char *id)
{
    if (!id) return false;
    if (hovered_id && strcmp(hovered_id, id) == 0) return true;
    if (touch_hovered_id && strcmp(touch_hovered_id, id) == 0) return true;
    return false;
}

bool input_is_pressed(const char *id)
{
    if (!id) return false;
    if (mouse_down_id && strcmp(mouse_down_id, id) == 0) return true;
    if (touch_down_id && strcmp(touch_down_id, id) == 0) return true;
    return false;
}
