#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "ui/layout.h"
#include "ui/text.h"

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

static ReactInstance *text(ReactInstance *parent, TextProps props, const char *content)
{
    ReactInstance *inst = add(parent, (ReactInstance){.type = NT_TEXT, .props.text = props});
    add(inst, (ReactInstance){.type = NT_RAW_TEXT, .props.raw_text = (char *)content});
    return inst;
}

static ReactInstance *stack(ReactInstance *parent, StackProps props)
{
    return add(parent, (ReactInstance){.type = NT_STACK, .props.stack = props});
}

static ReactInstance *scroll(ReactInstance *parent, StackProps props)
{
    return add(parent, (ReactInstance){.type = NT_SCROLL, .props.stack = props});
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

// A Scroll at x 20, y 82, 920x404, gap 8, padding 10, with three 64px rows, a 72px image row
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
    rows[4] = button(scroll, 0, 0, 220, 36);
    rows[5] = button(scroll, 0, 0, 220, 36);
    run_layout(scroll);

    const int want_y[6] = {10, 82, 154, 226, 306, 350};
    for (int i = 0; i < 6; i++) {
        expect_at(rows[i], 10, want_y[i], "scroll children land where the previous flow put them");
    }
    expect_at(scroll, 20, 82, "a scroll sits at its own x/y");
    expect_size(scroll, 920, 404, "a scroll keeps its viewport size");
    expect(scroll->layout.content_height == 396, "scroll content height matches the previous flow");
    free_tree(scroll);
}

static void test_scroll_in_column(void)
{
    ReactInstance *col = stack(NULL, (StackProps){.width = 300, .height = 400, .gap = 10, .padding = 5});
    rect(col, 0, 0, 290, 50);
    ReactInstance *list = scroll(col, (StackProps){.x = 2, .y = 3, .width = 200, .height = 120});
    rect(list, 0, 0, 200, 500);
    ReactInstance *after = rect(col, 0, 0, 10, 10);
    run_layout(col);

    // 5 + 50 + 10 + its margin 3.
    expect_at(list, 7, 68, "a scroll flows like any child, with a margin and an offset");
    expect_size(list, 200, 120, "a scroll keeps its stated height in a column");
    expect(list->layout.content_height == 500, "its content height is still its own flow");
    expect_at(after, 5, 198, "the flow continues after the scroll's stated height");
    free_tree(col);
}

static void test_scroll_fills_rest(void)
{
    ReactInstance *col = stack(NULL, (StackProps){.height = 400, .gap = 10, .padding = 5});
    rect(col, 0, 0, 290, 50);
    ReactInstance *list = scroll(col, (StackProps){.y = 4, .width = 290, .gap = 8});
    ReactInstance *first = rect(list, 0, 0, 290, 300);
    ReactInstance *second = rect(list, 0, 0, 290, 300);
    run_layout(col);

    expect_at(list, 5, 69, "a filling scroll still takes its margin");
    // 400 - 2 * 5 of padding - (50 + 10) - its margin 4.
    expect_size(list, 290, 326, "the last scroll without a height fills the rest of the column");
    expect(list->layout.content_height == 608, "a filling scroll still scrolls its full content");
    expect_at(first, 0, 0, "a filling scroll places its own children from its top");
    expect_at(second, 0, 308, "and flows them with its gap");
    expect_size(col, 300, 400, "the column keeps its stated height and fits its width");
    free_tree(col);

    ReactInstance *fit = stack(NULL, (StackProps){.gap = 10});
    rect(fit, 0, 0, 100, 50);
    ReactInstance *fitted = scroll(fit, (StackProps){.width = 100});
    rect(fitted, 0, 0, 100, 30);
    run_layout(fit);
    expect_size(fitted, 100, 30, "in a parent without a height there is no rest, so the scroll fits its content");
    free_tree(fit);

    ReactInstance *crowded = stack(NULL, (StackProps){.height = 40});
    rect(crowded, 0, 0, 100, 60);
    ReactInstance *squeezed = scroll(crowded, (StackProps){.width = 100});
    rect(squeezed, 0, 0, 100, 30);
    run_layout(crowded);
    expect_size(squeezed, 100, 0, "with nothing left the filling scroll is empty, not negative");
    free_tree(crowded);

    ReactInstance *middle = stack(NULL, (StackProps){.height = 400});
    ReactInstance *not_last = scroll(middle, (StackProps){.width = 100});
    rect(not_last, 0, 0, 100, 30);
    rect(middle, 0, 0, 100, 10);
    run_layout(middle);
    expect_size(not_last, 100, 30, "a scroll that is not the last child fits its content");
    free_tree(middle);
}

static void test_scroll_in_row(void)
{
    ReactInstance *col = stack(NULL, (StackProps){.x = 10, .y = 10, .gap = 6});
    rect(col, 0, 0, 500, 40);
    ReactInstance *row =
        stack(col, (StackProps){.direction = STACK_ROW, .width = 500, .gap = 20, .align = STACK_ALIGN_CENTER});
    ReactInstance *side = button(row, 0, 0, 100, 40);
    ReactInstance *list = scroll(row, (StackProps){.height = 200, .padding = 4});
    ReactInstance *item = button(list, 0, 0, 80, 300);
    run_layout(col);

    expect_at(side, 0, 80, "a row centers a shorter child against the scroll's height");
    // 100 + 20 along the row; 500 - 120 is left for the scroll's width.
    expect_at(list, 120, 0, "a scroll flows along a row");
    expect_size(list, 380, 200, "the last scroll without a width fills the rest of the row");
    expect(list->layout.content_height == 308, "a scroll in a row still scrolls vertically");
    expect_at(item, 4, 4, "the scroll places its children inside its padding");
    expect_at(row, 0, 46, "the row flows in its column");
    expect_size(col, 500, 246, "the column fits the row holding the scroll");
    free_tree(col);
}

// Expected sizes come from the measurement the renderer uses, at the Runtime Default Font's own metrics.
static int width_of(const char *line, int font_size)
{
    return (int)(text_measure(font_registry_get(NULL), line, font_size) + 0.5f);
}

static int line_height_of(int font_size)
{
    return text_line_height(font_registry_get(NULL), font_size) + 4;
}

static void test_text_in_column(void)
{
    ReactInstance *s = stack(NULL, (StackProps){.gap = 4, .padding = 2});
    ReactInstance *title = text(s, (TextProps){.font_size = 20}, "Hello");
    ReactInstance *r = rect(s, 0, 0, 50, 10);
    ReactInstance *lines = text(s, (TextProps){.font_size = 16, .line_height = 30, .x = 3, .y = 5}, "a\nwider line");
    ReactInstance *defaulted = text(s, (TextProps){0}, "x");
    run_layout(s);

    const int h = line_height_of(20);
    expect_at(title, 2, 2, "text starts inside the padding like any child");
    expect_size(title, width_of("Hello", 20), h, "text is as wide as its line and one line high");
    expect_at(r, 2, 2 + h + 4, "the next child follows the text and the gap");
    expect_at(lines, 5, 2 + h + 4 + 10 + 4 + 5, "y is a leading margin and x an offset, as for any child");
    expect_size(lines, width_of("wider line", 16), 60, "text is as wide as its widest line and lineHeight per line");
    expect_size(defaulted, width_of("x", VD_TEXT_FONT_SIZE_DEFAULT), line_height_of(VD_TEXT_FONT_SIZE_DEFAULT),
                "text without fontSize is measured at the default size");
    expect(s->layout.height == lines->layout.y + 60 + 4 + defaulted->layout.height + 2,
           "the column fits its text children");
    free_tree(s);
}

static void test_text_in_row(void)
{
    ReactInstance *s = stack(NULL, (StackProps){.direction = STACK_ROW, .gap = 10, .align = STACK_ALIGN_CENTER});
    ReactInstance *label = text(s, (TextProps){.font_size = 20}, "Label");
    ReactInstance *b = button(s, 0, 0, 100, 60);
    ReactInstance *after = text(s, (TextProps){.font_size = 20}, "After");
    run_layout(s);

    const int w = width_of("Label", 20);
    const int h = line_height_of(20);
    expect_at(label, 0, (60 - h) / 2, "text is centered on the cross axis by its line height");
    expect_at(b, w + 10, 0, "a button follows the text's width and the gap");
    expect_at(after, w + 10 + 100 + 10, (60 - h) / 2, "text flows after a button");
    expect_size(s, w + 10 + 100 + 10 + width_of("After", 20), 60, "the row fits its text children");
    free_tree(s);
}

static void test_wrapped_text(void)
{
    const char *copy = "Word wrap breaks this sentence into several lines inside its width.";
    ReactInstance *s = stack(NULL, (StackProps){.gap = 6});
    ReactInstance *wrapped =
        text(s, (TextProps){.font_size = 16, .width = 150, .line_height = 20, .wrap = TEXT_WRAP_WORD}, copy);
    ReactInstance *after = rect(s, 0, 0, 10, 10);
    run_layout(s);

    const int count = text_lines(wrapped)->count;
    expect(count > 2, "the sentence wraps into several lines");
    expect_size(wrapped, 150, count * 20, "wrapped text takes its width and one lineHeight per line");
    expect_at(after, 0, count * 20 + 6, "the next child follows every wrapped line");
    free_tree(s);

    ReactInstance *unwrapped = text(NULL, (TextProps){.font_size = 16, .width = 150}, copy);
    run_layout(unwrapped);
    expect_size(unwrapped, width_of(copy, 16), line_height_of(16),
                "without wrap a line wider than width stays one line and sets the width");
    free_tree(unwrapped);
}

static void test_text_align_in_stack(void)
{
    ReactInstance *s = stack(NULL, (StackProps){.width = 400, .padding = 10, .align = STACK_ALIGN_END});
    ReactInstance *fitted = text(s, (TextProps){.font_size = 20, .align = TEXT_ALIGN_CENTER}, "short");
    ReactInstance *boxed = text(s, (TextProps){.font_size = 20, .width = 200, .align = TEXT_ALIGN_RIGHT}, "short");
    run_layout(s);

    expect_at(fitted, 390 - width_of("short", 20), 10, "the stack aligns text by its measured width");
    expect_size(boxed, 200, line_height_of(20), "a text's own align works inside its stated width");
    expect_at(boxed, 190, 10 + line_height_of(20), "the stack aligns text by its stated width");
    free_tree(s);
}

static void test_over_width_text_aligns_in_its_box(void)
{
    const char *copy = "A line wider than its width";
    const TextAlign aligns[] = {TEXT_ALIGN_CENTER, TEXT_ALIGN_RIGHT};
    for (int i = 0; i < 2; i++) {
        ReactInstance *s = stack(NULL, (StackProps){.direction = STACK_ROW, .gap = 10});
        rect(s, 0, 0, 40, 10);
        ReactInstance *t = text(s, (TextProps){.font_size = 16, .width = 100, .align = aligns[i]}, copy);
        run_layout(s);

        const TextLines *lines = text_lines(t);
        const int offset = text_align_offset(lines->width, lines->widths[0], aligns[i]);
        expect_size(t, width_of(copy, 16), line_height_of(16), "an over-width line sets the text's width");
        expect_at(t, 50, 0, "the text's slot follows its sibling");
        expect(offset >= 0, "an over-width aligned line does not start left of its slot");
        expect(offset + lines->widths[0] <= t->layout.width, "an over-width aligned line stays inside its slot");
        free_tree(s);
    }
}

static void test_text_in_rect(void)
{
    ReactInstance *row = rect(NULL, 0, 0, 900, 64);
    ReactInstance *title = text(row, (TextProps){.x = 10, .y = 6, .font_size = 16}, "Title");
    ReactInstance *panel = rect(row, 0, 0, 10, 10);
    ReactInstance *body = text(row, (TextProps){.x = 10, .y = 26, .font_size = 13, .line_height = 15}, "Body");
    ReactInstance *third = text(row, (TextProps){.x = 10, .y = 0, .font_size = 20}, "Third");
    run_layout(row);

    expect_at(title, 10, 6, "the first text in a rect keeps its own x/y");
    expect_at(panel, 0, 0, "other rect children are not moved by text");
    expect_at(body, 10, 26 + 15, "each later text in a rect sits one of its own lines lower per text before it");
    expect_at(third, 10, 2 * line_height_of(20), "two texts before it move it two of its lines");
    free_tree(row);

    ReactInstance *s = stack(NULL, (StackProps){0});
    text(s, (TextProps){.font_size = 20}, "One");
    ReactInstance *two = text(s, (TextProps){.font_size = 20}, "Two");
    run_layout(s);
    expect_at(two, 0, line_height_of(20), "texts in a stack only flow, without the rect's extra line");
    free_tree(s);
}

static bool default_font_present(void)
{
    FILE *file = fopen("assets/fonts/DejaVuSans.ttf", "rb");
    if (!file) return false;
    fclose(file);
    return true;
}

int main(void)
{
    test_column();
    test_row_center();
    test_align_end();
    test_nested();
    test_scroll_matches_previous_flow();
    test_scroll_in_column();
    test_scroll_fills_rest();
    test_scroll_in_row();

    // Without the Runtime Default Font, text would measure with raylib's built-in font, which needs a window.
    if (default_font_present()) {
        font_registry_init();
        test_text_in_column();
        test_text_in_row();
        test_wrapped_text();
        test_text_align_in_stack();
    test_over_width_text_aligns_in_its_box();
        test_text_in_rect();
        font_registry_shutdown();
    } else {
        printf("layout_harness: skipping Text cases, assets/fonts/DejaVuSans.ttf is missing\n");
    }

    if (failures) {
        fprintf(stderr, "layout_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("layout_harness: ok\n");
    return 0;
}
