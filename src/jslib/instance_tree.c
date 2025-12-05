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

static InstanceEntry *instance_registry = NULL;
static ReactInstance **root_children = NULL;

static ReactInstance *find_instance(const char *id)
{
    if (!id) return NULL;
    int idx = shgeti(instance_registry, id);
    if (idx < 0) return NULL;
    return instance_registry[idx].value;
}

ReactInstance *instance_find_by_id(const char *id)
{
    return find_instance(id);
}

bool instance_exists(const char *id)
{
    return find_instance(id) != NULL;
}

ReactInstance **instance_get_root_children(void)
{
    return root_children;
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
    return hit_test_recursive(root_children, x, y, 0, 0);
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
// Native Mutation Functions
// =============================================================================

static void native_create_rect(js_State *J)
{
    const char *id = js_tostring(J, 1);
    
    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_RECT;
    inst->props.rect.x = js_tointeger(J, 2);
    inst->props.rect.y = js_tointeger(J, 3);
    inst->props.rect.width = js_tointeger(J, 4);
    inst->props.rect.height = js_tointeger(J, 5);
    inst->props.rect.has_fill = js_toboolean(J, 6);
    if (inst->props.rect.has_fill) {
        inst->props.rect.fill_color = (Color){
            (unsigned char)js_tointeger(J, 7),
            (unsigned char)js_tointeger(J, 8),
            (unsigned char)js_tointeger(J, 9),
            (unsigned char)js_tointeger(J, 10)
        };
    }
    inst->props.rect.has_outline = js_toboolean(J, 11);
    if (inst->props.rect.has_outline) {
        inst->props.rect.border_color = (Color){
            (unsigned char)js_tointeger(J, 12),
            (unsigned char)js_tointeger(J, 13),
            (unsigned char)js_tointeger(J, 14),
            (unsigned char)js_tointeger(J, 15)
        };
    }
    inst->children = NULL;
    inst->parent = NULL;
    
    shput(instance_registry, inst->id, inst);
    js_pushundefined(J);
}

static void native_create_text(js_State *J)
{
    const char *id = js_tostring(J, 1);
    
    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_TEXT;
    inst->props.text.font_size = js_tointeger(J, 2);
    inst->props.text.has_color = js_toboolean(J, 3);
    if (inst->props.text.has_color) {
        inst->props.text.color = (Color){
            (unsigned char)js_tointeger(J, 4),
            (unsigned char)js_tointeger(J, 5),
            (unsigned char)js_tointeger(J, 6),
            (unsigned char)js_tointeger(J, 7)
        };
    }
    inst->props.text.border = js_toboolean(J, 8);
    inst->children = NULL;
    inst->parent = NULL;
    
    shput(instance_registry, inst->id, inst);
    js_pushundefined(J);
}

static void native_create_button(js_State *J)
{
    const char *id = js_tostring(J, 1);
    
    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_BUTTON;
    inst->props.button.x = js_tointeger(J, 2);
    inst->props.button.y = js_tointeger(J, 3);
    inst->props.button.width = js_tointeger(J, 4);
    inst->props.button.height = js_tointeger(J, 5);
    inst->props.button.color = (Color){
        (unsigned char)js_tointeger(J, 6),
        (unsigned char)js_tointeger(J, 7),
        (unsigned char)js_tointeger(J, 8),
        (unsigned char)js_tointeger(J, 9)
    };
    inst->props.button.label = strdup(js_tostring(J, 10));
    inst->props.button.font_size = js_tointeger(J, 11);
    inst->children = NULL;
    inst->parent = NULL;
    
    shput(instance_registry, inst->id, inst);
    js_pushundefined(J);
}

static void native_create_raw_text(js_State *J)
{
    const char *id = js_tostring(J, 1);
    const char *text = js_tostring(J, 2);
    
    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_RAW_TEXT;
    inst->props.raw_text = strdup(text);
    inst->children = NULL;
    inst->parent = NULL;
    
    shput(instance_registry, inst->id, inst);
    js_pushundefined(J);
}

static void native_append_child(js_State *J)
{
    const char *parent_id = js_tostring(J, 1);
    const char *child_id = js_tostring(J, 2);
    
    ReactInstance *child = find_instance(child_id);
    if (!child) {
        js_pushundefined(J);
        return;
    }
    
    if (parent_id[0] == '\0') {
        child->parent = NULL;
        arrput(root_children, child);
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (parent) {
            child->parent = parent;
            arrput(parent->children, child);
        }
    }
    
    js_pushundefined(J);
}

static void native_insert_before(js_State *J)
{
    const char *parent_id = js_tostring(J, 1);
    const char *child_id = js_tostring(J, 2);
    const char *before_id = js_tostring(J, 3);
    
    ReactInstance *child = find_instance(child_id);
    ReactInstance *before = find_instance(before_id);
    if (!child) {
        js_pushundefined(J);
        return;
    }
    
    ReactInstance **children_arr;
    if (parent_id[0] == '\0') {
        children_arr = root_children;
        child->parent = NULL;
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (!parent) {
            js_pushundefined(J);
            return;
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
        root_children = children_arr;
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (parent) parent->children = children_arr;
    }
    
    js_pushundefined(J);
}

static void native_remove_child(js_State *J)
{
    const char *parent_id = js_tostring(J, 1);
    const char *child_id = js_tostring(J, 2);
    
    ReactInstance *child = find_instance(child_id);
    if (!child) {
        js_pushundefined(J);
        return;
    }
    
    ReactInstance **children_arr;
    if (parent_id[0] == '\0') {
        children_arr = root_children;
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (!parent) {
            js_pushundefined(J);
            return;
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
        root_children = children_arr;
    } else {
        ReactInstance *parent = find_instance(parent_id);
        if (parent) parent->children = children_arr;
    }
    
    child->parent = NULL;
    js_pushundefined(J);
}

static void native_destroy_instance(js_State *J)
{
    const char *id = js_tostring(J, 1);
    
    ReactInstance *inst = find_instance(id);
    if (!inst) {
        js_pushundefined(J);
        return;
    }
    
    shdel(instance_registry, id);
    free_instance(inst);
    
    js_pushundefined(J);
}

static void native_update_rect(js_State *J)
{
    const char *id = js_tostring(J, 1);
    ReactInstance *inst = find_instance(id);
    if (!inst || inst->type != NT_RECT) {
        js_pushundefined(J);
        return;
    }
    
    inst->props.rect.x = js_tointeger(J, 2);
    inst->props.rect.y = js_tointeger(J, 3);
    inst->props.rect.width = js_tointeger(J, 4);
    inst->props.rect.height = js_tointeger(J, 5);
    inst->props.rect.has_fill = js_toboolean(J, 6);
    if (inst->props.rect.has_fill) {
        inst->props.rect.fill_color = (Color){
            (unsigned char)js_tointeger(J, 7),
            (unsigned char)js_tointeger(J, 8),
            (unsigned char)js_tointeger(J, 9),
            (unsigned char)js_tointeger(J, 10)
        };
    }
    inst->props.rect.has_outline = js_toboolean(J, 11);
    if (inst->props.rect.has_outline) {
        inst->props.rect.border_color = (Color){
            (unsigned char)js_tointeger(J, 12),
            (unsigned char)js_tointeger(J, 13),
            (unsigned char)js_tointeger(J, 14),
            (unsigned char)js_tointeger(J, 15)
        };
    }
    
    js_pushundefined(J);
}

static void native_update_text(js_State *J)
{
    const char *id = js_tostring(J, 1);
    ReactInstance *inst = find_instance(id);
    if (!inst || inst->type != NT_TEXT) {
        js_pushundefined(J);
        return;
    }
    
    inst->props.text.font_size = js_tointeger(J, 2);
    inst->props.text.has_color = js_toboolean(J, 3);
    if (inst->props.text.has_color) {
        inst->props.text.color = (Color){
            (unsigned char)js_tointeger(J, 4),
            (unsigned char)js_tointeger(J, 5),
            (unsigned char)js_tointeger(J, 6),
            (unsigned char)js_tointeger(J, 7)
        };
    }
    inst->props.text.border = js_toboolean(J, 8);
    
    js_pushundefined(J);
}

static void native_update_button(js_State *J)
{
    const char *id = js_tostring(J, 1);
    ReactInstance *inst = find_instance(id);
    if (!inst || inst->type != NT_BUTTON) {
        js_pushundefined(J);
        return;
    }
    
    inst->props.button.x = js_tointeger(J, 2);
    inst->props.button.y = js_tointeger(J, 3);
    inst->props.button.width = js_tointeger(J, 4);
    inst->props.button.height = js_tointeger(J, 5);
    inst->props.button.color = (Color){
        (unsigned char)js_tointeger(J, 6),
        (unsigned char)js_tointeger(J, 7),
        (unsigned char)js_tointeger(J, 8),
        (unsigned char)js_tointeger(J, 9)
    };
    if (inst->props.button.label) free(inst->props.button.label);
    inst->props.button.label = strdup(js_tostring(J, 10));
    inst->props.button.font_size = js_tointeger(J, 11);
    
    js_pushundefined(J);
}

static void native_update_raw_text(js_State *J)
{
    const char *id = js_tostring(J, 1);
    ReactInstance *inst = find_instance(id);
    if (!inst || inst->type != NT_RAW_TEXT) {
        js_pushundefined(J);
        return;
    }
    
    if (inst->props.raw_text) free(inst->props.raw_text);
    inst->props.raw_text = strdup(js_tostring(J, 2));
    
    js_pushundefined(J);
}

static void native_clear_container(js_State *J)
{
    arrfree(root_children);
    root_children = NULL;
    js_pushundefined(J);
}

void register_instance_tree(js_State *J)
{
    js_newcfunction(J, native_create_rect, "nativeCreateRect", 0);
    js_setglobal(J, "nativeCreateRect");
    
    js_newcfunction(J, native_create_text, "nativeCreateText", 0);
    js_setglobal(J, "nativeCreateText");
    
    js_newcfunction(J, native_create_button, "nativeCreateButton", 0);
    js_setglobal(J, "nativeCreateButton");
    
    js_newcfunction(J, native_create_raw_text, "nativeCreateRawText", 0);
    js_setglobal(J, "nativeCreateRawText");
    
    js_newcfunction(J, native_append_child, "nativeAppendChild", 0);
    js_setglobal(J, "nativeAppendChild");
    
    js_newcfunction(J, native_insert_before, "nativeInsertBefore", 0);
    js_setglobal(J, "nativeInsertBefore");
    
    js_newcfunction(J, native_remove_child, "nativeRemoveChild", 0);
    js_setglobal(J, "nativeRemoveChild");
    
    js_newcfunction(J, native_destroy_instance, "nativeDestroyInstance", 0);
    js_setglobal(J, "nativeDestroyInstance");
    
    js_newcfunction(J, native_update_rect, "nativeUpdateRect", 0);
    js_setglobal(J, "nativeUpdateRect");
    
    js_newcfunction(J, native_update_text, "nativeUpdateText", 0);
    js_setglobal(J, "nativeUpdateText");
    
    js_newcfunction(J, native_update_button, "nativeUpdateButton", 0);
    js_setglobal(J, "nativeUpdateButton");
    
    js_newcfunction(J, native_update_raw_text, "nativeUpdateRawText", 0);
    js_setglobal(J, "nativeUpdateRawText");
    
    js_newcfunction(J, native_clear_container, "nativeClearContainer", 0);
    js_setglobal(J, "nativeClearContainer");
}


