#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "arena.h"
#include "stb_ds.h"
#include "instance_tree.h"
#include "scroll.h"
#include "platform/thread.h"

typedef struct {
    char *key;
    ReactInstance *value;
} InstanceEntry;

// Snapshot structure for thread-safe access
typedef struct {
    Arena arena;
    InstanceEntry *registry;
    ReactInstance **root_children;
} InstanceSnapshot;

// Back buffer: JS thread writes here (single-threaded access)
static InstanceEntry *back_registry = NULL;
static ReactInstance **back_root_children = NULL;

// Front snapshot: UI thread reads from here
static InstanceSnapshot *front_snapshot = NULL;

// Mutex to protect front_snapshot access during swap
static vd_mutex *snapshot_mutex = NULL;

// Free an instance (shallow, does not free children)
void instance_back_free(ReactInstance *inst)
{
    if (!inst) return;
    if (inst->id) free(inst->id);
    if (inst->type == NT_TEXT && inst->props.text.font_name) {
        free(inst->props.text.font_name);
    }
    if (inst->type == NT_BUTTON && inst->props.button.label) {
        free(inst->props.button.label);
    }
    if (inst->type == NT_RAW_TEXT && inst->props.raw_text) {
        free(inst->props.raw_text);
    }
    arrfree(inst->children);
    free(inst);
}

// Free an instance tree recursively
static void free_instance_tree(ReactInstance *inst)
{
    if (!inst) return;

    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        free_instance_tree(inst->children[i]);
    }

    if (inst->id) free(inst->id);
    if (inst->type == NT_TEXT && inst->props.text.font_name) {
        free(inst->props.text.font_name);
    }
    if (inst->type == NT_BUTTON && inst->props.button.label) {
        free(inst->props.button.label);
    }
    if (inst->type == NT_RAW_TEXT && inst->props.raw_text) {
        free(inst->props.raw_text);
    }
    arrfree(inst->children);
    free(inst);
}

// Free stb_ds child arrays in a front-snapshot tree (instance data lives in the arena).
static void free_snapshot_children_arrays(ReactInstance *inst)
{
    if (!inst) return;

    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        free_snapshot_children_arrays(inst->children[i]);
    }
    arrfree(inst->children);
}

// Free a snapshot
static void free_snapshot(InstanceSnapshot *snap)
{
    if (!snap) return;

    int count = arrlen(snap->root_children);
    for (int i = 0; i < count; i++) {
        free_snapshot_children_arrays(snap->root_children[i]);
    }
    arrfree(snap->root_children);
    shfree(snap->registry);
    arena_free(&snap->arena);
    free(snap);
}

// Deep copy a single instance (without children links)
static ReactInstance *copy_instance(Arena *arena, ReactInstance *src)
{
    if (!src) return NULL;

    ReactInstance *dst = arena_alloc(arena, sizeof(ReactInstance));
    if (!dst) return NULL;
    memset(dst, 0, sizeof(ReactInstance));
    dst->id = src->id ? arena_strdup(arena, src->id) : NULL;
    dst->type = src->type;
    dst->children = NULL;
    dst->parent = NULL;

    switch (src->type) {
    case NT_RECT:
        dst->props.rect = src->props.rect;
        break;
    case NT_TEXT:
        dst->props.text = src->props.text;
        dst->props.text.font_name = src->props.text.font_name ? arena_strdup(arena, src->props.text.font_name) : NULL;
        break;
    case NT_BUTTON:
        dst->props.button = src->props.button;
        dst->props.button.label = src->props.button.label ? arena_strdup(arena, src->props.button.label) : NULL;
        break;
    case NT_RAW_TEXT:
        dst->props.raw_text = src->props.raw_text ? arena_strdup(arena, src->props.raw_text) : NULL;
        break;
    case NT_SCROLL:
        dst->props.scroll = src->props.scroll;
        break;
    }

    return dst;
}

bool scroll_flow_step(const ReactInstance *child, int gap, int *flow_y, int *base_y_out)
{
    if (!child) return false;

    int margin_top, height;
    switch (child->type) {
    case NT_RECT:
        margin_top = child->props.rect.y;
        height = child->props.rect.height;
        break;
    case NT_BUTTON:
        margin_top = child->props.button.y;
        height = child->props.button.height;
        break;
    default:
        return false;
    }

    *base_y_out = *flow_y;
    *flow_y += margin_top + height + gap;
    return true;
}

int scroll_content_height(const ReactInstance *scroll)
{
    if (!scroll || scroll->type != NT_SCROLL) return 0;

    const ScrollProps *s = &scroll->props.scroll;
    int flow_y = 0;
    int base_y = 0;
    bool any = false;

    int count = arrlen(scroll->children);
    for (int i = 0; i < count; i++) {
        if (scroll_flow_step(scroll->children[i], s->gap, &flow_y, &base_y)) any = true;
    }

    int inner = any ? flow_y - s->gap : 0;
    return inner + s->padding * 2;
}

// Recursively copy instance and all children
static ReactInstance *deep_copy_instance(Arena *arena, ReactInstance *src, InstanceEntry **new_registry)
{
    if (!src) return NULL;

    ReactInstance *dst = copy_instance(arena, src);
    if (!dst) return NULL;
    if (dst->id) {
        shput(*new_registry, dst->id, dst);
    }

    int child_count = arrlen(src->children);
    for (int i = 0; i < child_count; i++) {
        if (src->children[i]) {
            ReactInstance *child_copy = deep_copy_instance(arena, src->children[i], new_registry);
            if (child_copy) {
                child_copy->parent = dst;
                arrput(dst->children, child_copy);
            }
        }
    }

    return dst;
}

void instance_tree_init(void)
{
    if (!snapshot_mutex) {
        snapshot_mutex = vd_mutex_create();
    }
}

void instance_tree_swap(void)
{
    // Create new snapshot from back buffer (outside lock)
    InstanceSnapshot *new_snap = calloc(1, sizeof(InstanceSnapshot));
    new_snap->registry = NULL;
    new_snap->root_children = NULL;

    int count = arrlen(back_root_children);
    for (int i = 0; i < count; i++) {
        if (back_root_children[i]) {
            ReactInstance *copy = deep_copy_instance(&new_snap->arena, back_root_children[i], &new_snap->registry);
            arrput(new_snap->root_children, copy);
        }
    }

    // Swap under lock
    InstanceSnapshot *old_snap = NULL;
    vd_mutex_lock(snapshot_mutex);
    old_snap = front_snapshot;
    front_snapshot = new_snap;
    vd_mutex_unlock(snapshot_mutex);

    // Free old snapshot (outside lock, safe because UI no longer references it)
    free_snapshot(old_snap);
}

void instance_tree_clear(void)
{
    int back_count = arrlen(back_root_children);
    for (int i = 0; i < back_count; i++) {
        free_instance_tree(back_root_children[i]);
    }
    arrfree(back_root_children);
    back_root_children = NULL;
    shfree(back_registry);
    back_registry = NULL;

    InstanceSnapshot *old_snap = NULL;
    vd_mutex_lock(snapshot_mutex);
    old_snap = front_snapshot;
    front_snapshot = NULL;
    vd_mutex_unlock(snapshot_mutex);
    free_snapshot(old_snap);
}

void instance_tree_render_lock(void)
{
    vd_mutex_lock(snapshot_mutex);
}

void instance_tree_render_unlock(void)
{
    vd_mutex_unlock(snapshot_mutex);
}

ReactInstance **instance_get_root_children(void)
{
    return front_snapshot ? front_snapshot->root_children : NULL;
}

// For hit testing, find in front buffer (UI thread) - must be called under lock
static ReactInstance *find_front_instance_unlocked(InstanceSnapshot *snap, const char *id)
{
    if (!id || !snap) return NULL;
    int idx = shgeti(snap->registry, id);
    if (idx < 0) return NULL;
    return snap->registry[idx].value;
}

bool instance_exists(const char *id)
{
    if (!id) return false;
    vd_mutex_lock(snapshot_mutex);
    bool exists = front_snapshot && find_front_instance_unlocked(front_snapshot, id) != NULL;
    vd_mutex_unlock(snapshot_mutex);
    return exists;
}

// Hit testing
static const char *hit_test_recursive(ReactInstance **children, int x, int y, int offset_x, int offset_y);

static const char *hit_test_instance(ReactInstance *inst, int x, int y, int offset_x, int offset_y)
{
    if (!inst) return NULL;

    if (inst->type == NT_RECT) {
        RectProps *r = &inst->props.rect;
        int abs_x = offset_x + r->x;
        int abs_y = offset_y + r->y;

        const char *child_hit = hit_test_recursive(inst->children, x, y, abs_x, abs_y);
        if (child_hit) return child_hit;

        if (x >= abs_x && x < abs_x + r->width && y >= abs_y && y < abs_y + r->height) {
            return inst->id;
        }
    } else if (inst->type == NT_BUTTON) {
        ButtonProps *b = &inst->props.button;
        int abs_x = offset_x + b->x;
        int abs_y = offset_y + b->y;

        if (x >= abs_x && x < abs_x + b->width && y >= abs_y && y < abs_y + b->height) {
            return inst->id;
        }
    } else if (inst->type == NT_SCROLL) {
        ScrollProps *s = &inst->props.scroll;
        int abs_x = offset_x + s->x;
        int abs_y = offset_y + s->y;

        bool inside = x >= abs_x && x < abs_x + s->width && y >= abs_y && y < abs_y + s->height;
        if (!inside) return NULL;

        int content_x = abs_x + s->padding;
        int content_y = abs_y + s->padding - scroll_get_offset(inst->id);

        const char *hit = NULL;
        int flow_y = 0;
        int count = arrlen(inst->children);
        for (int i = 0; i < count; i++) {
            ReactInstance *child = inst->children[i];
            if (!child) continue;
            int base_y = 0;
            int child_offset_y = content_y;
            if (scroll_flow_step(child, s->gap, &flow_y, &base_y)) {
                child_offset_y = content_y + base_y;
            }
            const char *child_hit = hit_test_instance(child, x, y, content_x, child_offset_y);
            if (child_hit) hit = child_hit; /* later children render on top */
        }
        if (hit) return hit;

        return inst->id;
    }

    return NULL;
}

static const char *hit_test_recursive(ReactInstance **children, int x, int y, int offset_x, int offset_y)
{
    if (!children) return NULL;
    int count = arrlen(children);

    for (int i = count - 1; i >= 0; i--) {
        const char *hit = hit_test_instance(children[i], x, y, offset_x, offset_y);
        if (hit) return hit;
    }

    return NULL;
}

const char *instance_hit_test(int x, int y)
{
    vd_mutex_lock(snapshot_mutex);
    const char *result = NULL;
    if (front_snapshot) {
        result = hit_test_recursive(front_snapshot->root_children, x, y, 0, 0);
    }
    vd_mutex_unlock(snapshot_mutex);
    return result;
}

static bool rect_intersects_clip(int x, int y, int width, int height, bool has_clip, int clip_x, int clip_y,
                                 int clip_width, int clip_height)
{
    if (!has_clip) return true;
    return x < clip_x + clip_width && x + width > clip_x && y < clip_y + clip_height && y + height > clip_y;
}

static void add_focusable_if_visible(const char *id, int x, int y, int width, int height, bool has_clip, int clip_x,
                                     int clip_y, int clip_width, int clip_height, FocusableElement **out_elems)
{
    if (!id || !rect_intersects_clip(x, y, width, height, has_clip, clip_x, clip_y, clip_width, clip_height)) return;

    FocusableElement elem = {.id = strdup(id), .x = x, .y = y, .width = width, .height = height};
    arrput(*out_elems, elem);
}

static bool intersect_clip(bool has_a, int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh, bool *has_out,
                           int *out_x, int *out_y, int *out_w, int *out_h)
{
    if (!has_a) {
        *has_out = true;
        *out_x = bx;
        *out_y = by;
        *out_w = bw;
        *out_h = bh;
        return bw > 0 && bh > 0;
    }

    int x1 = ax > bx ? ax : bx;
    int y1 = ay > by ? ay : by;
    int x2 = ax + aw < bx + bw ? ax + aw : bx + bw;
    int y2 = ay + ah < by + bh ? ay + ah : by + bh;

    *has_out = true;
    *out_x = x1;
    *out_y = y1;
    *out_w = x2 - x1;
    *out_h = y2 - y1;
    return *out_w > 0 && *out_h > 0;
}

static void collect_focusable_instance(ReactInstance *inst, int offset_x, int offset_y, bool has_clip, int clip_x,
                                       int clip_y, int clip_width, int clip_height, FocusableElement **out_elems);

// Focusable element collection for gamepad navigation
static void collect_focusable_recursive(ReactInstance **children, int offset_x, int offset_y, bool has_clip, int clip_x,
                                        int clip_y, int clip_width, int clip_height, FocusableElement **out_elems)
{
    if (!children) return;
    int count = arrlen(children);

    for (int i = 0; i < count; i++) {
        collect_focusable_instance(children[i], offset_x, offset_y, has_clip, clip_x, clip_y, clip_width, clip_height,
                                   out_elems);
    }
}

static void collect_focusable_instance(ReactInstance *inst, int offset_x, int offset_y, bool has_clip, int clip_x,
                                       int clip_y, int clip_width, int clip_height, FocusableElement **out_elems)
{
    if (!inst) return;

    if (inst->type == NT_BUTTON) {
        ButtonProps *b = &inst->props.button;
        add_focusable_if_visible(inst->id, offset_x + b->x, offset_y + b->y, b->width, b->height, has_clip, clip_x,
                                 clip_y, clip_width, clip_height, out_elems);
    } else if (inst->type == NT_RECT) {
        RectProps *r = &inst->props.rect;
        collect_focusable_recursive(inst->children, offset_x + r->x, offset_y + r->y, has_clip, clip_x, clip_y,
                                    clip_width, clip_height, out_elems);
    } else if (inst->type == NT_SCROLL) {
        ScrollProps *s = &inst->props.scroll;
        int abs_x = offset_x + s->x;
        int abs_y = offset_y + s->y;
        add_focusable_if_visible(inst->id, abs_x, abs_y, s->width, s->height, has_clip, clip_x, clip_y, clip_width,
                                 clip_height, out_elems);

        bool child_has_clip = false;
        int child_clip_x = 0, child_clip_y = 0, child_clip_width = 0, child_clip_height = 0;
        if (!intersect_clip(has_clip, clip_x, clip_y, clip_width, clip_height, abs_x, abs_y, s->width, s->height,
                            &child_has_clip, &child_clip_x, &child_clip_y, &child_clip_width, &child_clip_height)) {
            return;
        }

        int content_x = abs_x + s->padding;
        int content_y = abs_y + s->padding - scroll_get_offset(inst->id);
        int flow_y = 0;
        int child_count = arrlen(inst->children);
        for (int child_i = 0; child_i < child_count; child_i++) {
            ReactInstance *child = inst->children[child_i];
            if (!child) continue;

            int base_y = 0;
            int child_offset_y = content_y;
            if (scroll_flow_step(child, s->gap, &flow_y, &base_y)) {
                child_offset_y = content_y + base_y;
            }
            collect_focusable_instance(child, content_x, child_offset_y, child_has_clip, child_clip_x, child_clip_y,
                                       child_clip_width, child_clip_height, out_elems);
        }
    }
}

FocusableElement *get_focusable_elements(int *count)
{
    FocusableElement *elems = NULL;

    vd_mutex_lock(snapshot_mutex);
    if (front_snapshot) {
        collect_focusable_recursive(front_snapshot->root_children, 0, 0, false, 0, 0, 0, 0, &elems);
    }
    vd_mutex_unlock(snapshot_mutex);

    *count = arrlen(elems);
    return elems;
}

void free_focusable_elements(FocusableElement *elems, int count)
{
    for (int i = 0; i < count; i++) {
        free(elems[i].id);
    }
    arrfree(elems);
}

bool instance_scroll_metrics(const char *id, int *viewport_height, int *content_height)
{
    if (!id) return false;

    bool found = false;
    vd_mutex_lock(snapshot_mutex);
    if (front_snapshot) {
        ReactInstance *inst = find_front_instance_unlocked(front_snapshot, id);
        if (inst && inst->type == NT_SCROLL) {
            *viewport_height = inst->props.scroll.height;
            *content_height = scroll_content_height(inst);
            found = true;
        }
    }
    vd_mutex_unlock(snapshot_mutex);
    return found;
}

static ReactInstance *scroll_at_instance(ReactInstance *inst, int x, int y, int offset_x, int offset_y)
{
    if (!inst) return NULL;

    if (inst->type == NT_RECT) {
        RectProps *r = &inst->props.rect;
        ReactInstance *found = NULL;
        int count = arrlen(inst->children);
        for (int i = 0; i < count; i++) {
            ReactInstance *hit = scroll_at_instance(inst->children[i], x, y, offset_x + r->x, offset_y + r->y);
            if (hit) found = hit;
        }
        return found;
    }

    if (inst->type != NT_SCROLL) return NULL;

    ScrollProps *s = &inst->props.scroll;
    int abs_x = offset_x + s->x;
    int abs_y = offset_y + s->y;
    if (x < abs_x || x >= abs_x + s->width || y < abs_y || y >= abs_y + s->height) return NULL;

    /* Prefer a nested scroll container under the point. */
    ReactInstance *found = inst;
    int content_x = abs_x + s->padding;
    int content_y = abs_y + s->padding - scroll_get_offset(inst->id);
    int flow_y = 0;
    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        ReactInstance *child = inst->children[i];
        if (!child) continue;
        int base_y = 0;
        int child_offset_y = content_y;
        if (scroll_flow_step(child, s->gap, &flow_y, &base_y)) {
            child_offset_y = content_y + base_y;
        }
        ReactInstance *hit = scroll_at_instance(child, x, y, content_x, child_offset_y);
        if (hit) found = hit;
    }
    return found;
}

char *instance_scroll_for_descendant(const char *id)
{
    if (!id) return NULL;

    char *result = NULL;
    vd_mutex_lock(snapshot_mutex);
    if (front_snapshot) {
        ReactInstance *inst = find_front_instance_unlocked(front_snapshot, id);
        while (inst) {
            if (inst->type == NT_SCROLL && inst->id) {
                result = strdup(inst->id);
                break;
            }
            inst = inst->parent;
        }
    }
    vd_mutex_unlock(snapshot_mutex);
    return result;
}

char *instance_scroll_at(int x, int y)
{
    char *result = NULL;
    vd_mutex_lock(snapshot_mutex);
    if (front_snapshot) {
        ReactInstance *found = NULL;
        int count = arrlen(front_snapshot->root_children);
        for (int i = 0; i < count; i++) {
            ReactInstance *hit = scroll_at_instance(front_snapshot->root_children[i], x, y, 0, 0);
            if (hit) found = hit;
        }
        if (found && found->id) result = strdup(found->id);
    }
    vd_mutex_unlock(snapshot_mutex);
    return result;
}

// Back-buffer operations (JS thread only)
ReactInstance *instance_back_find(const char *id)
{
    if (!id) return NULL;
    int idx = shgeti(back_registry, id);
    if (idx < 0) return NULL;
    return back_registry[idx].value;
}

void instance_back_put(ReactInstance *inst)
{
    if (!inst || !inst->id) return;
    shput(back_registry, inst->id, inst);
}

void instance_back_del(const char *id)
{
    if (!id) return;
    shdel(back_registry, id);
}

void instance_back_root_append(ReactInstance *child)
{
    if (!child) return;
    child->parent = NULL;
    arrput(back_root_children, child);
}

void instance_back_root_insert(ReactInstance *child, ReactInstance *before)
{
    if (!child) return;
    child->parent = NULL;

    int count = arrlen(back_root_children);
    int insert_idx = count;
    for (int i = 0; i < count; i++) {
        if (back_root_children[i] == before) {
            insert_idx = i;
            break;
        }
    }

    arrins(back_root_children, insert_idx, child);
}

void instance_back_root_remove(ReactInstance *child)
{
    if (!child) return;

    int count = arrlen(back_root_children);
    for (int i = 0; i < count; i++) {
        if (back_root_children[i] == child) {
            arrdel(back_root_children, i);
            break;
        }
    }
    child->parent = NULL;
}

void instance_back_root_clear(void)
{
    arrfree(back_root_children);
    back_root_children = NULL;
}

static void collect_text_recursive(const ReactInstance *inst, char *buffer, size_t buffer_size)
{
    if (!inst || buffer_size == 0) return;

    if (inst->type == NT_RAW_TEXT && inst->props.raw_text) {
        size_t used = strlen(buffer);
        if (used < buffer_size - 1) {
            strncat(buffer, inst->props.raw_text, buffer_size - used - 1);
        }
    }

    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        collect_text_recursive(inst->children[i], buffer, buffer_size);
    }
}

static int count_node_type_recursive(const ReactInstance *inst, NodeType type)
{
    if (!inst) return 0;

    int count = inst->type == type ? 1 : 0;
    int child_count = arrlen(inst->children);
    for (int i = 0; i < child_count; i++) {
        count += count_node_type_recursive(inst->children[i], type);
    }
    return count;
}

void instance_tree_collect_text(char *buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0) return;
    buffer[0] = '\0';

    instance_tree_render_lock();
    ReactInstance **roots = instance_get_root_children();
    int count = arrlen(roots);
    for (int i = 0; i < count; i++) {
        collect_text_recursive(roots[i], buffer, buffer_size);
    }
    instance_tree_render_unlock();
}

bool instance_tree_contains_text(const char *needle)
{
    if (!needle) return false;

    char text[4096];
    instance_tree_collect_text(text, sizeof(text));
    return strstr(text, needle) != NULL;
}

int instance_tree_count_nodes(NodeType type)
{
    int count = 0;
    instance_tree_render_lock();
    ReactInstance **roots = instance_get_root_children();
    int root_count = arrlen(roots);
    for (int i = 0; i < root_count; i++) {
        count += count_node_type_recursive(roots[i], type);
    }
    instance_tree_render_unlock();
    return count;
}
