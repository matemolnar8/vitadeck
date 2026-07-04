#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#include "instance_tree.h"
#include "scroll.h"
#include "core/event_queue.h"

// Scroll speeds tuned at 60 FPS reference; continuous motion scales with delta time.
#define SCROLL_REF_FPS 60.0f
#define SCROLL_MAX_DT 0.1f
#define SCROLL_ANALOG_SPEED_PX_PER_S (14.0f * SCROLL_REF_FPS)
#define SCROLL_WHEEL_STEP 40.0f
#define SCROLL_ANALOG_DEADZONE 0.25f

// Touch scroll: drag threshold (screen px), momentum decay, velocity smoothing
#define SCROLL_TOUCH_THRESHOLD 10.0f
#define SCROLL_TOUCH_FRICTION_60 0.92f
#define SCROLL_TOUCH_MIN_VELOCITY_PX_PER_S (0.5f * SCROLL_REF_FPS)
#define SCROLL_TOUCH_MAX_VELOCITY_PX_PER_S (48.0f * SCROLL_REF_FPS)
#define SCROLL_TOUCH_VELOCITY_BLEND 0.35f
#define SCROLL_TOUCH_STATIONARY_RESET_S 0.08f

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

// Touch scroll gesture state
static char *touch_scroll_id = NULL;
static char *touch_momentum_scroll_id = NULL;
static int touch_start_x = 0;
static int touch_start_y = 0;
static int touch_prev_y = 0;
static bool touch_scroll_viewport = false;
static bool touch_gesture_scrolling = false;
static bool touch_pending_press = false;
static float touch_velocity_y = 0.0f;
static float touch_stationary_s = 0.0f;

static float input_frame_dt(void)
{
    float dt = GetFrameTime();
    if (dt <= 0.0f) dt = 1.0f / SCROLL_REF_FPS;
    if (dt > SCROLL_MAX_DT) dt = SCROLL_MAX_DT;
    return dt;
}

static float scroll_friction_for_dt(float dt)
{
    return powf(SCROLL_TOUCH_FRICTION_60, dt * SCROLL_REF_FPS);
}

// Gamepad focus state
static char *focused_id = NULL;
static char *gamepad_down_id = NULL;
static bool prev_confirm_down = false;

// Push an input event to the queue (called from UI thread)
static void push_input_event(const char *id, const char *event)
{
    InputEvent evt;
    evt.type = EVT_INPUT;
    strncpy(evt.id, id, sizeof(evt.id) - 1);
    evt.id[sizeof(evt.id) - 1] = '\0';
    strncpy(evt.event_name, event, sizeof(evt.event_name) - 1);
    evt.event_name[sizeof(evt.event_name) - 1] = '\0';
    event_queue_push(&evt);
}

static int scroll_clamp_offset(const char *scroll_id, int offset)
{
    int viewport_h = 0, content_h = 0;
    if (!instance_scroll_metrics(scroll_id, &viewport_h, &content_h)) return offset;
    int max_scroll = content_h - viewport_h;
    if (max_scroll < 0) max_scroll = 0;
    if (offset < 0) offset = 0;
    if (offset > max_scroll) offset = max_scroll;
    return offset;
}

static void scroll_apply_delta(const char *scroll_id, int delta)
{
    if (!scroll_id || delta == 0) return;
    int next = scroll_clamp_offset(scroll_id, scroll_get_offset(scroll_id) + delta);
    scroll_set_offset(scroll_id, next);
}

static void touch_stop_momentum(void)
{
    touch_velocity_y = 0.0f;
    if (touch_momentum_scroll_id) {
        free(touch_momentum_scroll_id);
        touch_momentum_scroll_id = NULL;
    }
}

static void touch_reset_gesture(void)
{
    if (touch_scroll_id) {
        free(touch_scroll_id);
        touch_scroll_id = NULL;
    }
    touch_scroll_viewport = false;
    touch_gesture_scrolling = false;
    touch_pending_press = false;
    touch_stationary_s = 0.0f;
}

static void touch_apply_momentum(void)
{
    if (!touch_momentum_scroll_id) return;

    const float dt = input_frame_dt();

    if (fabsf(touch_velocity_y) < SCROLL_TOUCH_MIN_VELOCITY_PX_PER_S) {
        touch_stop_momentum();
        return;
    }

    if (!instance_exists(touch_momentum_scroll_id)) {
        touch_stop_momentum();
        return;
    }

    int viewport_h = 0, content_h = 0;
    if (!instance_scroll_metrics(touch_momentum_scroll_id, &viewport_h, &content_h)) {
        touch_stop_momentum();
        return;
    }

    int max_scroll = content_h - viewport_h;
    if (max_scroll < 0) max_scroll = 0;

    int offset = scroll_get_offset(touch_momentum_scroll_id);
    int next = offset + (int)roundf(touch_velocity_y * dt);
    if (next <= 0) {
        next = 0;
        touch_velocity_y = 0.0f;
    } else if (next >= max_scroll) {
        next = max_scroll;
        touch_velocity_y = 0.0f;
    }

    scroll_set_offset(touch_momentum_scroll_id, next);
    touch_velocity_y *= scroll_friction_for_dt(dt);

    if (fabsf(touch_velocity_y) < SCROLL_TOUCH_MIN_VELOCITY_PX_PER_S) {
        touch_stop_momentum();
    }
}

static bool touch_finish_scroll_gesture(void)
{
    if (!touch_gesture_scrolling || !touch_scroll_id) return false;

    if (fabsf(touch_velocity_y) >= SCROLL_TOUCH_MIN_VELOCITY_PX_PER_S) {
        if (touch_velocity_y > SCROLL_TOUCH_MAX_VELOCITY_PX_PER_S)
            touch_velocity_y = SCROLL_TOUCH_MAX_VELOCITY_PX_PER_S;
        else if (touch_velocity_y < -SCROLL_TOUCH_MAX_VELOCITY_PX_PER_S)
            touch_velocity_y = -SCROLL_TOUCH_MAX_VELOCITY_PX_PER_S;

        if (touch_momentum_scroll_id) free(touch_momentum_scroll_id);
        touch_momentum_scroll_id = strdup(touch_scroll_id);
    } else {
        touch_stop_momentum();
    }

    return true;
}

void input_clear_focus(void)
{
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

void poll_mouse_input(void)
{
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
            if (mouse_down_id) {
                free(mouse_down_id);
                mouse_down_id = NULL;
            }
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

    // Mouse wheel scrolls the scroll container under the cursor
    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        char *scroll_id = instance_scroll_at(x, y);
        if (scroll_id) {
            scroll_apply_delta(scroll_id, -(int)(wheel * SCROLL_WHEEL_STEP));
            free(scroll_id);
        }
    }

    prev_is_mouse_down = is_mouse_down;
}

void poll_touch_input(void)
{
    const int count = GetTouchPointCount();
    const bool is_down = count > 0;

    if (!is_down) {
        touch_apply_momentum();
    }

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

    // Hover enter/leave while finger is down (suppressed during active scroll drag)
    if (is_down && !touch_gesture_scrolling) {
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

    if (just_pressed) {
        touch_stop_momentum();
        touch_reset_gesture();
        touch_start_x = x;
        touch_start_y = y;
        touch_prev_y = y;

        char *scroll_at = instance_scroll_at(x, y);
        if (scroll_at) {
            touch_scroll_viewport = true;
            touch_scroll_id = scroll_at;
            touch_pending_press = top_id != NULL;
        } else {
            touch_scroll_viewport = false;
            touch_pending_press = false;
        }

        input_clear_focus();
        if (top_id) {
            if (touch_down_id) {
                free(touch_down_id);
                touch_down_id = NULL;
            }
            touch_down_id = strdup(top_id);
            if (!touch_scroll_viewport) {
                push_input_event(touch_down_id, "mousedown");
            }
        }
    }

    if (is_down && prev_touch_down && touch_scroll_viewport && touch_scroll_id) {
        const float drag_y = (float)(y - touch_start_y);

        if (!touch_gesture_scrolling && fabsf(drag_y) >= SCROLL_TOUCH_THRESHOLD) {
            touch_gesture_scrolling = true;
            touch_pending_press = false;

            if (touch_down_id) {
                free(touch_down_id);
                touch_down_id = NULL;
            }

            if (touch_hovered_id) {
                push_input_event(touch_hovered_id, "mouseleave");
                free(touch_hovered_id);
                touch_hovered_id = NULL;
            }
        }

        if (touch_gesture_scrolling) {
            const int delta_y = y - touch_prev_y;
            const float dt = input_frame_dt();
            if (delta_y != 0) {
                scroll_apply_delta(touch_scroll_id, -delta_y);
                const float instant_velocity = -(float)delta_y / dt;
                touch_velocity_y = touch_velocity_y * (1.0f - SCROLL_TOUCH_VELOCITY_BLEND) +
                                   instant_velocity * SCROLL_TOUCH_VELOCITY_BLEND;
                touch_stationary_s = 0.0f;
            } else {
                touch_stationary_s += dt;
                touch_velocity_y *= scroll_friction_for_dt(dt);
                if (touch_stationary_s >= SCROLL_TOUCH_STATIONARY_RESET_S ||
                    fabsf(touch_velocity_y) < SCROLL_TOUCH_MIN_VELOCITY_PX_PER_S) {
                    touch_velocity_y = 0.0f;
                }
            }
        }
    }

    if (just_released) {
        if (touch_finish_scroll_gesture()) {
        } else if (touch_down_id) {
            if (touch_pending_press) {
                push_input_event(touch_down_id, "mousedown");
            }

            push_input_event(touch_down_id, "mouseup");

            if (touch_hovered_id && strcmp(touch_down_id, touch_hovered_id) == 0) {
                push_input_event(touch_down_id, "click");
            }
        }

        if (touch_hovered_id) {
            push_input_event(touch_hovered_id, "mouseleave");
        }

        if (touch_down_id) {
            free(touch_down_id);
            touch_down_id = NULL;
        }
        if (touch_hovered_id) {
            free(touch_hovered_id);
            touch_hovered_id = NULL;
        }

        touch_reset_gesture();
    }

    if (is_down) {
        touch_prev_y = y;
    }

    prev_touch_down = is_down;
}

typedef enum { NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT } NavDirection;

static void set_focus(const char *new_id)
{
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

static const char *find_nearest_focusable(NavDirection dir)
{
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
        case NAV_UP:
            valid = dy < 0;
            break;
        case NAV_DOWN:
            valid = dy > 0;
            break;
        case NAV_LEFT:
            valid = dx < 0;
            break;
        case NAV_RIGHT:
            valid = dx > 0;
            break;
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

void poll_gamepad_input(void)
{
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
    float stick_y = 0.0f;

    if (IsGamepadAvailable(0)) {
        up = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
        down = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        left = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        right = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
        confirm_down = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        stick_y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
    }

    // Keyboard fallback
    up = up || IsKeyPressed(KEY_UP);
    down = down || IsKeyPressed(KEY_DOWN);
    left = left || IsKeyPressed(KEY_LEFT);
    right = right || IsKeyPressed(KEY_RIGHT);
    confirm_down = confirm_down || IsKeyDown(KEY_ENTER);

    int viewport_h = 0, content_h = 0;
    char *scroll_id = focused_id ? instance_scroll_for_descendant(focused_id) : NULL;
    if (scroll_id && instance_scroll_metrics(scroll_id, &viewport_h, &content_h)) {
        const float dt = input_frame_dt();
        if (fabsf(stick_y) > SCROLL_ANALOG_DEADZONE) {
            const float delta = stick_y * SCROLL_ANALOG_SPEED_PX_PER_S * dt;
            scroll_apply_delta(scroll_id, (int)roundf(delta));
        }
    }
    if (scroll_id) free(scroll_id);

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
        if (gamepad_down_id) {
            free(gamepad_down_id);
            gamepad_down_id = NULL;
        }
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

bool input_is_focused(const char *id)
{
    if (!id) return false;
    return focused_id && strcmp(focused_id, id) == 0;
}

bool input_is_pressed(const char *id)
{
    if (!id) return false;
    if (mouse_down_id && strcmp(mouse_down_id, id) == 0) return true;
    if (touch_down_id && strcmp(touch_down_id, id) == 0) return true;
    if (gamepad_down_id && strcmp(gamepad_down_id, id) == 0) return true;
    return false;
}
