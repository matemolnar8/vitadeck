#define STB_DS_IMPLEMENTATION
#include "instance_fixture.h"
#include "core/event_queue.h"
#include "ui/input.h"
#include "ui/scroll.h"

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
    pad(PAD_CIRCLE, true);
    pad(PAD_CIRCLE, false);
    expect_events("pad:circle:pressstart pad:circle:pressend",
                  "a button held through a runtime start presses once, without ending the old press");

    pad(PAD_SQUARE, true);
    drain();
    input_ignore_held_pad_buttons();
    input_clear_focus();
    pad(PAD_SQUARE, true);
    pad(PAD_SQUARE, false);
    expect_events("", "a button held when the Shell hides to restart the runtime stays ignored");

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

static void expect_offset(const char *scroll_id, int want, const char *message)
{
    const int got = scroll_get_offset(scroll_id);
    if (got != want) {
        fprintf(stderr, "input_harness: %s offset %d, want %d\n", scroll_id, got, want);
        expect(false, message);
    }
}

static void request_focus(const char *id)
{
    instance_back_request_focus(id);
    fixture_publish();
    gamepad((GamepadSample){0});
}

static void down(void)
{
    gamepad((GamepadSample){.down = true});
}

static void up(void)
{
    gamepad((GamepadSample){.up = true});
}

// Scroll S 300x120 with gap 10: s0..s3 are 40 high at content y 0, 50, 100 and 150, so the offset goes up to 70.
// The Scrolls here are wider than their buttons so that the D-pad, which can focus a Scroll too, prefers the buttons.
static void test_focus_scrolls_into_view(void)
{
    ReactInstance *s = fixture_scroll(NULL, "S", 0, 0, 300, 120, 10, 0);
    fixture_button(s, "s0", 0, 0, 100, 40);
    fixture_button(s, "s1", 0, 0, 100, 40);
    fixture_button(s, "s2", 0, 0, 100, 40);
    fixture_button(s, "s3", 0, 0, 100, 40);
    fixture_publish();
    reset();

    request_focus("s0");
    down();
    expect_events("focus:s0 blur:s0 focus:s1", "the D-pad moves to the visible s1");
    expect_offset("S", 0, "a visible control keeps the offset");
    down();
    expect_events("blur:s1 focus:s2", "the D-pad moves to the partly hidden s2");
    expect_offset("S", 20, "the D-pad scrolls just enough to show s2's bottom edge");
    down();
    expect_events("blur:s2 focus:s3", "the D-pad reaches the fully hidden s3");
    expect_offset("S", 70, "the D-pad scrolls s3 into view");
    down();
    expect_events("", "Down at the end of the Scroll keeps Focus");
    up();
    expect_events("blur:s3 focus:s2", "the D-pad moves back up");
    expect_offset("S", 70, "moving back to a visible control keeps the offset");
    up();
    up();
    expect_events("blur:s2 focus:s1 blur:s1 focus:s0", "the D-pad walks back to s0");
    expect_offset("S", 0, "the D-pad scrolls just enough to show s0's top edge");

    down();
    down();
    drain();
    mouse(10, 5, false);
    expect_events("blur:s2 focus:s0", "hovering the partly hidden s0 focuses it");
    expect_offset("S", 20, "hover does not scroll, so the content stays under the pointer");

    instance_tree_clear();
    scroll_reset();
}

// Scroll T 300x80 holds t0..t3, 40 high each: t0 and t1 fill the viewport exactly and t2, t3 are fully hidden.
static void test_focus_reaches_tiled_rows(void)
{
    ReactInstance *t = fixture_scroll(NULL, "T", 0, 0, 300, 80, 0, 0);
    fixture_button(t, "t0", 0, 0, 100, 40);
    fixture_button(t, "t1", 0, 0, 100, 40);
    fixture_button(t, "t2", 0, 0, 100, 40);
    fixture_button(t, "t3", 0, 0, 100, 40);
    fixture_publish();
    reset();

    request_focus("t0");
    down();
    drain();
    expect_offset("T", 0, "t1 is already visible");
    down();
    expect_events("blur:t1 focus:t2", "Down from the last visible row reaches the hidden t2");
    expect_offset("T", 40, "t2 scrolls into view");
    down();
    expect_events("blur:t2 focus:t3", "Down again reaches t3");
    expect_offset("T", 80, "t3 scrolls into view");
    up();
    up();
    expect_events("blur:t3 focus:t2 blur:t2 focus:t1", "Up walks back to t1");
    expect_offset("T", 40, "t1 scrolls back into view");
    up();
    expect_events("blur:t1 focus:t0", "Up reaches t0");
    expect_offset("T", 0, "t0 scrolls back into view");

    instance_tree_clear();
    scroll_reset();
}

// A column Stack holds Scroll U (150x80, u0..u3 40 high each) and, 10 below it, Button below. Hidden rows of U lie
// over below's position, so the D-pad must prefer the nearer row and only then leave the Scroll.
static void test_focus_leaves_scroll(void)
{
    ReactInstance *stack = fixture_stack(NULL, "C", 0, 0, STACK_COLUMN, 10);
    ReactInstance *u = fixture_scroll(stack, "U", 0, 0, 150, 80, 0, 0);
    fixture_button(u, "u0", 0, 0, 100, 40);
    fixture_button(u, "u1", 0, 0, 100, 40);
    fixture_button(u, "u2", 0, 0, 100, 40);
    fixture_button(u, "u3", 0, 0, 100, 40);
    fixture_button(stack, "below", 0, 0, 100, 40);
    fixture_publish();
    reset();

    request_focus("u1");
    drain();
    down();
    expect_events("blur:u1 focus:u2", "the hidden u2 is nearer than the Button below the Scroll");
    down();
    expect_events("blur:u2 focus:u3", "the hidden u3 is nearer too");
    expect_offset("U", 80, "u3 scrolls into view");
    down();
    expect_events("blur:u3 focus:below", "Down from the last row leaves the Scroll");
    expect_offset("U", 80, "leaving the Scroll keeps its offset");
    up();
    expect_events("blur:below focus:u3", "Up comes back to the nearest row");

    instance_tree_clear();
    scroll_reset();
}

// Same Stack as test_focus_leaves_scroll, at offset 0: hidden u2 and u3 lie below Button below, so Down from
// below must not jump to them; Up enters at the last visible row instead.
static void test_focus_outside_scroll_below(void)
{
    ReactInstance *stack = fixture_stack(NULL, "C", 0, 0, STACK_COLUMN, 10);
    ReactInstance *u = fixture_scroll(stack, "U", 0, 0, 150, 80, 0, 0);
    fixture_button(u, "u0", 0, 0, 100, 40);
    fixture_button(u, "u1", 0, 0, 100, 40);
    fixture_button(u, "u2", 0, 0, 100, 40);
    fixture_button(u, "u3", 0, 0, 100, 40);
    fixture_button(stack, "below", 0, 0, 100, 40);
    fixture_publish();
    reset();

    request_focus("below");
    drain();
    down();
    expect_events("", "Down from the Button below a Scroll does not jump to a hidden row");
    expect_offset("U", 0, "the Scroll does not move when Down finds nothing below");
    expect(input_is_focused("below"), "Focus stays on the Button below the Scroll");
    up();
    expect_events("blur:below focus:u1", "Up from below enters at the last visible row");
    expect_offset("U", 0, "entering at a visible row keeps the offset");

    instance_tree_clear();
    scroll_reset();
}

// Header Button over Scroll L (300x160, r0..r9 40 high each) in a column Stack, scrolled to the bottom
// (offset 240): r6 is the first visible row. Up from the header finds nothing; Down enters at r6 without
// shifting the list.
static void test_focus_outside_scroll_header(void)
{
    ReactInstance *stack = fixture_stack(NULL, "H", 0, 0, STACK_COLUMN, 0);
    fixture_button(stack, "header", 0, 0, 100, 40);
    ReactInstance *l = fixture_scroll(stack, "L", 0, 0, 300, 160, 0, 0);
    fixture_button(l, "r0", 0, 0, 100, 40);
    fixture_button(l, "r1", 0, 0, 100, 40);
    fixture_button(l, "r2", 0, 0, 100, 40);
    fixture_button(l, "r3", 0, 0, 100, 40);
    fixture_button(l, "r4", 0, 0, 100, 40);
    fixture_button(l, "r5", 0, 0, 100, 40);
    fixture_button(l, "r6", 0, 0, 100, 40);
    fixture_button(l, "r7", 0, 0, 100, 40);
    fixture_button(l, "r8", 0, 0, 100, 40);
    fixture_button(l, "r9", 0, 0, 100, 40);
    fixture_publish();
    reset();

    scroll_set_offset("L", 240);
    request_focus("header");
    drain();
    expect_offset("L", 240, "focusing the header keeps the list scrolled to the bottom");
    up();
    expect_events("", "Up from the header above a scrolled list is a no-op");
    expect_offset("L", 240, "the no-op Up keeps the offset");
    expect(input_is_focused("header"), "Focus stays on the header");
    down();
    expect_events("blur:header focus:r6", "Down from the header enters at the first visible row");
    expect_offset("L", 240, "entering at the first visible row does not shift the list");

    instance_tree_clear();
    scroll_reset();
}

// Scroll O 100x100 flows o0 (80 high), Scroll I (60 high) and o1 (40 high), so its offset goes up to 80.
// I holds i0..i3, 40 high each, so its offset goes up to 100.
static void test_nested_scrolls_into_view(void)
{
    ReactInstance *o = fixture_scroll(NULL, "O", 0, 0, 100, 100, 0, 0);
    fixture_button(o, "o0", 0, 0, 100, 80);
    ReactInstance *i = fixture_scroll(o, "I", 0, 0, 100, 60, 0, 0);
    fixture_button(i, "i0", 0, 0, 100, 40);
    fixture_button(i, "i1", 0, 0, 100, 40);
    fixture_button(i, "i2", 0, 0, 100, 40);
    fixture_button(i, "i3", 0, 0, 100, 40);
    fixture_button(o, "o1", 0, 0, 100, 40);
    fixture_publish();
    reset();

    request_focus("i2");
    expect_offset("I", 60, "the inner Scroll shows i2 first");
    expect_offset("O", 40, "then the outer Scroll shows where i2 ended up");

    request_focus("o0");
    expect_offset("O", 0, "the outer Scroll scrolls back for o0");
    expect_offset("I", 60, "a Scroll that does not contain the focused control keeps its offset");

    instance_tree_clear();
    scroll_reset();
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
    type("\xf0\x9f\x98\x80"
         "a");
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
    test_focus_scrolls_into_view();
    test_focus_reaches_tiled_rows();
    test_focus_leaves_scroll();
    test_focus_outside_scroll_below();
    test_focus_outside_scroll_header();
    test_nested_scrolls_into_view();
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
