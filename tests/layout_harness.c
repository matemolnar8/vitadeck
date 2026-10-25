#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "ui/layout.h"

#include <stdio.h>
#include <stdlib.h>

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "layout_harness: %s\n", message);
    failures++;
}

static void expect_at(const ReactInstance *inst, int x, int y, const char *message)
{
    if (inst->layout.x == x && inst->layout.y == y) return;
    fprintf(stderr, "layout_harness: at (%d, %d), want (%d, %d)\n", inst->layout.x, inst->layout.y, x, y);
    expect(false, message);
}

static void expect_size(const ReactInstance *inst, int width, int height, const char *message)
{
    if (inst->layout.width == width && inst->layout.height == height) return;
    fprintf(stderr, "layout_harness: size %dx%d, want %dx%d\n", inst->layout.width, inst->layout.height, width,
            height);
    expect(false, message);
}

static ReactInstance *add(ReactInstance *parent, ReactInstance node)
{
    ReactInstance *inst = malloc(sizeof(*inst));
    *inst = node;
    if (parent) {
        inst->parent = parent;
        arrput(parent->children, inst);
    }
    return inst;
}

static ReactInstance *rect(ReactInstance *parent, int x, int y, int width, int height)
{
    return add(parent, (ReactInstance){.type = NT_RECT, .props.rect = {.x = x, .y = y, .width = width, .height = height}});
}

static ReactInstance *button(ReactInstance *parent, int x, int y, int width, int height)
{
    return add(parent,
               (ReactInstance){.type = NT_BUTTON, .props.button = {.x = x, .y = y, .width = width, .height = height}});
}

static ReactInstance *stack(ReactInstance *parent, StackProps props)
{
    return add(parent, (ReactInstance){.type = NT_STACK, .props.stack = props});
}

static void free_tree(ReactInstance *inst)
{
    for (ptrdiff_t i = 0; i < arrlen(inst->children); i++) {
        free_tree(inst->children[i]);
    }
    arrfree(inst->children);
    free(inst);
}

static void run_layout(ReactInstance *root)
{
    ReactInstance **roots = NULL;
    arrput(roots, root);
    layout_tree(roots);
    arrfree(roots);
}

static void test_column(void)
{
    ReactInstance *s = stack(NULL, (StackProps){.x = 10, .y = 20, .gap = 5, .padding = 4});
    ReactInstance *r = rect(s, 0, 0, 100, 30);
    ReactInstance *b = button(s, 2, 3, 50, 20);
    ReactInstance *img =
        add(s, (ReactInstance){.type = NT_IMAGE, .props.image = {.x = 0, .y = 0, .width = 40, .height = 10}});
    run_layout(s);

    expect_at(s, 10, 20, "a root stack sits at its own x/y");
    expect_at(r, 4, 4, "the first child starts inside the padding");
    // 4 + (30 + 5) + its margin 3.
    expect_at(b, 6, 42, "y is a leading margin and x an offset from the start");
    expect_at(img, 4, 67, "an image follows the button and its gap");
    // Widest child 2 + 50 = 52 is narrower than the rect; 67 + 10 + 4 of padding.
    expect_size(s, 108, 81, "a stack without width or height fits its content");
    expect(s->layout.content_height == 81, "a column's content height is its flowed height");
    free_tree(s);

    ReactInstance *empty = stack(NULL, (StackProps){.padding = 6, .gap = 9});
    run_layout(empty);
    expect_size(empty, 12, 12, "an empty stack is as large as its padding");
    free_tree(empty);
}

static void test_row_center(void)
{
    ReactInstance *s = stack(NULL, (StackProps){.direction = STACK_ROW, .gap = 10, .align = STACK_ALIGN_CENTER});
    ReactInstance *b1 = button(s, 0, 0, 180, 40);
    ReactInstance *b2 = button(s, 0, 0, 180, 60);
    ReactInstance *r = rect(s, 5, 2, 20, 20);
    run_layout(s);

    expect_at(b1, 0, 10, "a shorter child is centered on the cross axis");
    expect_at(b2, 190, 0, "the tallest child sets the cross size");
    // 190 + 180 + 10 + its margin 5; centered at (60 - 20) / 2 plus its offset 2.
    expect_at(r, 385, 22, "x is a leading margin and y an offset from the centered position in a row");
    expect_size(s, 405, 60, "a row fits its width to the flow and its height to the tallest child");
    free_tree(s);
}

static void test_align_end(void)
{
    ReactInstance *s = stack(NULL, (StackProps){.width = 200, .padding = 10, .align = STACK_ALIGN_END});
    ReactInstance *r = rect(s, 0, 0, 50, 10);
    ReactInstance *offset = rect(s, -5, 0, 50, 10);
    run_layout(s);

    expect_at(r, 140, 10, "end alignment uses the stated width inside the padding");
    expect_at(offset, 135, 20, "a cross offset moves the child from its aligned position");
    expect_size(s, 200, 40, "a stated width is kept while the height fits");
    free_tree(s);
}

static void test_nested(void)
{
    ReactInstance *outer = stack(NULL, (StackProps){.x = 20, .y = 20, .gap = 8});
    rect(outer, 0, 0, 300, 50);
    ReactInstance *row = stack(outer, (StackProps){.direction = STACK_ROW, .gap = 4});
    ReactInstance *b1 = button(row, 0, 0, 100, 30);
    ReactInstance *b2 = button(row, 0, 0, 100, 30);
    ReactInstance *last = rect(outer, 0, 0, 10, 10);
    run_layout(outer);

    expect_size(row, 204, 30, "a nested stack is sized before its parent places it");
    expect_at(row, 0, 58, "a nested stack flows like any child");
    expect_at(b1, 0, 0, "children of a nested stack are relative to it");
    expect_at(b2, 104, 0, "a nested row places its own children");
    expect_at(last, 0, 96, "the flow continues after the nested stack");
    expect_size(outer, 300, 106, "the outer stack fits the nested one");
    free_tree(outer);
}

static void test_unflowed_children(void)
{
    ReactInstance *s = stack(NULL, (StackProps){.gap = 5, .padding = 4});
    ReactInstance *text = add(s, (ReactInstance){.type = NT_TEXT, .props.text = {.x = 3, .y = 7}});
    ReactInstance *raw = add(text, (ReactInstance){.type = NT_RAW_TEXT});
    ReactInstance *panel = rect(s, 0, 0, 100, 40);
    ReactInstance *inner = rect(panel, 12, 9, 10, 10);
    run_layout(s);

    expect_at(text, 7, 11, "text keeps its own x/y from inside the padding");
    expect_at(raw, 0, 0, "raw text has no position of its own");
    expect_at(panel, 4, 4, "text does not take space in the flow");
    expect_at(inner, 12, 9, "a rect's children sit at their own x/y");
    free_tree(s);
}

// The smoke Deck App's Scroll: x 20, y 82, 920x404, gap 8, padding 10, with three 64px rows, a 72px image row
// and two 36px buttons. The flow before Stack placed them at 10, 82, 154, 226, 306 and 350 with content height
// 6 * 8 + 3 * 64 + 72 + 2 * 36 - 8 + 2 * 10 = 396.
static void test_scroll_matches_previous_flow(void)
{
    ReactInstance *scroll = add(NULL, (ReactInstance){.type = NT_SCROLL,
                                                      .props.stack = {.x = 20,
                                                                      .y = 82,
                                                                      .width = 920,
                                                                      .height = 404,
                                                                      .gap = 8,
                                                                      .padding = 10}});
    ReactInstance *rows[6];
    for (int i = 0; i < 3; i++) {
        rows[i] = rect(scroll, 0, 0, 900, 64);
    }
    rows[3] = rect(scroll, 0, 0, 900, 72);
    ReactInstance *title = add(rows[3], (ReactInstance){.type = NT_TEXT, .props.text = {.x = 10, .y = 6}});
    rows[4] = button(scroll, 0, 0, 220, 36);
    rows[5] = button(scroll, 0, 0, 220, 36);
    ReactInstance *nested = add(scroll, (ReactInstance){.type = NT_SCROLL,
                                                        .props.stack = {.x = 110, .y = 3, .width = 80, .height = 40}});
    run_layout(scroll);

    const int want_y[6] = {10, 82, 154, 226, 306, 350};
    for (int i = 0; i < 6; i++) {
        expect_at(rows[i], 10, want_y[i], "scroll children land where the previous flow put them");
    }
    expect_at(title, 10, 6, "text inside a row keeps its own x/y");
    expect_at(nested, 120, 13, "a nested scroll stays out of the flow, inside the padding");
    expect_at(scroll, 20, 82, "a scroll sits at its own x/y");
    expect_size(scroll, 920, 404, "a scroll keeps its viewport size");
    expect(scroll->layout.content_height == 396, "scroll content height matches the previous flow");
    free_tree(scroll);
}

int main(void)
{
    test_column();
    test_row_center();
    test_align_end();
    test_nested();
    test_unflowed_children();
    test_scroll_matches_previous_flow();

    if (failures) {
        fprintf(stderr, "layout_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("layout_harness: ok\n");
    return 0;
}
