#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "ui/instance_tree.h"
#include "ui/scroll.h"

#include <stdio.h>

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "scroll_math_harness: %s\n", message);
    failures++;
}

static void test_flow_step(void)
{
    ReactInstance rect = {.type = NT_RECT, .props.rect = {.y = 3, .height = 20}};
    ReactInstance button = {.type = NT_BUTTON, .props.button = {.y = 0, .height = 10}};
    ReactInstance image = {.type = NT_IMAGE, .props.image = {.y = 2, .height = 10}};
    ReactInstance text = {.type = NT_TEXT, .props.text = {.y = 50}};
    ReactInstance nested = {.type = NT_SCROLL, .props.scroll = {.y = 50, .height = 50}};

    int flow = 0, base = -1;
    expect(scroll_flow_step(&rect, 5, &flow, &base) && base == 0 && flow == 28,
           "a rect is placed at the flow start and advances by margin + height + gap");
    expect(scroll_flow_step(&button, 5, &flow, &base) && base == 28 && flow == 43, "a button follows the rect");
    expect(scroll_flow_step(&image, 5, &flow, &base) && base == 43 && flow == 60, "an image is part of the flow");
    base = -1;
    expect(!scroll_flow_step(&text, 5, &flow, &base) && base == -1 && flow == 60, "text is not part of the flow");
    expect(!scroll_flow_step(&nested, 5, &flow, &base) && flow == 60, "a nested scroll is not part of the flow");
    expect(!scroll_flow_step(NULL, 5, &flow, &base) && flow == 60, "a missing child is skipped");

    ReactInstance scroll = {.type = NT_SCROLL, .props.scroll = {.height = 40, .gap = 5, .padding = 4}};
    expect(scroll_content_height(&scroll) == 8, "an empty scroll is as tall as its padding");
    arrput(scroll.children, &rect);
    arrput(scroll.children, &text);
    arrput(scroll.children, &button);
    arrput(scroll.children, &image);
    // 60 of flow, minus the trailing gap, plus padding on both sides.
    expect(scroll_content_height(&scroll) == 63, "content height drops the last gap and adds padding");
    arrfree(scroll.children);

    expect(scroll_content_height(&rect) == 0, "a non-scroll has no content height");
    expect(scroll_content_height(NULL) == 0, "no instance has no content height");
}

static void test_max_offset(void)
{
    expect(scroll_max_offset(200, 500) == 300, "max offset is content minus viewport");
    expect(scroll_max_offset(200, 200) == 0, "content that exactly fits does not scroll");
    expect(scroll_max_offset(200, 150) == 0, "short content does not scroll");
}

static void expect_step(int offset, int max_offset, float velocity, float dt, int want_offset, float want_velocity,
                        const char *message)
{
    float v = velocity;
    int next = scroll_momentum_step(offset, max_offset, &v, dt);
    if (next != want_offset || v != want_velocity) {
        fprintf(stderr, "scroll_math_harness: step(%d, %d, %g, %g) = %d, %g; want %d, %g\n", offset, max_offset,
                velocity, dt, next, v, want_offset, want_velocity);
        expect(false, message);
    }
}

static void test_momentum_step(void)
{
    const float frame = 1.0f / 60.0f;
    expect_step(100, 300, 600.0f, frame, 110, 600.0f, "momentum moves by velocity * dt and keeps its speed");
    expect_step(100, 300, -600.0f, frame, 90, -600.0f, "negative velocity scrolls back up");
    expect_step(100, 300, 600.0f, 0.1f, 160, 600.0f, "a longer frame moves further");
    expect_step(100, 300, 30.0f, frame, 101, 30.0f, "half a pixel or more moves one pixel");
    expect_step(100, 300, 20.0f, frame, 100, 20.0f, "less than half a pixel holds the offset");
    expect_step(5, 300, -600.0f, frame, 0, 0.0f, "overshooting the top clamps and stops");
    expect_step(10, 300, -600.0f, frame, 0, 0.0f, "landing exactly on the top stops");
    expect_step(295, 300, 600.0f, frame, 300, 0.0f, "overshooting the bottom clamps and stops");
    expect_step(0, 0, 600.0f, frame, 0, 0.0f, "content that fits never moves");
}

static void test_offset_store(void)
{
    expect(scroll_get_offset("list") == 0, "an unknown scroll is at the top");
    expect(scroll_get_offset(NULL) == 0, "no id is at the top");
    scroll_set_offset("list", 42);
    scroll_set_offset("other", -3);
    expect(scroll_get_offset("list") == 42, "offsets are stored per id");
    expect(scroll_get_offset("other") == 0, "negative offsets are stored as zero");
    scroll_reset();
    expect(scroll_get_offset("list") == 0, "reset drops every offset");
    scroll_reset();
}

int main(void)
{
    test_flow_step();
    test_max_offset();
    test_momentum_step();
    test_offset_store();

    if (failures) {
        fprintf(stderr, "scroll_math_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("scroll_math_harness: ok\n");
    return 0;
}
