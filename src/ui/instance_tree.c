#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "arena.h"
#include "stb_ds.h"
#include "instance_tree.h"
#include "layout.h"
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
static bool back_dirty = false;

// Front snapshot: UI thread reads from here
static InstanceSnapshot *front_snapshot = NULL;

// Mutex to protect front_snapshot access during swap
static vd_mutex *snapshot_mutex = NULL;

// autoFocus requests travel with the snapshot so the UI thread sees them once the instance is in the front tree.
// The back request is JS-thread only; the front request is guarded by snapshot_mutex. "" means none.
static char back_focus_request[VD_INSTANCE_ID_MAX] = "";
static char front_focus_request[VD_INSTANCE_ID_MAX] = "";

// Free an instance (shallow, does not free children)
void instance_back_free(ReactInstance *inst)
{
    if (!inst) return;
    if (inst->id) free(inst->id);
    if (inst->type == NT_TEXT && inst->props.text.font_name) {
        free(inst->props.text.font_name);
    }
    if (inst->type == NT_BUTTON) {
        free(inst->props.button.label);
        free(inst->props.button.font_name);
    }
    if (inst->type == NT_TEXT_INPUT) {
        free(inst->props.text_input.font_name);
        free(inst->props.text_input.value);
        free(inst->props.text_input.placeholder);
    }
    if (inst->type == NT_RAW_TEXT && inst->props.raw_text) {
        free(inst->props.raw_text);
    }
    if (inst->type == NT_IMAGE && inst->props.image.image_name) {
        free(inst->props.image.image_name);
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
    instance_back_free(inst);
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
        dst->props.button.font_name =
            src->props.button.font_name ? arena_strdup(arena, src->props.button.font_name) : NULL;
        break;
    case NT_TEXT_INPUT: {
        const TextInputProps *t = &src->props.text_input;
        dst->props.text_input = *t;
        dst->props.text_input.font_name = t->font_name ? arena_strdup(arena, t->font_name) : NULL;
        dst->props.text_input.value = t->value ? arena_strdup(arena, t->value) : NULL;
        dst->props.text_input.placeholder = t->placeholder ? arena_strdup(arena, t->placeholder) : NULL;
        break;
    }
    case NT_RAW_TEXT:
        dst->props.raw_text = src->props.raw_text ? arena_strdup(arena, src->props.raw_text) : NULL;
        break;
    case NT_STACK:
    case NT_SCROLL:
        dst->props.stack = src->props.stack;
        break;
    case NT_IMAGE:
        dst->props.image = src->props.image;
        dst->props.image.image_name =
            src->props.image.image_name ? arena_strdup(arena, src->props.image.image_name) : NULL;
        break;
    }

    return dst;
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

void instance_back_mark_dirty(void)
{
    back_dirty = true;
}

bool instance_tree_swap(void)
{
    if (!back_dirty) return false;
    back_dirty = false;

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
    layout_tree(new_snap->root_children);

    // Swap under lock
    InstanceSnapshot *old_snap = NULL;
    vd_mutex_lock(snapshot_mutex);
    old_snap = front_snapshot;
    front_snapshot = new_snap;
    if (back_focus_request[0] != '\0') {
        memcpy(front_focus_request, back_focus_request, sizeof(front_focus_request));
        back_focus_request[0] = '\0';
    }
    vd_mutex_unlock(snapshot_mutex);

    // Free old snapshot (outside lock, safe because UI no longer references it)
    free_snapshot(old_snap);
    return true;
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
    back_dirty = false;
    back_focus_request[0] = '\0';

    InstanceSnapshot *old_snap = NULL;
    vd_mutex_lock(snapshot_mutex);
    old_snap = front_snapshot;
    front_snapshot = NULL;
    front_focus_request[0] = '\0';
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

bool instance_is_focusable(const char *id)
{
    if (!id) return false;
    vd_mutex_lock(snapshot_mutex);
    ReactInstance *inst = find_front_instance_unlocked(front_snapshot, id);
    bool focusable = inst && (inst->type == NT_BUTTON || inst->type == NT_TEXT_INPUT);
    vd_mutex_unlock(snapshot_mutex);
    return focusable;
}

bool instance_text_input_session(const char *id, char **value, char **placeholder, int *max_length)
{
    if (!id) return false;
    vd_mutex_lock(snapshot_mutex);
    ReactInstance *inst = find_front_instance_unlocked(front_snapshot, id);
    bool found = inst && inst->type == NT_TEXT_INPUT;
    if (found) {
        const TextInputProps *t = &inst->props.text_input;
        *value = strdup(t->value ? t->value : "");
        *placeholder = strdup(t->placeholder ? t->placeholder : "");
        *max_length = t->max_length;
    }
    vd_mutex_unlock(snapshot_mutex);
    return found;
}

void instance_back_request_focus(const char *id)
{
    snprintf(back_focus_request, sizeof(back_focus_request), "%s", id);
    back_dirty = true;
}

bool instance_take_focus_request(char out_id[VD_INSTANCE_ID_MAX])
{
    vd_mutex_lock(snapshot_mutex);
    bool requested = front_focus_request[0] != '\0';
    if (requested) {
        memcpy(out_id, front_focus_request, VD_INSTANCE_ID_MAX);
        front_focus_request[0] = '\0';
    }
    vd_mutex_unlock(snapshot_mutex);
    return requested;
}

static bool layout_contains(const ReactInstance *inst, int abs_x, int abs_y, int x, int y)
{
    return x >= abs_x && x < abs_x + inst->layout.width && y >= abs_y && y < abs_y + inst->layout.height;
}

// Hit testing
static const char *hit_test_recursive(ReactInstance **children, int x, int y, int offset_x, int offset_y);

static const char *hit_test_instance(ReactInstance *inst, int x, int y, int offset_x, int offset_y)
{
    if (!inst) return NULL;

    const int abs_x = offset_x + inst->layout.x;
    const int abs_y = offset_y + inst->layout.y;
    const bool inside = layout_contains(inst, abs_x, abs_y, x, y);

    switch (inst->type) {
    case NT_RECT:
    case NT_STACK: {
        const char *child_hit = hit_test_recursive(inst->children, x, y, abs_x, abs_y);
        if (child_hit) return child_hit;
        // A Stack without a fill is only layout, so points between its children fall through to what is behind.
        const bool transparent = inst->type == NT_STACK && !inst->props.stack.has_fill;
        return inside && !transparent ? inst->id : NULL;
    }
    case NT_BUTTON:
    case NT_TEXT_INPUT:
        return inside ? inst->id : NULL;
    case NT_SCROLL: {
        if (!inside) return NULL;
        const char *child_hit = hit_test_recursive(inst->children, x, y, abs_x, abs_y - scroll_get_offset(inst->id));
        return child_hit ? child_hit : inst->id;
    }
    default:
        return NULL;
    }
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

// Copies under the snapshot lock: the id lives in the snapshot arena, which the JS thread frees on swap.
static bool copy_id(char out_id[VD_INSTANCE_ID_MAX], const char *id)
{
    if (!id) return false;
    snprintf(out_id, VD_INSTANCE_ID_MAX, "%s", id);
    return true;
}

bool instance_hit_test(int x, int y, char out_id[VD_INSTANCE_ID_MAX])
{
    bool hit = false;
    vd_mutex_lock(snapshot_mutex);
    if (front_snapshot) hit = copy_id(out_id, hit_test_recursive(front_snapshot->root_children, x, y, 0, 0));
    vd_mutex_unlock(snapshot_mutex);
    return hit;
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

    const int abs_x = offset_x + inst->layout.x;
    const int abs_y = offset_y + inst->layout.y;
    const int width = inst->layout.width;
    const int height = inst->layout.height;

    if (inst->type == NT_BUTTON || inst->type == NT_TEXT_INPUT) {
        add_focusable_if_visible(inst->id, abs_x, abs_y, width, height, has_clip, clip_x, clip_y, clip_width,
                                 clip_height, out_elems);
    } else if (inst->type == NT_RECT || inst->type == NT_STACK) {
        collect_focusable_recursive(inst->children, abs_x, abs_y, has_clip, clip_x, clip_y, clip_width, clip_height,
                                    out_elems);
    } else if (inst->type == NT_SCROLL) {
        add_focusable_if_visible(inst->id, abs_x, abs_y, width, height, has_clip, clip_x, clip_y, clip_width,
                                 clip_height, out_elems);

        bool child_has_clip = false;
        int child_clip_x = 0, child_clip_y = 0, child_clip_width = 0, child_clip_height = 0;
        if (!intersect_clip(has_clip, clip_x, clip_y, clip_width, clip_height, abs_x, abs_y, width, height,
                            &child_has_clip, &child_clip_x, &child_clip_y, &child_clip_width, &child_clip_height)) {
            return;
        }

        collect_focusable_recursive(inst->children, abs_x, abs_y - scroll_get_offset(inst->id), child_has_clip,
                                    child_clip_x, child_clip_y, child_clip_width, child_clip_height, out_elems);
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
            *viewport_height = inst->layout.height;
            *content_height = inst->layout.content_height;
            found = true;
        }
    }
    vd_mutex_unlock(snapshot_mutex);
    return found;
}

static ReactInstance *scroll_at_instance(ReactInstance *inst, int x, int y, int offset_x, int offset_y);

// The last child's match wins, since later children are on top.
static ReactInstance *scroll_at_children(ReactInstance **children, int x, int y, int offset_x, int offset_y)
{
    ReactInstance *found = NULL;
    int count = arrlen(children);
    for (int i = 0; i < count; i++) {
        ReactInstance *hit = scroll_at_instance(children[i], x, y, offset_x, offset_y);
        if (hit) found = hit;
    }
    return found;
}

static ReactInstance *scroll_at_instance(ReactInstance *inst, int x, int y, int offset_x, int offset_y)
{
    if (!inst) return NULL;

    const int abs_x = offset_x + inst->layout.x;
    const int abs_y = offset_y + inst->layout.y;
    if (inst->type == NT_RECT || inst->type == NT_STACK) return scroll_at_children(inst->children, x, y, abs_x, abs_y);
    if (inst->type != NT_SCROLL || !layout_contains(inst, abs_x, abs_y, x, y)) return NULL;

    /* Prefer a nested scroll container under the point. */
    ReactInstance *nested = scroll_at_children(inst->children, x, y, abs_x, abs_y - scroll_get_offset(inst->id));
    return nested ? nested : inst;
}

bool instance_scroll_for_descendant(const char *id, char out_id[VD_INSTANCE_ID_MAX])
{
    if (!id) return false;

    bool found = false;
    vd_mutex_lock(snapshot_mutex);
    if (front_snapshot) {
        ReactInstance *inst = find_front_instance_unlocked(front_snapshot, id);
        while (inst) {
            if (inst->type == NT_SCROLL && inst->id) {
                found = copy_id(out_id, inst->id);
                break;
            }
            inst = inst->parent;
        }
    }
    vd_mutex_unlock(snapshot_mutex);
    return found;
}

bool instance_scroll_at(int x, int y, char out_id[VD_INSTANCE_ID_MAX])
{
    bool found = false;
    vd_mutex_lock(snapshot_mutex);
    if (front_snapshot) {
        ReactInstance *scroll = scroll_at_children(front_snapshot->root_children, x, y, 0, 0);
        found = scroll && copy_id(out_id, scroll->id);
    }
    vd_mutex_unlock(snapshot_mutex);
    return found;
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
