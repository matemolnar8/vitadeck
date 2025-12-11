#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "stb_ds.h"
#include "instance_tree.h"
#include "platform/thread.h"

typedef struct {
    char *key;
    ReactInstance *value;
} InstanceEntry;

// Snapshot structure for thread-safe access
typedef struct {
    InstanceEntry *registry;
    ReactInstance **root_children;
} InstanceSnapshot;

// Back buffer: JS thread writes here (single-threaded access)
static InstanceEntry *back_registry = NULL;
static ReactInstance **back_root_children = NULL;

// Front snapshot: UI thread reads from here
static InstanceSnapshot* front_snapshot = NULL;

// Mutex to protect front_snapshot access during swap
static vd_mutex* snapshot_mutex = NULL;

// Free an instance (shallow, does not free children)
void instance_back_free(ReactInstance *inst)
{
    if (!inst) return;
    if (inst->id) free(inst->id);
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
static void free_instance_tree(ReactInstance* inst) {
    if (!inst) return;
    
    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        free_instance_tree(inst->children[i]);
    }
    
    if (inst->id) free(inst->id);
    if (inst->type == NT_BUTTON && inst->props.button.label) {
        free(inst->props.button.label);
    }
    if (inst->type == NT_RAW_TEXT && inst->props.raw_text) {
        free(inst->props.raw_text);
    }
    arrfree(inst->children);
    free(inst);
}

// Free a snapshot
static void free_snapshot(InstanceSnapshot* snap) {
    if (!snap) return;
    
    int count = arrlen(snap->root_children);
    for (int i = 0; i < count; i++) {
        free_instance_tree(snap->root_children[i]);
    }
    arrfree(snap->root_children);
    shfree(snap->registry);
    free(snap);
}

// Deep copy a single instance (without children links)
static ReactInstance* copy_instance(ReactInstance* src) {
    if (!src) return NULL;
    
    ReactInstance* dst = calloc(1, sizeof(ReactInstance));
    dst->id = src->id ? strdup(src->id) : NULL;
    dst->type = src->type;
    dst->children = NULL;
    dst->parent = NULL;
    
    switch (src->type) {
        case NT_RECT:
            dst->props.rect = src->props.rect;
            break;
        case NT_TEXT:
            dst->props.text = src->props.text;
            break;
        case NT_BUTTON:
            dst->props.button = src->props.button;
            dst->props.button.label = src->props.button.label ? strdup(src->props.button.label) : NULL;
            break;
        case NT_RAW_TEXT:
            dst->props.raw_text = src->props.raw_text ? strdup(src->props.raw_text) : NULL;
            break;
    }
    
    return dst;
}

// Recursively copy instance and all children
static ReactInstance* deep_copy_instance(ReactInstance* src, InstanceEntry** new_registry) {
    if (!src) return NULL;
    
    ReactInstance* dst = copy_instance(src);
    if (dst->id) {
        shput(*new_registry, dst->id, dst);
    }
    
    int child_count = arrlen(src->children);
    for (int i = 0; i < child_count; i++) {
        if (src->children[i]) {
            ReactInstance* child_copy = deep_copy_instance(src->children[i], new_registry);
            if (child_copy) {
                child_copy->parent = dst;
                arrput(dst->children, child_copy);
            }
        }
    }
    
    return dst;
}

void instance_tree_init(void) {
    if (!snapshot_mutex) {
        snapshot_mutex = vd_mutex_create();
    }
}

void instance_tree_swap(void) {
    // Create new snapshot from back buffer (outside lock)
    InstanceSnapshot* new_snap = calloc(1, sizeof(InstanceSnapshot));
    new_snap->registry = NULL;
    new_snap->root_children = NULL;
    
    int count = arrlen(back_root_children);
    for (int i = 0; i < count; i++) {
        if (back_root_children[i]) {
            ReactInstance* copy = deep_copy_instance(back_root_children[i], &new_snap->registry);
            arrput(new_snap->root_children, copy);
        }
    }
    
    // Swap under lock
    InstanceSnapshot* old_snap = NULL;
    vd_mutex_lock(snapshot_mutex);
    old_snap = front_snapshot;
    front_snapshot = new_snap;
    vd_mutex_unlock(snapshot_mutex);
    
    // Free old snapshot (outside lock, safe because UI no longer references it)
    free_snapshot(old_snap);
}

void instance_tree_render_lock(void) {
    vd_mutex_lock(snapshot_mutex);
}

void instance_tree_render_unlock(void) {
    vd_mutex_unlock(snapshot_mutex);
}

ReactInstance **instance_get_root_children(void) {
    return front_snapshot ? front_snapshot->root_children : NULL;
}

// For hit testing, find in front buffer (UI thread) - must be called under lock
static ReactInstance *find_front_instance_unlocked(InstanceSnapshot* snap, const char *id)
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

