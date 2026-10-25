#define STB_DS_IMPLEMENTATION
#include "instance_fixture.h"
#include "core/event_queue.h"
#include "ui/input.h"

#include <raylib.h>
#include <stdio.h>

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "input_harness: %s\n", message);
    failures++;
}

// Drains the queue as space-separated "event:id" for instance events, "pad:button:phase" for Pad Buttons and
// "text:id:value" for confirmed Text Input values.
static const char *drain(void)
{
    static char out[512];
    out[0] = '\0';
    size_t len = 0;
    InputEvent evt;
    while (event_queue_pop(&evt)) {
        const char *sep = len > 0 ? " " : "";
        int n = 0;
        if (evt.type == EVT_PAD_BUTTON)
            n = snprintf(out + len, sizeof(out) - len, "%spad:%s:%s", sep, evt.id, evt.event_name);
        else if (evt.type == EVT_TEXT_INPUT)
            n = snprintf(out + len, sizeof(out) - len, "%stext:%s:%s", sep, evt.id, evt.text);
        else
            n = snprintf(out + len, sizeof(out) - len, "%s%s:%s", sep, evt.event_name, evt.id);
        free(evt.text);
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

// One frame of host keyboard editing: typed UTF-8, then Backspace, Enter or Escape.
static void keys(const char *typed, bool backspace, bool confirm, bool cancel)
{
    int codepoints[64];
    int count = 0;
    for (const char *p = typed; *p != '\0' && count < 64;) {
        int size = 0;
        codepoints[count++] = GetCodepointNext(p, &size);
        p += size;
    }
    const VdTextInputKeys sample = {
        .codepoints = codepoints,
        .codepoint_count = count,
        .backspace = backspace,
        .confirm = confirm,
        .cancel = cancel,
    };
    input_apply_text_input(vd_text_input_apply_keys(&sample));
}

static void type(const char *typed)
{
    keys(typed, false, false, false);
}

static void expect_editing(const char *text, const char *message)
{
    const char *got = input_editing_text("t");
    if (!got || strcmp(got, text) != 0) {
        fprintf(stderr, "input_harness: editing \"%s\", want \"%s\"\n", got ? got : "(none)", text);
        expect(false, message);
    }
}

// Text Input t (value "hi", at most 5 UTF-16 code units) left of Button a.
static ReactInstance *build_text_input_tree(void)
{
    ReactInstance *t = fixture_text_input(NULL, "t", 0, 0, 200, 40, "hi", 5);
    fixture_button(NULL, "a", 300, 0, 40, 40);
    fixture_publish();
    return t;
}

static void test_text_input_editing(void)
{
    build_text_input_tree();
    reset();

    mouse(10, 10, false);
    expect_events("focus:t", "hovering a Text Input focuses it");
    mouse(10, 10, true);
    expect(!input_is_editing_text(), "a Text Input does not edit before the Press ends");
    mouse(10, 10, false);
    expect_events("pressstart:t pressend:t press:t", "a Text Input is pressed like a Button");
    expect(input_is_editing_text(), "a Press on a Text Input starts editing");
    expect_editing("hi", "editing starts from the value");
    expect(input_editing_text("a") == NULL, "only the edited Text Input shows editing text");

    type("!é");
    expect_editing("hi!é", "typed characters append, UTF-8 included");
    type("xyz");
    expect_editing("hi!éx", "typing stops at maxLength");
    keys("", true, false, false);
    keys("", true, false, false);
    expect_editing("hi!", "Backspace deletes whole characters");
    expect_events("", "editing reports nothing before it ends");

    keys("", false, true, false);
    expect_events("text:t:hi!", "Enter confirms once with the edited value");
    expect(!input_is_editing_text(), "confirming ends editing");
    expect(input_editing_text("t") == NULL, "no editing text after the edit");
    expect(input_is_focused("t"), "the Text Input keeps Focus after the edit");

    gamepad((GamepadSample){.confirm = true});
    gamepad((GamepadSample){0});
    expect_events("", "the Enter that confirmed does not press again");
    gamepad((GamepadSample){.confirm = true});
    gamepad((GamepadSample){0});
    expect_events("pressstart:t pressend:t press:t", "the next Cross presses the Text Input again");
    expect_editing("hi", "a new edit starts from the value the Deck App holds");

    type("zz");
    keys("q", false, false, true);
    expect_events("", "Escape cancels without a change");
    expect(!input_is_editing_text(), "cancelling ends editing");

    instance_tree_clear();
}

static void test_text_input_ends(void)
{
    ReactInstance *t = build_text_input_tree();
    reset();

    gamepad((GamepadSample){.right = true});
    gamepad((GamepadSample){.confirm = true});
    gamepad((GamepadSample){0});
    drain();
    expect(input_is_editing_text(), "Cross on a focused Text Input starts editing");
    input_cancel_text_input();
    expect(!input_is_editing_text(), "opening the Shell cancels editing");
    keys("", false, true, false);
    expect_events("", "keys after a cancel reach nothing");

    gamepad((GamepadSample){0});
    gamepad((GamepadSample){.confirm = true});
    gamepad((GamepadSample){0});
    drain();
    input_clear_focus();
    expect(!input_is_editing_text(), "a runtime restart cancels editing");
    expect_events("blur:t", "a runtime restart reports no value, only the blur");

    gamepad((GamepadSample){.right = true});
    gamepad((GamepadSample){.confirm = true});
    gamepad((GamepadSample){0});
    drain();
    fixture_destroy(t);
    fixture_publish();
    type("x");
    expect(!input_is_editing_text(), "removing the Text Input ends editing");
    expect_events("", "a removed Text Input reports no value");

    instance_tree_clear();
}

static void test_text_input_limits(void)
{
    fixture_text_input(NULL, "t", 0, 0, 200, 40, "abcdef", 3);
    fixture_publish();
    reset();

    mouse(10, 10, true);
    mouse(10, 10, false);
    expect_editing("abc", "a value longer than maxLength is cut when editing starts");
    keys("", false, false, true);

    instance_tree_clear();
    fixture_text_input(NULL, "t", 0, 0, 200, 40, "", 2);
    fixture_publish();
    reset();

    mouse(10, 10, true);
    mouse(10, 10, false);
    type("\t\x7f");
    expect_editing("", "control characters are not typed");
    type("\xf0\x9f\x98\x80" "a");
    expect_editing("\xf0\x9f\x98\x80", "an emoji counts as two code units, like a JS string");
    keys("", true, false, false);
    type("ab");
    keys("", false, true, false);
    expect_events("focus:t pressstart:t pressend:t press:t text:t:ab", "the value is confirmed once");

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
    test_text_input_editing();
    test_text_input_ends();
    test_text_input_limits();

    reset();
    event_queue_destroy();

    if (failures) {
        fprintf(stderr, "input_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("input_harness: ok\n");
    return 0;
}
