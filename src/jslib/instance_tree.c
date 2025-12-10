#include "platform/thread.h"

typedef enum {
    NT_RECT = 1,
    NT_TEXT = 2,
    NT_BUTTON = 3,
    NT_RAW_TEXT = 4
} NodeType;

typedef struct {
    int x, y, width, height;
    bool has_fill, has_outline;
    Color fill_color, border_color;
} RectProps;

typedef struct {
    int font_size;
    bool has_color;
    Color color;
    bool border;
} TextProps;

typedef struct {
    int x, y, width, height;
    Color color;
    char *label;
    int font_size;
} ButtonProps;

typedef struct ReactInstance ReactInstance;

struct ReactInstance {
    char *id;
    NodeType type;
    union {
        RectProps rect;
        TextProps text;
        ButtonProps button;
        char *raw_text;
    } props;
    ReactInstance **children;
    ReactInstance *parent;
};

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

// Initialize the snapshot mutex (call once at startup)
void instance_tree_init(void) {
    if (!snapshot_mutex) {
        snapshot_mutex = vd_mutex_create();
    }
}

// Swap back buffer to front buffer (called from JS thread after mutations)
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

// Called from back buffer operations (JS thread)
static ReactInstance *find_instance(const char *id)
{
    if (!id) return NULL;
    int idx = shgeti(back_registry, id);
    if (idx < 0) return NULL;
    return back_registry[idx].value;
}

// For hit testing, find in front buffer (UI thread) - must be called under lock
static ReactInstance *find_front_instance_unlocked(InstanceSnapshot* snap, const char *id)
{
    if (!id || !snap) return NULL;
    int idx = shgeti(snap->registry, id);
    if (idx < 0) return NULL;
    return snap->registry[idx].value;
}

ReactInstance *instance_find_by_id(const char *id)
{
    return find_instance(id);
}

bool instance_exists(const char *id)
{
    if (!id) return false;
    vd_mutex_lock(snapshot_mutex);
    bool exists = front_snapshot && find_front_instance_unlocked(front_snapshot, id) != NULL;
    vd_mutex_unlock(snapshot_mutex);
    return exists;
}

ReactInstance **instance_get_root_children(void)
{
    // Note: caller must hold snapshot_mutex or use instance_tree_render_lock/unlock
    return front_snapshot ? front_snapshot->root_children : NULL;
}

// Lock/unlock for rendering - UI thread calls these around the entire render
void instance_tree_render_lock(void) {
    vd_mutex_lock(snapshot_mutex);
}

void instance_tree_render_unlock(void) {
    vd_mutex_unlock(snapshot_mutex);
}

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

static void free_instance(ReactInstance *inst)
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

// =============================================================================
// Native Mutation Functions (operate on back buffer)
// =============================================================================

static JSValue native_create_rect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 15) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_RECT;
    
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]); inst->props.rect.x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]); inst->props.rect.y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]); inst->props.rect.width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]); inst->props.rect.height = tmp;
    inst->props.rect.has_fill = JS_ToBool(ctx, argv[5]);
    if (inst->props.rect.has_fill) {
        int32_t r, g, b, a;
        JS_ToInt32(ctx, &r, argv[6]);
        JS_ToInt32(ctx, &g, argv[7]);
        JS_ToInt32(ctx, &b, argv[8]);
        JS_ToInt32(ctx, &a, argv[9]);
        inst->props.rect.fill_color = (Color){r, g, b, a};
    }
    inst->props.rect.has_outline = JS_ToBool(ctx, argv[10]);
    if (inst->props.rect.has_outline) {
        int32_t r, g, b, a;
        JS_ToInt32(ctx, &r, argv[11]);
        JS_ToInt32(ctx, &g, argv[12]);
        JS_ToInt32(ctx, &b, argv[13]);
        JS_ToInt32(ctx, &a, argv[14]);
        inst->props.rect.border_color = (Color){r, g, b, a};
    }
    inst->children = NULL;
    inst->parent = NULL;
    
    shput(back_registry, inst->id, inst);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_create_text(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 8) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_TEXT;
    
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]); inst->props.text.font_size = tmp;
    inst->props.text.has_color = JS_ToBool(ctx, argv[2]);
    if (inst->props.text.has_color) {
        int32_t r, g, b, a;
        JS_ToInt32(ctx, &r, argv[3]);
        JS_ToInt32(ctx, &g, argv[4]);
        JS_ToInt32(ctx, &b, argv[5]);
        JS_ToInt32(ctx, &a, argv[6]);
        inst->props.text.color = (Color){r, g, b, a};
    }
    inst->props.text.border = JS_ToBool(ctx, argv[7]);
    inst->children = NULL;
    inst->parent = NULL;
    
    shput(back_registry, inst->id, inst);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_create_button(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 11) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_BUTTON;
    
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]); inst->props.button.x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]); inst->props.button.y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]); inst->props.button.width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]); inst->props.button.height = tmp;
    
    int32_t r, g, b, a;
    JS_ToInt32(ctx, &r, argv[5]);
    JS_ToInt32(ctx, &g, argv[6]);
    JS_ToInt32(ctx, &b, argv[7]);
    JS_ToInt32(ctx, &a, argv[8]);
    inst->props.button.color = (Color){r, g, b, a};
    
    const char *label = JS_ToCString(ctx, argv[9]);
    inst->props.button.label = strdup(label ? label : "");
    JS_FreeCString(ctx, label);
    
    JS_ToInt32(ctx, &tmp, argv[10]); inst->props.button.font_size = tmp;
    inst->children = NULL;
    inst->parent = NULL;
    
    shput(back_registry, inst->id, inst);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_create_raw_text(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 2) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    const char *text = JS_ToCString(ctx, argv[1]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_RAW_TEXT;
    inst->props.raw_text = strdup(text ? text : "");
    inst->children = NULL;
    inst->parent = NULL;
    
    shput(back_registry, inst->id, inst);
    JS_FreeCString(ctx, id);
    JS_FreeCString(ctx, text);
    return JS_UNDEFINED;
}

static JSValue native_append_child(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 2) return JS_UNDEFINED;
    
    const char *parent_id = JS_ToCString(ctx, argv[0]);
    const char *child_id = JS_ToCString(ctx, argv[1]);
    if (!parent_id || !child_id) {
        JS_FreeCString(ctx, parent_id);
        JS_FreeCString(ctx, child_id);
        return JS_UNDEFINED;
    }
    
    ReactInstance *child = find_instance(child_id);
    if (!child) {
        JS_FreeCString(ctx, parent_id);
        JS_FreeCString(ctx, child_id);
        return JS_UNDEFINED;
    }
    
    if (parent_id[0] == '\0') {
        child->parent = NULL;
        arrput(back_root_children, child);
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (parent) {
            child->parent = parent;
            arrput(parent->children, child);
        }
    }
    
    JS_FreeCString(ctx, parent_id);
    JS_FreeCString(ctx, child_id);
    return JS_UNDEFINED;
}

static JSValue native_insert_before(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 3) return JS_UNDEFINED;
    
    const char *parent_id = JS_ToCString(ctx, argv[0]);
    const char *child_id = JS_ToCString(ctx, argv[1]);
    const char *before_id = JS_ToCString(ctx, argv[2]);
    
    if (!parent_id || !child_id || !before_id) {
        JS_FreeCString(ctx, parent_id);
        JS_FreeCString(ctx, child_id);
        JS_FreeCString(ctx, before_id);
        return JS_UNDEFINED;
    }
    
    ReactInstance *child = find_instance(child_id);
    ReactInstance *before = find_instance(before_id);
    if (!child) {
        JS_FreeCString(ctx, parent_id);
        JS_FreeCString(ctx, child_id);
        JS_FreeCString(ctx, before_id);
        return JS_UNDEFINED;
    }
    
    ReactInstance **children_arr;
    if (parent_id[0] == '\0') {
        children_arr = back_root_children;
        child->parent = NULL;
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (!parent) {
            JS_FreeCString(ctx, parent_id);
            JS_FreeCString(ctx, child_id);
            JS_FreeCString(ctx, before_id);
            return JS_UNDEFINED;
        }
        children_arr = parent->children;
        child->parent = parent;
    }
    
    int count = arrlen(children_arr);
    int insert_idx = count;
    for (int i = 0; i < count; i++) {
        if (children_arr[i] == before) {
            insert_idx = i;
            break;
        }
    }
    
    arrins(children_arr, insert_idx, child);
    
    if (parent_id[0] == '\0') {
        back_root_children = children_arr;
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (parent) parent->children = children_arr;
    }
    
    JS_FreeCString(ctx, parent_id);
    JS_FreeCString(ctx, child_id);
    JS_FreeCString(ctx, before_id);
    return JS_UNDEFINED;
}

static JSValue native_remove_child(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 2) return JS_UNDEFINED;
    
    const char *parent_id = JS_ToCString(ctx, argv[0]);
    const char *child_id = JS_ToCString(ctx, argv[1]);
    
    if (!parent_id || !child_id) {
        JS_FreeCString(ctx, parent_id);
        JS_FreeCString(ctx, child_id);
        return JS_UNDEFINED;
    }
    
    ReactInstance *child = find_instance(child_id);
    if (!child) {
        JS_FreeCString(ctx, parent_id);
        JS_FreeCString(ctx, child_id);
        return JS_UNDEFINED;
    }
    
    ReactInstance **children_arr;
    if (parent_id[0] == '\0') {
        children_arr = back_root_children;
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (!parent) {
            JS_FreeCString(ctx, parent_id);
            JS_FreeCString(ctx, child_id);
            return JS_UNDEFINED;
        }
        children_arr = parent->children;
    }
    
    int count = arrlen(children_arr);
    for (int i = 0; i < count; i++) {
        if (children_arr[i] == child) {
            arrdel(children_arr, i);
            break;
        }
    }
    
    if (parent_id[0] == '\0') {
        back_root_children = children_arr;
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (parent) parent->children = children_arr;
    }
    
    child->parent = NULL;
    JS_FreeCString(ctx, parent_id);
    JS_FreeCString(ctx, child_id);
    return JS_UNDEFINED;
}

static JSValue native_destroy_instance(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 1) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = find_instance(id);
    if (!inst) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }
    
    shdel(back_registry, id);
    free_instance(inst);
    
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_rect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 15) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = find_instance(id);
    if (!inst || inst->type != NT_RECT) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }
    
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]); inst->props.rect.x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]); inst->props.rect.y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]); inst->props.rect.width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]); inst->props.rect.height = tmp;
    inst->props.rect.has_fill = JS_ToBool(ctx, argv[5]);
    if (inst->props.rect.has_fill) {
        int32_t r, g, b, a;
        JS_ToInt32(ctx, &r, argv[6]);
        JS_ToInt32(ctx, &g, argv[7]);
        JS_ToInt32(ctx, &b, argv[8]);
        JS_ToInt32(ctx, &a, argv[9]);
        inst->props.rect.fill_color = (Color){r, g, b, a};
    }
    inst->props.rect.has_outline = JS_ToBool(ctx, argv[10]);
    if (inst->props.rect.has_outline) {
        int32_t r, g, b, a;
        JS_ToInt32(ctx, &r, argv[11]);
        JS_ToInt32(ctx, &g, argv[12]);
        JS_ToInt32(ctx, &b, argv[13]);
        JS_ToInt32(ctx, &a, argv[14]);
        inst->props.rect.border_color = (Color){r, g, b, a};
    }
    
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_text(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 8) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = find_instance(id);
    if (!inst || inst->type != NT_TEXT) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }
    
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]); inst->props.text.font_size = tmp;
    inst->props.text.has_color = JS_ToBool(ctx, argv[2]);
    if (inst->props.text.has_color) {
        int32_t r, g, b, a;
        JS_ToInt32(ctx, &r, argv[3]);
        JS_ToInt32(ctx, &g, argv[4]);
        JS_ToInt32(ctx, &b, argv[5]);
        JS_ToInt32(ctx, &a, argv[6]);
        inst->props.text.color = (Color){r, g, b, a};
    }
    inst->props.text.border = JS_ToBool(ctx, argv[7]);
    
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_button(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 11) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = find_instance(id);
    if (!inst || inst->type != NT_BUTTON) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }
    
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]); inst->props.button.x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]); inst->props.button.y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]); inst->props.button.width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]); inst->props.button.height = tmp;
    
    int32_t r, g, b, a;
    JS_ToInt32(ctx, &r, argv[5]);
    JS_ToInt32(ctx, &g, argv[6]);
    JS_ToInt32(ctx, &b, argv[7]);
    JS_ToInt32(ctx, &a, argv[8]);
    inst->props.button.color = (Color){r, g, b, a};
    
    if (inst->props.button.label) free(inst->props.button.label);
    const char *label = JS_ToCString(ctx, argv[9]);
    inst->props.button.label = strdup(label ? label : "");
    JS_FreeCString(ctx, label);
    
    JS_ToInt32(ctx, &tmp, argv[10]); inst->props.button.font_size = tmp;
    
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_raw_text(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 2) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = find_instance(id);
    if (!inst || inst->type != NT_RAW_TEXT) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }
    
    if (inst->props.raw_text) free(inst->props.raw_text);
    const char *text = JS_ToCString(ctx, argv[1]);
    inst->props.raw_text = strdup(text ? text : "");
    JS_FreeCString(ctx, text);
    
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_clear_container(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)ctx; (void)this_val; (void)argc; (void)argv;
    arrfree(back_root_children);
    back_root_children = NULL;
    return JS_UNDEFINED;
}

void register_instance_tree(JSContext *ctx)
{
    js_set_global_function(ctx, "nativeCreateRect", native_create_rect, 15);
    js_set_global_function(ctx, "nativeCreateText", native_create_text, 8);
    js_set_global_function(ctx, "nativeCreateButton", native_create_button, 11);
    js_set_global_function(ctx, "nativeCreateRawText", native_create_raw_text, 2);
    js_set_global_function(ctx, "nativeAppendChild", native_append_child, 2);
    js_set_global_function(ctx, "nativeInsertBefore", native_insert_before, 3);
    js_set_global_function(ctx, "nativeRemoveChild", native_remove_child, 2);
    js_set_global_function(ctx, "nativeDestroyInstance", native_destroy_instance, 1);
    js_set_global_function(ctx, "nativeUpdateRect", native_update_rect, 15);
    js_set_global_function(ctx, "nativeUpdateText", native_update_text, 8);
    js_set_global_function(ctx, "nativeUpdateButton", native_update_button, 11);
    js_set_global_function(ctx, "nativeUpdateRawText", native_update_raw_text, 2);
    js_set_global_function(ctx, "nativeClearContainer", native_clear_container, 0);
}
