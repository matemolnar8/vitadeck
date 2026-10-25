#include <math.h>
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

int scroll_max_offset(int viewport_height, int content_height)
{
    int max_offset = content_height - viewport_height;
    return max_offset < 0 ? 0 : max_offset;
}

int scroll_momentum_step(int offset, int max_offset, float *velocity, float dt)
{
    int next = offset + (int)roundf(*velocity * dt);
    if (next <= 0) {
        next = 0;
        *velocity = 0.0f;
    } else if (next >= max_offset) {
        next = max_offset;
        *velocity = 0.0f;
    }
    return next;
}
