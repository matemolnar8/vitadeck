#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#include "input.h"
#include "instance_tree.h"
#include "focus_nav.h"
#include "scroll.h"
#include "core/event_queue.h"
#include "platform/text_input.h"

_Static_assert(sizeof(((InputEvent *)0)->id) >= VD_INSTANCE_ID_MAX, "input events must hold every instance id");

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

// Focus state
static char *focused_id = NULL;
static char *gamepad_down_id = NULL;
static bool prev_confirm_down = false;

// Keep in sync with the Host mappings table in CONTEXT.md.
static const struct {
    const char *name;
    int gamepad_button;
    int key;
} pad_buttons[PAD_BUTTON_COUNT] = {
    [PAD_CIRCLE] = {"circle", GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, KEY_BACKSPACE},
    [PAD_TRIANGLE] = {"triangle", GAMEPAD_BUTTON_RIGHT_FACE_UP, KEY_T},
    [PAD_SQUARE] = {"square", GAMEPAD_BUTTON_RIGHT_FACE_LEFT, KEY_S},
    [PAD_L] = {"l", GAMEPAD_BUTTON_LEFT_TRIGGER_1, KEY_L},
    [PAD_R] = {"r", GAMEPAD_BUTTON_RIGHT_TRIGGER_1, KEY_R},
};

// A button held when the Shell hides is ignored until released, so the Circle that closes the Shell does not reach
// the Deck App. One held when the runtime starts otherwise presses on its first sample.
typedef enum { PAD_STATE_UP, PAD_STATE_DOWN, PAD_STATE_IGNORED } PadState;
static PadState pad_states[PAD_BUTTON_COUNT];
static bool pad_ignore_held = false;

// The Text Input being edited, if any. Other Deck App input waits until the edit ends.
static char *editing_id = NULL;

static void push_event(EventType type, const char *id, const char *event)
{
    InputEvent evt = {.type = type};
    snprintf(evt.id, sizeof(evt.id), "%s", id);
    snprintf(evt.event_name, sizeof(evt.event_name), "%s", event);
    event_queue_push(&evt);
}

static void push_input_event(const char *id, const char *event)
{
    push_event(EVT_INPUT, id, event);
}

static void push_text_event(const char *id, const char *text)
{
    InputEvent evt = {.type = EVT_TEXT_INPUT, .text = strdup(text)};
    snprintf(evt.id, sizeof(evt.id), "%s", id);
    event_queue_push(&evt);
}

static bool same_id(const char *a, const char *b)
{
    if (!a || !b) return a == b;
    return strcmp(a, b) == 0;
}

static void replace_id(char **slot, const char *id)
{
    free(*slot);
    *slot = id ? strdup(id) : NULL;
}

static int scroll_clamp_offset(const char *scroll_id, int offset)
{
    int viewport_h = 0, content_h = 0;
    if (!instance_scroll_metrics(scroll_id, &viewport_h, &content_h)) return offset;
    int max_scroll = scroll_max_offset(viewport_h, content_h);
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

    int offset = scroll_get_offset(touch_momentum_scroll_id);
    int next = scroll_momentum_step(offset, scroll_max_offset(viewport_h, content_h), &touch_velocity_y, dt);
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

// New Focus scrolls into view; touch momentum stops so it does not carry the control back out.
static void set_focus(const char *new_id)
{
    if (same_id(focused_id, new_id)) return;
    if (focused_id) push_input_event(focused_id, "blur");
    replace_id(&focused_id, new_id);
    if (!focused_id) return;
    push_input_event(focused_id, "focus");
    touch_stop_momentum();
    instance_scroll_into_view(focused_id);
}

static void clear_focus(void)
{
    set_focus(NULL);
    replace_id(&gamepad_down_id, NULL);
}

static void end_text_input(void)
{
    replace_id(&editing_id, NULL);
    // Whatever ended the edit (Enter, Cross, a touch on the IME dialog) may still be held; it must not press again.
    prev_confirm_down = true;
    prev_touch_down = true;
    prev_is_mouse_down = true;
    pad_ignore_held = true;
}

static void begin_text_input(const char *id)
{
    char *value = NULL;
    char *placeholder = NULL;
    int max_length = 0;
    if (!instance_text_input_session(id, &value, &placeholder, &max_length)) return;
    if (vd_text_input_begin(placeholder, value, max_length)) replace_id(&editing_id, id);
    free(value);
    free(placeholder);
}

// A Press on a Text Input starts editing it.
static void push_press(const char *id)
{
    push_input_event(id, "press");
    begin_text_input(id);
}

bool input_is_editing_text(void)
{
    return editing_id != NULL;
}

void input_apply_text_input(VdTextInputStatus status)
{
    if (!editing_id) return;
    if (status == VD_TEXT_INPUT_EDITING && instance_exists(editing_id)) return;

    if (status == VD_TEXT_INPUT_EDITING)
        vd_text_input_cancel();
    else if (status == VD_TEXT_INPUT_CONFIRMED)
        push_text_event(editing_id, vd_text_input_text());
    end_text_input();
}

void poll_text_input(void)
{
    input_apply_text_input(vd_text_input_poll());
}

void input_cancel_text_input(void)
{
    if (!editing_id) return;
    vd_text_input_cancel();
    end_text_input();
}

const char *input_editing_text(const char *id)
{
    if (!vd_text_input_edits_in_place() || !same_id(editing_id, id)) return NULL;
    return vd_text_input_text();
}

void input_clear_focus(void)
{
    input_cancel_text_input();
    clear_focus();
    for (int i = 0; i < PAD_BUTTON_COUNT; i++)
        pad_states[i] = PAD_STATE_UP;
}

void input_ignore_held_pad_buttons(void)
{
    pad_ignore_held = true;
}

void input_apply_mouse(const MouseSample *s)
{
    const bool mouse_moved = s->x != prev_mouse_x || s->y != prev_mouse_y;
    prev_mouse_x = s->x;
    prev_mouse_y = s->y;

    char top_id_buf[VD_INSTANCE_ID_MAX];
    const char *top_id = instance_hit_test(s->x, s->y, top_id_buf) ? top_id_buf : NULL;
    const bool top_is_focusable = top_id && instance_is_focusable(top_id);

    if (hovered_id && !instance_exists(hovered_id)) replace_id(&hovered_id, NULL);

    // Hovering a focusable control gives it Focus; leaving it for empty space keeps Focus.
    if (mouse_moved && !same_id(hovered_id, top_id)) {
        replace_id(&hovered_id, top_id);
        if (top_is_focusable) set_focus(top_id);
    }

    const bool just_pressed = s->left_down && !prev_is_mouse_down;
    const bool just_released = !s->left_down && prev_is_mouse_down;

    if (just_pressed) {
        if (top_is_focusable)
            set_focus(top_id);
        else
            clear_focus();
        if (top_id) {
            replace_id(&mouse_down_id, top_id);
            push_input_event(mouse_down_id, "pressstart");
        }
    }

    if (just_released && mouse_down_id) {
        push_input_event(mouse_down_id, "pressend");
        if (same_id(mouse_down_id, hovered_id)) push_press(mouse_down_id);
        replace_id(&mouse_down_id, NULL);
    }

    if (s->wheel != 0.0f) {
        char scroll_id[VD_INSTANCE_ID_MAX];
        if (instance_scroll_at(s->x, s->y, scroll_id))
            scroll_apply_delta(scroll_id, -(int)(s->wheel * SCROLL_WHEEL_STEP));
    }

    prev_is_mouse_down = s->left_down;
}

void poll_mouse_input(void)
{
    const MouseSample sample = {
        .x = GetMouseX(),
        .y = GetMouseY(),
        .left_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT),
        .wheel = GetMouseWheelMove(),
    };
    input_apply_mouse(&sample);
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

    char top_id_buf[VD_INSTANCE_ID_MAX];
    const char *top_id = is_down && instance_hit_test(x, y, top_id_buf) ? top_id_buf : NULL;

    if (touch_hovered_id && !instance_exists(touch_hovered_id)) {
        free(touch_hovered_id);
        touch_hovered_id = NULL;
    }

    // Hover visual while the finger is down (suppressed during active scroll drag)
    if (is_down && !touch_gesture_scrolling) {
        bool hover_changed = false;
        if ((touch_hovered_id == NULL && top_id != NULL) ||
            (touch_hovered_id != NULL && (top_id == NULL || strcmp(touch_hovered_id, top_id) != 0))) {
            hover_changed = true;
        }

        if (hover_changed) replace_id(&touch_hovered_id, top_id);
    }

    const bool just_pressed = is_down && !prev_touch_down;
    const bool just_released = !is_down && prev_touch_down;

    if (just_pressed) {
        touch_stop_momentum();
        touch_reset_gesture();
        touch_start_x = x;
        touch_start_y = y;
        touch_prev_y = y;

        char scroll_at[VD_INSTANCE_ID_MAX];
        if (instance_scroll_at(x, y, scroll_at)) {
            touch_scroll_viewport = true;
            touch_scroll_id = strdup(scroll_at);
            touch_pending_press = top_id != NULL;
        } else {
            touch_scroll_viewport = false;
            touch_pending_press = false;
        }

        clear_focus();
        if (top_id) {
            replace_id(&touch_down_id, top_id);
            if (!touch_scroll_viewport) push_input_event(touch_down_id, "pressstart");
        }
    }

    if (is_down && prev_touch_down && touch_scroll_viewport && touch_scroll_id) {
        const float drag_y = (float)(y - touch_start_y);

        if (!touch_gesture_scrolling && fabsf(drag_y) >= SCROLL_TOUCH_THRESHOLD) {
            touch_gesture_scrolling = true;
            touch_pending_press = false;

            replace_id(&touch_down_id, NULL);
            replace_id(&touch_hovered_id, NULL);
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
            if (touch_pending_press) push_input_event(touch_down_id, "pressstart");
            push_input_event(touch_down_id, "pressend");
            if (same_id(touch_down_id, touch_hovered_id)) push_press(touch_down_id);
        }

        replace_id(&touch_down_id, NULL);
        replace_id(&touch_hovered_id, NULL);

        touch_reset_gesture();
    }

    if (is_down) {
        touch_prev_y = y;
    }

    prev_touch_down = is_down;
}

static char *find_nearest_focusable(NavDirection dir)
{
    int elem_count = 0;
    FocusableElement *elems = get_focusable_elements(&elem_count);
    int pick = focus_nav_pick(elems, elem_count, focused_id, dir);
    char *result = pick >= 0 ? strdup(elems[pick].id) : NULL;
    free_focusable_elements(elems, elem_count);
    return result;
}

static void apply_pad_buttons(const bool held[PAD_BUTTON_COUNT])
{
    for (int i = 0; i < PAD_BUTTON_COUNT; i++) {
        if (pad_ignore_held && held[i] && pad_states[i] == PAD_STATE_UP) pad_states[i] = PAD_STATE_IGNORED;
    }
    pad_ignore_held = false;

    for (int i = 0; i < PAD_BUTTON_COUNT; i++) {
        if (pad_states[i] == PAD_STATE_UP && held[i]) {
            pad_states[i] = PAD_STATE_DOWN;
            push_event(EVT_PAD_BUTTON, pad_buttons[i].name, "pressstart");
        } else if (pad_states[i] == PAD_STATE_DOWN && !held[i]) {
            pad_states[i] = PAD_STATE_UP;
            push_event(EVT_PAD_BUTTON, pad_buttons[i].name, "pressend");
        } else if (pad_states[i] == PAD_STATE_IGNORED && !held[i]) {
            pad_states[i] = PAD_STATE_UP;
        }
    }
}

void input_apply_gamepad(const GamepadSample *s)
{
    if (focused_id && !instance_exists(focused_id)) replace_id(&focused_id, NULL);
    if (gamepad_down_id && !instance_exists(gamepad_down_id)) replace_id(&gamepad_down_id, NULL);

    char requested_id[VD_INSTANCE_ID_MAX];
    if (instance_take_focus_request(requested_id) && instance_exists(requested_id)) set_focus(requested_id);

    int viewport_h = 0, content_h = 0;
    char scroll_id[VD_INSTANCE_ID_MAX];
    if (fabsf(s->stick_y) > SCROLL_ANALOG_DEADZONE && focused_id &&
        instance_scroll_for_descendant(focused_id, scroll_id) &&
        instance_scroll_metrics(scroll_id, &viewport_h, &content_h)) {
        const float delta = s->stick_y * SCROLL_ANALOG_SPEED_PX_PER_S * s->dt;
        scroll_apply_delta(scroll_id, (int)roundf(delta));
    }

    if (s->up || s->down || s->left || s->right) {
        NavDirection dir = s->up ? NAV_UP : s->down ? NAV_DOWN : s->left ? NAV_LEFT : NAV_RIGHT;
        char *next_id = find_nearest_focusable(dir);
        if (next_id) set_focus(next_id);
        free(next_id);
    }

    const bool just_pressed = s->confirm && !prev_confirm_down;
    const bool just_released = !s->confirm && prev_confirm_down;

    if (just_pressed && focused_id) {
        replace_id(&gamepad_down_id, focused_id);
        push_input_event(gamepad_down_id, "pressstart");
    }

    if (just_released && gamepad_down_id) {
        push_input_event(gamepad_down_id, "pressend");
        if (same_id(gamepad_down_id, focused_id)) push_press(gamepad_down_id);
        replace_id(&gamepad_down_id, NULL);
    }

    prev_confirm_down = s->confirm;

    apply_pad_buttons(s->pad_buttons);
}

static bool gamepad_pressed(bool available, int button)
{
    return available && IsGamepadButtonPressed(0, button);
}

static bool gamepad_down(bool available, int button)
{
    return available && IsGamepadButtonDown(0, button);
}

void poll_gamepad_input(void)
{
    // D-pad moves Focus, Cross presses it; the host keys are listed in CONTEXT.md.
    const bool pad = IsGamepadAvailable(0);
    GamepadSample sample = {
        .up = gamepad_pressed(pad, GAMEPAD_BUTTON_LEFT_FACE_UP) || IsKeyPressed(KEY_UP),
        .down = gamepad_pressed(pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || IsKeyPressed(KEY_DOWN),
        .left = gamepad_pressed(pad, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || IsKeyPressed(KEY_LEFT),
        .right = gamepad_pressed(pad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || IsKeyPressed(KEY_RIGHT),
        .confirm = gamepad_down(pad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsKeyDown(KEY_ENTER),
        .stick_y = pad ? GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) : 0.0f,
        .dt = input_frame_dt(),
    };
    for (int i = 0; i < PAD_BUTTON_COUNT; i++) {
        sample.pad_buttons[i] = gamepad_down(pad, pad_buttons[i].gamepad_button) || IsKeyDown(pad_buttons[i].key);
    }
    input_apply_gamepad(&sample);
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
