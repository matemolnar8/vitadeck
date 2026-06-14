#include <stdbool.h>
#include <stdlib.h>
#include "stb_ds.h"
#include "scroll.h"

typedef struct {
    char *key;
    int value;
} ScrollEntry;

static ScrollEntry *offsets = NULL;
static bool map_initialized = false;

static void ensure_map(void)
{
    if (!map_initialized) {
        sh_new_strdup(offsets);
        map_initialized = true;
    }
}

int scroll_get_offset(const char *id)
{
    if (!id) return 0;
    ensure_map();
    int idx = shgeti(offsets, id);
    return idx >= 0 ? offsets[idx].value : 0;
}

void scroll_set_offset(const char *id, int offset)
{
    if (!id) return;
    ensure_map();
    if (offset < 0) offset = 0;
    shput(offsets, id, offset);
}

void scroll_reset(void)
{
    if (!map_initialized) return;
    shfree(offsets);
    offsets = NULL;
    map_initialized = false;
}
