#define STB_DS_IMPLEMENTATION
#include "instance_fixture.h"
#include "core/event_queue.h"
#include "ui/input.h"

#include <stdio.h>

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "input_harness: %s\n", message);
    failures++;
}

// Drains the queue as space-separated "event:id" for instance events and "pad:button:phase" for Pad Buttons.
static const char *drain(void)
{
    static char out[512];
    out[0] = '\0';
    size_t len = 0;
    InputEvent evt;
    while (event_queue_pop(&evt)) {
        const char *sep = len > 0 ? " " : "";
        int n = evt.type == EVT_PAD_BUTTON ? snprintf(out + len, sizeof(out) - len, "%spad:%s:%s", sep, evt.id, evt.event_name)
                                           : snprintf(out + len, sizeof(out) - len, "%s%s:%s", sep, evt.event_name, evt.id);
        if (n > 0 && (size_t)n < sizeof(out) - len) len += (size_t)n;
    }
    return out;
}

static void expect_events(const char *expected, const char *message)
{
    const char *got = drain();
    if (strcmp(got, expected) != 0) {
        fprintf(stderr, "input_harness: events \"%s\", want \"%s\"\n", got, expected);
        expect(false, message);
    }
}

static void mouse(int x, int y, bool left_down)
{
    const MouseSample sample = {.x = x, .y = y, .left_down = left_down};
    input_apply_mouse(&sample);
}

static void gamepad(GamepadSample sample)
{
    sample.dt = 1.0f / 60.0f;
    input_apply_gamepad(&sample);
}

static void pad(PadButton button, bool held)
{
    GamepadSample sample = {0};
    sample.pad_buttons[button] = held;
    gamepad(sample);
}

// Buttons a and b side by side, a non-focusable rect below them.
static ReactInstance *build_tree(void)
{
    ReactInstance *a = fixture_button(NULL, "a", 0, 0, 40, 40);
    fixture_button(NULL, "b", 100, 0, 40, 40);
    fixture_rect(NULL, "r", 0, 100, 100, 100);
    fixture_publish();
    return a;
}

static void reset(void)
{
    mouse(500, 500, false);
    input_clear_focus();
    gamepad((GamepadSample){0});
    drain();
}

static void test_hover_focus(void)
{
    build_tree();
    reset();

    mouse(10, 10, false);
    expect_events("focus:a", "hovering a Button focuses it");
    mouse(11, 11, false);
    expect_events("", "moving within the Button keeps Focus quietly");
    mouse(110, 10, false);
    expect_events("blur:a focus:b", "hovering another Button moves Focus");
    mouse(300, 300, false);
    expect_events("", "hovering empty space keeps Focus");
    expect(input_is_focused("b"), "b keeps Focus over empty space");
    mouse(50, 150, false);
    expect_events("", "hovering a Rect does not focus it");
    mouse(110, 10, false);
    expect_events("", "hovering the focused Button again changes nothing");

    gamepad((GamepadSample){.left = true});
    expect_events("blur:b focus:a", "the D-pad moves Focus from where hover left it");
    gamepad((GamepadSample){.right = true});
    expect_events("blur:a focus:b", "the D-pad moves Focus again");
    gamepad((GamepadSample){.left = true});
    expect_events("blur:b focus:a", "the D-pad moves Focus off the hovered Button");
    mouse(110, 10, false);
    expect_events("", "a pointer that did not move does not take Focus back");

    instance_tree_clear();
}

static void test_press(void)
{
    build_tree();
    reset();

    mouse(10, 10, false);
    mouse(10, 10, true);
    mouse(10, 10, false);
    expect_events("focus:a pressstart:a pressend:a press:a", "a mouse press on the focused Button presses it");

    mouse(300, 300, true);
    mouse(300, 300, false);
    expect_events("blur:a", "a mouse press on empty space clears Focus");

    gamepad((GamepadSample){.right = true});
    expect_events("focus:a", "with no Focus the D-pad focuses the first Button");
    gamepad((GamepadSample){.right = true});
    gamepad((GamepadSample){.confirm = true});
    gamepad((GamepadSample){.confirm = true});
    gamepad((GamepadSample){0});
    expect_events("blur:a focus:b pressstart:b pressend:b press:b", "Cross presses the focused Button once");

    instance_tree_clear();
}

static void test_pad_buttons(void)
{
    build_tree();
    reset();

    pad(PAD_CIRCLE, true);
    pad(PAD_CIRCLE, true);
    pad(PAD_CIRCLE, false);
    expect_events("pad:circle:pressstart pad:circle:pressend", "a held Pad Button starts and ends once");

    GamepadSample both = {0};
    both.pad_buttons[PAD_TRIANGLE] = true;
    both.pad_buttons[PAD_R] = true;
    gamepad(both);
    gamepad((GamepadSample){0});
    expect_events("pad:triangle:pressstart pad:r:pressstart pad:triangle:pressend pad:r:pressend",
                  "Pad Buttons are independent");

    pad(PAD_SQUARE, true);
    pad(PAD_L, true);
    input_ignore_held_pad_buttons();
    pad(PAD_L, true);
    pad(PAD_L, false);
    expect_events("pad:square:pressstart pad:square:pressend pad:l:pressstart pad:l:pressend",
                  "a press already reported still ends after an ignore");

    input_ignore_held_pad_buttons();
    pad(PAD_CIRCLE, true);
    pad(PAD_CIRCLE, true);
    pad(PAD_CIRCLE, false);
    expect_events("", "a button held when the Shell hides is ignored until released");
    pad(PAD_CIRCLE, true);
    expect_events("pad:circle:pressstart", "the next press after the ignored one is reported");

    input_clear_focus();
    pad(PAD_CIRCLE, true);
    pad(PAD_CIRCLE, false);
    expect_events("", "a runtime restart forgets held Pad Buttons without ending them");

    gamepad((GamepadSample){.confirm = true});
    gamepad((GamepadSample){.up = true, .down = true});
    expect_events("focus:a", "Cross and the D-pad never reach Pad Buttons");
    gamepad((GamepadSample){0});
    drain();

    instance_tree_clear();
}

static void test_auto_focus(void)
{
    ReactInstance *a = build_tree();
    reset();

    instance_back_request_focus("b");
    gamepad((GamepadSample){0});
    expect_events("", "a request waits for the next snapshot");
    fixture_publish();
    gamepad((GamepadSample){0});
    expect_events("focus:b", "a published request focuses the Button");
    gamepad((GamepadSample){0});
    expect_events("", "a request is taken once");

    instance_back_request_focus("b");
    instance_back_request_focus("a");
    fixture_publish();
    gamepad((GamepadSample){0});
    expect_events("blur:b focus:a", "the last request before a swap wins");

    instance_back_request_focus("a");
    fixture_destroy(a);
    fixture_publish();
    gamepad((GamepadSample){0});
    expect_events("", "a request for a removed instance is dropped, and so is its Focus, without a blur");
    expect(!input_is_focused("a"), "Focus on a removed instance is gone");

    instance_tree_clear();
}

int main(void)
{
    instance_tree_init();
    if (!event_queue_init()) {
        fprintf(stderr, "input_harness: event queue init failed\n");
        return 1;
    }

    test_hover_focus();
    test_press();
    test_pad_buttons();
    test_auto_focus();

    reset();
    event_queue_destroy();

    if (failures) {
        fprintf(stderr, "input_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("input_harness: ok\n");
    return 0;
}
