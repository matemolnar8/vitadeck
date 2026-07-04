#include <math.h>
#include <stdio.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/event_queue.h"
#include "ui/input.h"

static int fail(const char *message)
{
    fprintf(stderr, "input_touch_harness: %s\n", message);
    return 1;
}

int main(void)
{
    if (!event_queue_init()) return fail("could not initialize event queue");

    input_test_reset_state();
    input_test_set_touch_hovered("scroll");
    if (!input_is_hovered("scroll")) return fail("touch hover should still report hover");
    if (input_is_focused("scroll")) return fail("touch hover must not report focus");

    input_test_set_focused("scroll");
    if (!input_is_focused("scroll")) return fail("gamepad/keyboard focus should report focus");

    input_test_reset_state();
    input_test_begin_touch_scroll_release("scroll", 600.0f);
    input_test_release_touch_scroll();
    if (!input_test_momentum_active("scroll")) return fail("released fling should arm momentum");
    if (fabsf(input_test_momentum_velocity()) < 1.0f) return fail("released fling should preserve velocity");

    input_test_reset_state();
    event_queue_destroy();
    printf("input_touch_harness: ok\n");
    return 0;
}
