// JS bindings for instance tree operations

#include "jslib_internal.h"
#include "ui/images.h"
#include "ui/instance_tree.h"

// Input events and hit tests copy ids into VD_INSTANCE_ID_MAX buffers, so an id that does not fit is rejected.
static const char *to_new_instance_id(JSContext *ctx, JSValueConst value)
{
    const char *id = JS_ToCString(ctx, value);
    if (id && strlen(id) >= VD_INSTANCE_ID_MAX) {
        TraceLog(LOG_WARNING, "Instance id is longer than %d characters: %s", VD_INSTANCE_ID_MAX - 1, id);
        JS_FreeCString(ctx, id);
        return NULL;
    }
    return id;
}

static JSValue native_create_rect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 16) return JS_UNDEFINED;

    const char *id = to_new_instance_id(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_RECT;

    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]);
    inst->props.rect.x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]);
    inst->props.rect.y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]);
    inst->props.rect.width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]);
    inst->props.rect.height = tmp;
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
    double border_radius;
    JS_ToFloat64(ctx, &border_radius, argv[15]);
    inst->props.rect.border_radius = (float)border_radius;
    inst->children = NULL;
    inst->parent = NULL;

    instance_back_put(inst);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static void read_text_layout_props(JSContext *ctx, TextProps *text, JSValueConst *argv, int layout_start)
{
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[layout_start]);
    text->x = tmp;
    JS_ToInt32(ctx, &tmp, argv[layout_start + 1]);
    text->y = tmp;
    JS_ToInt32(ctx, &tmp, argv[layout_start + 2]);
    text->width = tmp;
    JS_ToInt32(ctx, &tmp, argv[layout_start + 3]);
    text->line_height = tmp;
    JS_ToInt32(ctx, &tmp, argv[layout_start + 4]);
    text->align = (TextAlign)tmp;
    JS_ToInt32(ctx, &tmp, argv[layout_start + 5]);
    text->wrap = (TextWrap)tmp;
}

static JSValue native_create_text(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 15) return JS_UNDEFINED;

    const char *id = to_new_instance_id(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    const char *font_name = JS_ToCString(ctx, argv[1]);

    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_TEXT;
    inst->props.text.font_name = strdup(font_name ? font_name : "default");

    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[2]);
    inst->props.text.font_size = tmp;
    inst->props.text.has_color = JS_ToBool(ctx, argv[3]);
    if (inst->props.text.has_color) {
        int32_t r, g, b, a;
        JS_ToInt32(ctx, &r, argv[4]);
        JS_ToInt32(ctx, &g, argv[5]);
        JS_ToInt32(ctx, &b, argv[6]);
        JS_ToInt32(ctx, &a, argv[7]);
        inst->props.text.color = (Color){r, g, b, a};
    }
    inst->props.text.border = JS_ToBool(ctx, argv[8]);
    read_text_layout_props(ctx, &inst->props.text, argv, 9);
    inst->children = NULL;
    inst->parent = NULL;

    instance_back_put(inst);
    JS_FreeCString(ctx, id);
    JS_FreeCString(ctx, font_name);
    return JS_UNDEFINED;
}

// Everything but the label, which create and update own differently.
static void read_button_props(JSContext *ctx, ButtonProps *button, JSValueConst *argv)
{
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]);
    button->x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]);
    button->y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]);
    button->width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]);
    button->height = tmp;

    int32_t r, g, b, a;
    JS_ToInt32(ctx, &r, argv[5]);
    JS_ToInt32(ctx, &g, argv[6]);
    JS_ToInt32(ctx, &b, argv[7]);
    JS_ToInt32(ctx, &a, argv[8]);
    button->color = (Color){r, g, b, a};

    JS_ToInt32(ctx, &tmp, argv[10]);
    button->font_size = tmp;

    double border_radius;
    JS_ToFloat64(ctx, &border_radius, argv[11]);
    button->border_radius = (float)border_radius;

    JS_ToInt32(ctx, &r, argv[12]);
    JS_ToInt32(ctx, &g, argv[13]);
    JS_ToInt32(ctx, &b, argv[14]);
    JS_ToInt32(ctx, &a, argv[15]);
    button->text_color = (Color){r, g, b, a};

    button->has_focus_color = JS_ToBool(ctx, argv[16]);
    JS_ToInt32(ctx, &r, argv[17]);
    JS_ToInt32(ctx, &g, argv[18]);
    JS_ToInt32(ctx, &b, argv[19]);
    JS_ToInt32(ctx, &a, argv[20]);
    button->focus_color = (Color){r, g, b, a};
}

static JSValue native_create_button(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 21) return JS_UNDEFINED;

    const char *id = to_new_instance_id(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_BUTTON;
    read_button_props(ctx, &inst->props.button, argv);

    const char *label = JS_ToCString(ctx, argv[9]);
    inst->props.button.label = strdup(label ? label : "");
    JS_FreeCString(ctx, label);

    inst->children = NULL;
    inst->parent = NULL;

    instance_back_put(inst);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static void read_scroll_props(JSContext *ctx, ScrollProps *scroll, JSValueConst *argv)
{
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]);
    scroll->x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]);
    scroll->y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]);
    scroll->width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]);
    scroll->height = tmp;
    scroll->has_fill = JS_ToBool(ctx, argv[5]);
    if (scroll->has_fill) {
        int32_t r, g, b, a;
        JS_ToInt32(ctx, &r, argv[6]);
        JS_ToInt32(ctx, &g, argv[7]);
        JS_ToInt32(ctx, &b, argv[8]);
        JS_ToInt32(ctx, &a, argv[9]);
        scroll->fill_color = (Color){r, g, b, a};
    }
    JS_ToInt32(ctx, &tmp, argv[10]);
    scroll->gap = tmp;
    JS_ToInt32(ctx, &tmp, argv[11]);
    scroll->padding = tmp;
}

static JSValue native_create_scroll(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 12) return JS_UNDEFINED;

    const char *id = to_new_instance_id(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_SCROLL;
    read_scroll_props(ctx, &inst->props.scroll, argv);
    inst->children = NULL;
    inst->parent = NULL;

    instance_back_put(inst);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_scroll(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 12) return JS_UNDEFINED;

    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = instance_back_find(id);
    if (!inst || inst->type != NT_SCROLL) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }

    read_scroll_props(ctx, &inst->props.scroll, argv);

    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static void read_image_props(JSContext *ctx, ImageProps *image, JSValueConst *argv, const char *image_name)
{
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[2]);
    image->x = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]);
    image->y = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]);
    int req_width = tmp;
    JS_ToInt32(ctx, &tmp, argv[5]);
    int req_height = tmp;

    int resolved_width = 0;
    int resolved_height = 0;
    if (image_resolve_layout(image_name, req_width, req_height, &resolved_width, &resolved_height)) {
        image->width = resolved_width;
        image->height = resolved_height;
    } else {
        image->width = req_width > 0 ? req_width : 0;
        image->height = req_height > 0 ? req_height : 0;
    }
}

static JSValue native_create_image(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 6) return JS_UNDEFINED;

    const char *id = to_new_instance_id(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    const char *image_name = JS_ToCString(ctx, argv[1]);
    if (!image_name) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }

    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_IMAGE;
    inst->props.image.image_name = strdup(image_name);
    read_image_props(ctx, &inst->props.image, argv, image_name);
    inst->children = NULL;
    inst->parent = NULL;

    instance_back_put(inst);
    JS_FreeCString(ctx, id);
    JS_FreeCString(ctx, image_name);
    return JS_UNDEFINED;
}

static JSValue native_update_image(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 6) return JS_UNDEFINED;

    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    const char *image_name = JS_ToCString(ctx, argv[1]);
    if (!image_name) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }

    ReactInstance *inst = instance_back_find(id);
    if (!inst || inst->type != NT_IMAGE) {
        JS_FreeCString(ctx, id);
        JS_FreeCString(ctx, image_name);
        return JS_UNDEFINED;
    }

    if (inst->props.image.image_name) free(inst->props.image.image_name);
    inst->props.image.image_name = strdup(image_name);
    read_image_props(ctx, &inst->props.image, argv, image_name);

    JS_FreeCString(ctx, id);
    JS_FreeCString(ctx, image_name);
    return JS_UNDEFINED;
}

static JSValue native_create_raw_text(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 2) return JS_UNDEFINED;

    const char *id = to_new_instance_id(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    const char *text = JS_ToCString(ctx, argv[1]);

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
    instance_back_mark_dirty();
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
    instance_back_mark_dirty();
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
    instance_back_mark_dirty();
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
    instance_back_mark_dirty();
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
    instance_back_mark_dirty();
    if (argc < 16) return JS_UNDEFINED;

    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = instance_back_find(id);
    if (!inst || inst->type != NT_RECT) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }

    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]);
    inst->props.rect.x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]);
    inst->props.rect.y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]);
    inst->props.rect.width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]);
    inst->props.rect.height = tmp;
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
    double border_radius;
    JS_ToFloat64(ctx, &border_radius, argv[15]);
    inst->props.rect.border_radius = (float)border_radius;

    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_text(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 15) return JS_UNDEFINED;

    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;
    const char *font_name = JS_ToCString(ctx, argv[1]);

    ReactInstance *inst = instance_back_find(id);
    if (!inst || inst->type != NT_TEXT) {
        JS_FreeCString(ctx, id);
        JS_FreeCString(ctx, font_name);
        return JS_UNDEFINED;
    }

    if (inst->props.text.font_name) free(inst->props.text.font_name);
    inst->props.text.font_name = strdup(font_name ? font_name : "default");

    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[2]);
    inst->props.text.font_size = tmp;
    inst->props.text.has_color = JS_ToBool(ctx, argv[3]);
    if (inst->props.text.has_color) {
        int32_t r, g, b, a;
        JS_ToInt32(ctx, &r, argv[4]);
        JS_ToInt32(ctx, &g, argv[5]);
        JS_ToInt32(ctx, &b, argv[6]);
        JS_ToInt32(ctx, &a, argv[7]);
        inst->props.text.color = (Color){r, g, b, a};
    }
    inst->props.text.border = JS_ToBool(ctx, argv[8]);
    read_text_layout_props(ctx, &inst->props.text, argv, 9);

    JS_FreeCString(ctx, font_name);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_button(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < 21) return JS_UNDEFINED;

    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = instance_back_find(id);
    if (!inst || inst->type != NT_BUTTON) {
        JS_FreeCString(ctx, id);
        return JS_UNDEFINED;
    }

    read_button_props(ctx, &inst->props.button, argv);

    if (inst->props.button.label) free(inst->props.button.label);
    const char *label = JS_ToCString(ctx, argv[9]);
    inst->props.button.label = strdup(label ? label : "");
    JS_FreeCString(ctx, label);

    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_request_focus(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 1) return JS_UNDEFINED;

    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    if (instance_back_find(id)) instance_back_request_focus(id);

    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_raw_text(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
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
    (void)ctx;
    (void)this_val;
    (void)argc;
    (void)argv;
    instance_back_mark_dirty();
    instance_back_root_clear();
    return JS_UNDEFINED;
}

void register_instance_tree(JSContext *ctx)
{
    js_set_global_function(ctx, "nativeCreateRect", native_create_rect, 16);
    js_set_global_function(ctx, "nativeCreateText", native_create_text, 15);
    js_set_global_function(ctx, "nativeCreateButton", native_create_button, 21);
    js_set_global_function(ctx, "nativeCreateScroll", native_create_scroll, 12);
    js_set_global_function(ctx, "nativeCreateImage", native_create_image, 6);
    js_set_global_function(ctx, "nativeCreateRawText", native_create_raw_text, 2);
    js_set_global_function(ctx, "nativeAppendChild", native_append_child, 2);
    js_set_global_function(ctx, "nativeInsertBefore", native_insert_before, 3);
    js_set_global_function(ctx, "nativeRemoveChild", native_remove_child, 2);
    js_set_global_function(ctx, "nativeDestroyInstance", native_destroy_instance, 1);
    js_set_global_function(ctx, "nativeUpdateRect", native_update_rect, 16);
    js_set_global_function(ctx, "nativeUpdateText", native_update_text, 15);
    js_set_global_function(ctx, "nativeUpdateButton", native_update_button, 21);
    js_set_global_function(ctx, "nativeUpdateScroll", native_update_scroll, 12);
    js_set_global_function(ctx, "nativeUpdateImage", native_update_image, 6);
    js_set_global_function(ctx, "nativeUpdateRawText", native_update_raw_text, 2);
    js_set_global_function(ctx, "nativeRequestFocus", native_request_focus, 1);
    js_set_global_function(ctx, "nativeClearContainer", native_clear_container, 0);
}
