// JS bindings for instance tree operations

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
    
    instance_back_put(inst);
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
    
    instance_back_put(inst);
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
    
    instance_back_put(inst);
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
    
    instance_back_put(inst);
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
    
    ReactInstance *child = instance_back_find(child_id);
    if (!child) {
        JS_FreeCString(ctx, parent_id);
        JS_FreeCString(ctx, child_id);
        return JS_UNDEFINED;
    }
    
    if (parent_id[0] == '\0') {
        instance_back_root_append(child);
    } else {
        ReactInstance *parent = instance_back_find(parent_id);
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
    
    ReactInstance *child = instance_back_find(child_id);
    ReactInstance *before = instance_back_find(before_id);
    if (!child) {
        JS_FreeCString(ctx, parent_id);
        JS_FreeCString(ctx, child_id);
        JS_FreeCString(ctx, before_id);
        return JS_UNDEFINED;
    }
    
    if (parent_id[0] == '\0') {
        instance_back_root_insert(child, before);
    } else {
        ReactInstance *parent = instance_back_find(parent_id);
        if (!parent) {
            JS_FreeCString(ctx, parent_id);
            JS_FreeCString(ctx, child_id);
            JS_FreeCString(ctx, before_id);
            return JS_UNDEFINED;
        }
        child->parent = parent;
        
        int count = arrlen(parent->children);
        int insert_idx = count;
        for (int i = 0; i < count; i++) {
            if (parent->children[i] == before) {
                insert_idx = i;
                break;
            }
        }
        arrins(parent->children, insert_idx, child);
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
    
    ReactInstance *child = instance_back_find(child_id);
    if (!child) {
        JS_FreeCString(ctx, parent_id);
        JS_FreeCString(ctx, child_id);
        return JS_UNDEFINED;
    }
    
    if (parent_id[0] == '\0') {
        instance_back_root_remove(child);
    } else {
        ReactInstance *parent = instance_back_find(parent_id);
        if (!parent) {
            JS_FreeCString(ctx, parent_id);
            JS_FreeCString(ctx, child_id);
            return JS_UNDEFINED;
        }
        
        int count = arrlen(parent->children);
        for (int i = 0; i < count; i++) {
            if (parent->children[i] == child) {
                arrdel(parent->children, i);
                break;
            }
        }
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
    
    ReactInstance *inst = instance_back_find(id);
    if (!inst) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }
    
    instance_back_del(id);
    instance_back_free(inst);
    
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_rect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 15) return JS_UNDEFINED;
    
    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    
    ReactInstance *inst = instance_back_find(id);
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
    
    ReactInstance *inst = instance_back_find(id);
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
    
    ReactInstance *inst = instance_back_find(id);
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
    
    ReactInstance *inst = instance_back_find(id);
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
    instance_back_root_clear();
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
