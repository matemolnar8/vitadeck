// Regression test for #39: the JS thread mutates the back tree and swaps snapshots while the UI thread
// hit-tests and collects focusables. Run under ASan to catch reads of a freed snapshot.
#define STB_DS_IMPLEMENTATION
#include "instance_fixture.h"
#include "platform/thread.h"
#include "ui/scroll.h"

#include <stdatomic.h>
#include <stdio.h>

#define SWAPS 20000
#define ITEMS 10

static atomic_bool reader_started;
static atomic_bool writer_done;
static atomic_int reader_failures;
static atomic_long reader_passes;
static atomic_long reader_hits;

static bool known_id(const char *id)
{
    return strcmp(id, "root") == 0 || strcmp(id, "list") == 0 || strcmp(id, "toggle") == 0 ||
           strncmp(id, "item-", 5) == 0;
}

static void reader_fail(const char *message)
{
    if (atomic_fetch_add(&reader_failures, 1) == 0) fprintf(stderr, "snapshot_swap_harness: reader: %s\n", message);
}

static void *reader(void *arg)
{
    (void)arg;
    static const int points[][2] = {{20, 20}, {20, 70}, {200, 250}, {600, 40}, {900, 500}, {-5, -5}};
    atomic_store(&reader_started, true);

    while (!atomic_load(&writer_done)) {
        for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
            char id[VD_INSTANCE_ID_MAX];
            if (instance_hit_test(points[i][0], points[i][1], id)) {
                if (!known_id(id)) reader_fail("hit test returned an unknown id");
                atomic_fetch_add(&reader_hits, 1);
            }
        }

        int count = 0;
        FocusableElement *elems = get_focusable_elements(&count);
        if (count > ITEMS + 2) reader_fail("too many focusables");
        for (int i = 0; i < count; i++) {
            if (!known_id(elems[i].id)) reader_fail("focusable has an unknown id");
        }
        free_focusable_elements(elems, count);

        char scroll_id[VD_INSTANCE_ID_MAX];
        if (instance_scroll_at(20, 20, scroll_id) && strcmp(scroll_id, "list") != 0) {
            reader_fail("scroll at (20, 20) is not list");
        }
        if (instance_scroll_for_descendant("item-3", scroll_id) && strcmp(scroll_id, "list") != 0) {
            reader_fail("item-3 is not inside list");
        }
        int viewport = 0, content = 0;
        if (instance_scroll_metrics("list", &viewport, &content) && viewport != 400) {
            reader_fail("list viewport changed");
        }
        (void)instance_exists("toggle");
        (void)instance_tree_count_nodes(NT_BUTTON);

        atomic_fetch_add(&reader_passes, 1);
    }
    return NULL;
}

int main(void)
{
    instance_tree_init();

    ReactInstance *root = fixture_rect(NULL, "root", 0, 0, 960, 544);
    ReactInstance *list = fixture_scroll(root, "list", 0, 0, 400, 400, 8, 8);
    ReactInstance *items[ITEMS];
    for (int i = 0; i < ITEMS; i++) {
        char id[16];
        snprintf(id, sizeof(id), "item-%d", i);
        items[i] = fixture_button(list, id, 0, 0, 300, 40);
    }
    ReactInstance *toggle = NULL;
    fixture_publish();

    vd_thread *thread = vd_thread_create(reader, NULL);
    if (!thread) {
        fprintf(stderr, "snapshot_swap_harness: could not start reader thread\n");
        return 1;
    }
    while (!atomic_load(&reader_started))
        vd_thread_yield();

    for (int i = 0; i < SWAPS; i++) {
        for (int j = 0; j < ITEMS; j++)
            items[j]->props.button.x = (i + j) % 7;

        // Replace one list item, so the registry and child arrays change shape too.
        int k = i % ITEMS;
        char id[16];
        snprintf(id, sizeof(id), "item-%d", k);
        fixture_destroy(items[k]);
        items[k] = fixture_button(list, id, 0, i % 5, 300, 40);

        if (toggle) {
            fixture_destroy(toggle);
            toggle = NULL;
        } else {
            toggle = fixture_button(NULL, "toggle", 560, 0, 400, 544);
        }

        fixture_publish();
    }

    atomic_store(&writer_done, true);
    vd_thread_join(thread);
    vd_thread_destroy(thread);

    instance_tree_clear();
    scroll_reset();

    int failures = atomic_load(&reader_failures);
    long passes = atomic_load(&reader_passes);
    if (passes == 0) {
        fprintf(stderr, "snapshot_swap_harness: reader never completed a pass\n");
        failures++;
    }
    if (atomic_load(&reader_hits) == 0) {
        fprintf(stderr, "snapshot_swap_harness: reader never hit anything\n");
        failures++;
    }
    if (failures) {
        fprintf(stderr, "snapshot_swap_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("snapshot_swap_harness: ok (%d swaps, %ld reader passes)\n", SWAPS, passes);
    return 0;
}
