#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#include "instance_tree.h"
#include "core/event_queue.h"

// Mouse state
static bool prev_is_mouse_down = false;
static int prev_mouse_x = -1;
static int prev_mouse_y = -1;
static char *hovered_id = NULL;
static char *mouse_down_id = NULL;

// Touch state
static bool prev_touch_down = false;
static char *touch_hovered_id = NULL;
static char *touch_down_id = NULL;

// Gamepad focus state
static char *focused_id = NULL;
static char *gamepad_down_id = NULL;
static bool prev_confirm_down = false;

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

void input_clear_focus(void) {
    if (focused_id) {
        push_input_event(focused_id, "mouseleave");
        free(focused_id);
        focused_id = NULL;
    }
    if (gamepad_down_id) {
        free(gamepad_down_id);
        gamepad_down_id = NULL;
    }
}

void poll_mouse_input(void) {
    const int x = GetMouseX();
    const int y = GetMouseY();

    // Track if mouse actually moved
    const bool mouse_moved = (x != prev_mouse_x || y != prev_mouse_y);
    prev_mouse_x = x;
    prev_mouse_y = y;

    const char *top_id = instance_hit_test(x, y);

    if (hovered_id && !instance_exists(hovered_id)) {
        free(hovered_id);
        hovered_id = NULL;
    }

    // Mouse enter/leave (only process if mouse actually moved)
    if (mouse_moved) {
        bool hover_changed = false;
        if ((hovered_id == NULL && top_id != NULL) ||
            (hovered_id != NULL && (top_id == NULL || strcmp(hovered_id, top_id) != 0))) {
            hover_changed = true;
        }

        if (hover_changed) {
            input_clear_focus();
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
    }

    const bool is_mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    const bool just_pressed = is_mouse_down && !prev_is_mouse_down;
    const bool just_released = !is_mouse_down && prev_is_mouse_down;

    // Mouse down
    if (just_pressed) {
        input_clear_focus();
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

    const char *top_id = is_down ? instance_hit_test(x, y) : NULL;

    if (touch_hovered_id && !instance_exists(touch_hovered_id)) {
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
        input_clear_focus();
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

typedef enum { NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT } NavDirection;

static void set_focus(const char *new_id) {
    if (focused_id) {
        if (new_id && strcmp(focused_id, new_id) == 0) return;
        push_input_event(focused_id, "mouseleave");
        free(focused_id);
        focused_id = NULL;
    }
    if (new_id) {
        focused_id = strdup(new_id);
        push_input_event(focused_id, "mouseenter");
    }
}

static const char *find_nearest_focusable(NavDirection dir) {
    int elem_count = 0;
    FocusableElement *elems = get_focusable_elements(&elem_count);
    if (!elems || elem_count == 0) return NULL;

    int cur_cx = 0, cur_cy = 0;
    bool has_current = false;

    // Find current focused element center
    if (focused_id) {
        for (int i = 0; i < elem_count; i++) {
            if (strcmp(elems[i].id, focused_id) == 0) {
                cur_cx = elems[i].x + elems[i].width / 2;
                cur_cy = elems[i].y + elems[i].height / 2;
                has_current = true;
                break;
            }
        }
    }

    // If no current focus, pick first element
    if (!has_current) {
        const char *first_id = elems[0].id;
        char *result = strdup(first_id);
        free_focusable_elements(elems, elem_count);
        return result;
    }

    const char *best_id = NULL;
    float best_score = 1e9f;

    for (int i = 0; i < elem_count; i++) {
        if (focused_id && strcmp(elems[i].id, focused_id) == 0) continue;

        int cx = elems[i].x + elems[i].width / 2;
        int cy = elems[i].y + elems[i].height / 2;
        int dx = cx - cur_cx;
        int dy = cy - cur_cy;

        // Check if element is in the correct direction
        bool valid = false;
        switch (dir) {
            case NAV_UP:    valid = dy < 0; break;
            case NAV_DOWN:  valid = dy > 0; break;
            case NAV_LEFT:  valid = dx < 0; break;
            case NAV_RIGHT: valid = dx > 0; break;
        }
        if (!valid) continue;

        // Calculate weighted distance (favor elements more aligned with direction)
        float primary = 0, secondary = 0;
        switch (dir) {
            case NAV_UP:
            case NAV_DOWN:
                primary = fabsf((float)dy);
                secondary = fabsf((float)dx);
                break;
            case NAV_LEFT:
            case NAV_RIGHT:
                primary = fabsf((float)dx);
                secondary = fabsf((float)dy);
                break;
        }
        float score = primary + secondary * 2.0f;

        if (score < best_score) {
            best_score = score;
            best_id = elems[i].id;
        }
    }

    char *result = best_id ? strdup(best_id) : NULL;
    free_focusable_elements(elems, elem_count);
    return result;
}

void poll_gamepad_input(void) {
    // Check if focused element still exists
    if (focused_id && !instance_exists(focused_id)) {
        free(focused_id);
        focused_id = NULL;
    }
    if (gamepad_down_id && !instance_exists(gamepad_down_id)) {
        free(gamepad_down_id);
        gamepad_down_id = NULL;
    }

    // Read d-pad input (gamepad or keyboard)
    bool up = false, down = false, left = false, right = false;
    bool confirm_down = false;

    if (IsGamepadAvailable(0)) {
        up = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
        down = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        left = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        right = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
        confirm_down = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    }

    // Keyboard fallback
    up = up || IsKeyPressed(KEY_UP);
    down = down || IsKeyPressed(KEY_DOWN);
    left = left || IsKeyPressed(KEY_LEFT);
    right = right || IsKeyPressed(KEY_RIGHT);
    confirm_down = confirm_down || IsKeyDown(KEY_ENTER);

    // Navigation
    if (up || down || left || right) {
        NavDirection dir = up ? NAV_UP : down ? NAV_DOWN : left ? NAV_LEFT : NAV_RIGHT;
        const char *next_id = find_nearest_focusable(dir);
        if (next_id) {
            set_focus(next_id);
            free((void *)next_id);
        }
    }

    // Activation (Cross/Enter)
    bool just_pressed = confirm_down && !prev_confirm_down;
    bool just_released = !confirm_down && prev_confirm_down;

    if (just_pressed && focused_id) {
        if (gamepad_down_id) { free(gamepad_down_id); gamepad_down_id = NULL; }
        gamepad_down_id = strdup(focused_id);
        push_input_event(gamepad_down_id, "mousedown");
    }

    if (just_released && gamepad_down_id) {
        push_input_event(gamepad_down_id, "mouseup");
        if (focused_id && strcmp(gamepad_down_id, focused_id) == 0) {
            push_input_event(gamepad_down_id, "click");
        }
        free(gamepad_down_id);
        gamepad_down_id = NULL;
    }

    prev_confirm_down = confirm_down;
}

bool input_is_hovered(const char *id)
{
    if (!id) return false;
    if (hovered_id && strcmp(hovered_id, id) == 0) return true;
    if (touch_hovered_id && strcmp(touch_hovered_id, id) == 0) return true;
    if (focused_id && strcmp(focused_id, id) == 0) return true;
    return false;
}

bool input_is_pressed(const char *id)
{
    if (!id) return false;
    if (mouse_down_id && strcmp(mouse_down_id, id) == 0) return true;
    if (touch_down_id && strcmp(touch_down_id, id) == 0) return true;
    if (gamepad_down_id && strcmp(gamepad_down_id, id) == 0) return true;
    return false;
}

