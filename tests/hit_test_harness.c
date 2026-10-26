#define STB_DS_IMPLEMENTATION
#include "instance_fixture.h"
#include "ui/focus_nav.h"
#include "ui/scroll.h"

#include <stdio.h>

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "hit_test_harness: %s\n", message);
    failures++;
}

// Hit id at (x, y), or "" for a miss.
static const char *hit(int x, int y)
{
    static char id[VD_INSTANCE_ID_MAX];
    if (!instance_hit_test(x, y, id)) id[0] = '\0';
    return id;
}

static const char *scroll_at(int x, int y)
{
    static char id[VD_INSTANCE_ID_MAX];
    if (!instance_scroll_at(x, y, id)) id[0] = '\0';
    return id;
}

static void expect_hit(int x, int y, const char *expected, const char *message)
{
    const char *got = hit(x, y);
    if (strcmp(got, expected) != 0) {
        fprintf(stderr, "hit_test_harness: hit (%d, %d) = \"%s\", want \"%s\"\n", x, y, got, expected);
        expect(false, message);
    }
}

static void test_overlap_order(void)
{
    ReactInstance *a = fixture_rect(NULL, "a", 0, 0, 100, 100);
    fixture_rect(NULL, "b", 50, 50, 100, 100);
    ReactInstance *panel = fixture_rect(a, "panel", 10, 10, 60, 60);
    fixture_button(panel, "under", 0, 0, 30, 30);
    fixture_button(panel, "over", 20, 20, 30, 30);
    fixture_publish();

    expect_hit(75, 75, "b", "a later root sibling is on top");
    expect_hit(5, 5, "a", "a rect hits outside its children");
    expect_hit(15, 15, "under", "child offsets add up through rects");
    expect_hit(35, 35, "over", "a later child is on top of an earlier one");
    expect_hit(65, 65, "b", "a later root sibling covers the earlier root's children");
    expect_hit(65, 15, "panel", "the parent rect hits where no child is");
    expect_hit(149, 149, "b", "the last pixel is inside");
    expect_hit(150, 150, "", "right and bottom edges are exclusive");
    expect_hit(-1, 0, "", "points left of everything miss");

    instance_tree_clear();
}

// Scroll S at (200, 0) 200x200, padding 5, gap 10. Flow: the nested scroll at y 5, offset 110 to the right,
// b0 at 5 + 50, b1 at 55 + 60 + 4 (its margin), b2 at 55 + 124.
static void build_scroll_tree(void)
{
    ReactInstance *s = fixture_scroll(NULL, "S", 200, 0, 200, 200, 10, 5);
    ReactInstance *inner = fixture_scroll(s, "inner", 110, 0, 80, 40, 0, 0);
    fixture_button(inner, "inner-b", 0, 0, 80, 30);
    fixture_button(s, "b0", 0, 0, 100, 50);
    fixture_button(s, "b1", 0, 4, 100, 50);
    fixture_button(s, "b2", 0, 0, 100, 50);
    fixture_rect(NULL, "outside", 0, 0, 100, 100);
    fixture_publish();
}

static void test_scroll_offset(void)
{
    build_scroll_tree();

    expect_hit(350, 20, "inner-b", "a nested scroll flows from the padding with its cross offset");
    expect_hit(250, 80, "b0", "the next child follows the nested scroll and the gap");
    expect_hit(250, 110, "S", "the gap between scroll children hits the scroll container");
    expect_hit(250, 120, "b1", "a child's own y is a top margin in the flow");
    expect_hit(210, 3, "S", "padding hits the scroll container");
    expect(strcmp(scroll_at(250, 80), "S") == 0, "scroll at a child is its scroll container");
    expect(strcmp(scroll_at(350, 20), "inner") == 0, "scroll at prefers the nested scroll");
    expect(strcmp(scroll_at(50, 50), "") == 0, "scroll at outside any scroll is empty");

    char id[VD_INSTANCE_ID_MAX];
    expect(instance_scroll_for_descendant("inner-b", id) && strcmp(id, "inner") == 0,
           "the nearest scroll ancestor wins");
    expect(instance_scroll_for_descendant("S", id) && strcmp(id, "S") == 0, "a scroll is its own scroll container");
    expect(!instance_scroll_for_descendant("outside", id), "an instance outside scrolls has no scroll container");

    scroll_set_offset("S", 114);
    expect_hit(250, 30, "b1", "the scroll offset moves children up");
    expect_hit(250, 70, "b2", "later children move up by the same offset");
    expect_hit(250, -20, "", "a child scrolled out above the viewport does not hit");
    expect_hit(350, 20, "S", "nested content moves with the offset");
    scroll_set_offset("S", -20);
    expect(scroll_get_offset("S") == 0, "negative offsets are stored as zero");

    instance_tree_clear();
    scroll_reset();
}

static bool has_focusable(const FocusableElement *elems, int count, const char *id, int x, int y)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(elems[i].id, id) == 0) return elems[i].x == x && elems[i].y == y;
    }
    return false;
}

static void test_focusables_clip(void)
{
    build_scroll_tree();

    int count = 0;
    FocusableElement *elems = get_focusable_elements(&count);
    expect(count == 6, "every button and scroll is focusable");
    expect(count > 0 && strcmp(elems[0].id, "S") == 0, "a scroll comes before its children");
    expect(has_focusable(elems, count, "inner-b", 315, 5), "nested scroll children are focusable");
    free_focusable_elements(elems, count);

    scroll_set_offset("S", 114);
    elems = get_focusable_elements(&count);
    expect(count == 3, "only S, b1 and b2 intersect the viewport after scrolling");
    expect(!has_focusable(elems, count, "b0", 205, -59), "a child scrolled fully out of view is not focusable");
    expect(!has_focusable(elems, count, "inner-b", 315, -109), "a nested scroll clipped away hides its children");
    expect(has_focusable(elems, count, "b1", 205, 5), "focusables use scrolled absolute coordinates");
    free_focusable_elements(elems, count);

    instance_tree_clear();
    scroll_reset();
}

// Focus target from `from` in dir, or "" for none; NULL `from` means no current focus.
static const char *nav(const char *from, NavDirection dir)
{
    static char id[VD_INSTANCE_ID_MAX];
    int count = 0;
    FocusableElement *elems = get_focusable_elements(&count);
    int pick = focus_nav_pick(elems, count, from, dir);
    snprintf(id, sizeof(id), "%s", pick >= 0 ? elems[pick].id : "");
    free_focusable_elements(elems, count);
    return id;
}

static void expect_nav(const char *from, NavDirection dir, const char *expected, const char *message)
{
    const char *got = nav(from, dir);
    if (strcmp(got, expected) != 0) {
        fprintf(stderr, "hit_test_harness: nav from \"%s\" = \"%s\", want \"%s\"\n", from ? from : "(none)", got,
                expected);
        expect(false, message);
    }
}

static void test_focus_navigation(void)
{
    expect(focus_nav_pick(NULL, 0, "x", NAV_DOWN) == -1, "nothing to focus picks nothing");

    // A 2x2 grid, and "o" with a nearer off-axis target and a farther on-axis one to its right.
    fixture_button(NULL, "tl", 0, 0, 40, 40);
    fixture_button(NULL, "tr", 100, 0, 40, 40);
    fixture_button(NULL, "bl", 0, 100, 40, 40);
    fixture_button(NULL, "br", 100, 100, 40, 40);
    fixture_button(NULL, "o", 300, 400, 40, 40);
    fixture_button(NULL, "near-off", 350, 500, 40, 40);
    fixture_button(NULL, "far-on", 500, 400, 40, 40);
    fixture_publish();

    expect_nav(NULL, NAV_DOWN, "tl", "no focus picks the first focusable");
    expect_nav("gone", NAV_UP, "tl", "a stale focus id picks the first focusable");
    expect_nav("tl", NAV_RIGHT, "tr", "right moves along the row");
    expect_nav("tl", NAV_DOWN, "bl", "down moves along the column");
    expect_nav("tl", NAV_UP, "", "nothing above the top row");
    expect_nav("tl", NAV_LEFT, "", "nothing left of the first column");
    expect_nav("br", NAV_UP, "tr", "up from the bottom row");
    expect_nav("br", NAV_LEFT, "bl", "left from the right column");
    // Score is on-axis + 2 * off-axis distance: near-off 50 + 2 * 100 = 250, far-on 200 + 0 = 200.
    expect_nav("o", NAV_RIGHT, "far-on", "off-axis distance counts double");
    expect_nav("near-off", NAV_UP, "o", "up only considers centers above");

    instance_tree_clear();
}

// Scroll S2 at (0, 300) 400x100, padding 10, holding a row Stack of two buttons 20 apart: left at (10, 310),
// right at (130, 310). A column Stack at (500, 0) holds a 30px header and a Scroll that flows below it.
static void test_stack(void)
{
    ReactInstance *s = fixture_scroll(NULL, "S2", 0, 300, 400, 100, 0, 10);
    ReactInstance *row = fixture_stack(s, "row", 0, 0, STACK_ROW, 20);
    fixture_button(row, "left", 0, 0, 100, 40);
    fixture_button(row, "right", 0, 0, 100, 40);
    ReactInstance *col = fixture_stack(NULL, "col", 500, 0, STACK_COLUMN, 0);
    fixture_rect(col, "header", 0, 0, 100, 30);
    fixture_scroll(col, "list", 0, 0, 100, 50, 0, 0);
    fixture_publish();

    expect_hit(150, 330, "right", "a row stack places its second child after the first and the gap");
    expect_hit(120, 330, "S2", "the gap inside a stack without a fill hits what is behind it");
    expect_hit(50, 380, "S2", "below a fitted stack is its scroll container");
    expect_nav("left", NAV_RIGHT, "right", "right moves along a row stack");
    expect_nav("right", NAV_LEFT, "left", "left moves back along a row stack");
    expect_hit(550, 40, "list", "a scroll inside a stack flows below the header");
    expect(strcmp(scroll_at(550, 40), "list") == 0, "scroll at finds a scroll inside a stack");
    expect(strcmp(scroll_at(550, 10), "") == 0, "a stack is not a scroll container");

    scroll_set_offset("S2", 5);
    int count = 0;
    FocusableElement *elems = get_focusable_elements(&count);
    expect(has_focusable(elems, count, "right", 130, 305), "stack children move with the scroll offset");
    free_focusable_elements(elems, count);

    instance_tree_clear();
    scroll_reset();
}

// Two root Stacks over a Rect "back" at (0, 600) 300x100, each with two 100x40 buttons 50 apart in a row.
static void test_stack_transparency(void)
{
    fixture_rect(NULL, "back", 0, 600, 300, 100);
    ReactInstance *clear = fixture_stack(NULL, "clear", 0, 600, STACK_ROW, 50);
    fixture_button(clear, "c1", 0, 0, 100, 40);
    fixture_button(clear, "c2", 0, 0, 100, 40);
    ReactInstance *filled = fixture_stack(NULL, "filled", 0, 650, STACK_ROW, 50);
    filled->props.stack.has_fill = true;
    fixture_button(filled, "f1", 0, 0, 100, 40);
    fixture_button(filled, "f2", 0, 0, 100, 40);
    fixture_publish();

    expect_hit(50, 620, "c1", "a transparent stack still passes hits to its children");
    expect_hit(125, 620, "back", "the gap in a stack without a fill hits what is behind it");
    expect_hit(175, 620, "c2", "the second child of a transparent stack hits");
    expect_hit(125, 670, "filled", "the gap in a filled stack hits the stack, like a rect");
    expect_hit(175, 670, "f2", "children of a filled stack hit");

    instance_tree_clear();
}

int main(void)
{
    instance_tree_init();

    expect_hit(0, 0, "", "an empty tree hits nothing");
    test_overlap_order();
    test_scroll_offset();
    test_focusables_clip();
    test_focus_navigation();
    test_stack();
    test_stack_transparency();

    if (failures) {
        fprintf(stderr, "hit_test_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("hit_test_harness: ok\n");
    return 0;
}
