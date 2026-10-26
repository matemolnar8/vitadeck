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

static Color read_color(JSContext *ctx, JSValueConst *argv)
{
    int32_t r, g, b, a;
    JS_ToInt32(ctx, &r, argv[0]);
    JS_ToInt32(ctx, &g, argv[1]);
    JS_ToInt32(ctx, &b, argv[2]);
    JS_ToInt32(ctx, &a, argv[3]);
    return (Color){r, g, b, a};
}

static void replace_string(JSContext *ctx, char **slot, JSValueConst value, const char *fallback)
{
    const char *str = JS_ToCString(ctx, value);
    free(*slot);
    *slot = strdup(str ? str : fallback);
    JS_FreeCString(ctx, str);
}

#define BUTTON_ARGC 23

// Create and update share every argument; the strings replace any the instance already owns.
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
    button->color = read_color(ctx, argv + 5);
    replace_string(ctx, &button->label, argv[9], "");
    replace_string(ctx, &button->font_name, argv[10], "default");
    JS_ToInt32(ctx, &tmp, argv[11]);
    button->font_size = tmp;
    double border_radius;
    JS_ToFloat64(ctx, &border_radius, argv[12]);
    button->border_radius = (float)border_radius;
    button->text_color = read_color(ctx, argv + 13);
    button->has_focus_color = JS_ToBool(ctx, argv[17]);
    button->focus_color = read_color(ctx, argv + 18);
    JS_ToInt32(ctx, &tmp, argv[22]);
    button->align = (TextAlign)tmp;
}

static JSValue native_create_button(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < BUTTON_ARGC) return JS_UNDEFINED;

    const char *id = to_new_instance_id(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_BUTTON;
    read_button_props(ctx, &inst->props.button, argv);

    instance_back_put(inst);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_button(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < BUTTON_ARGC) return JS_UNDEFINED;

    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = instance_back_find(id);
    if (inst && inst->type == NT_BUTTON) read_button_props(ctx, &inst->props.button, argv);

    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

#define TEXT_INPUT_ARGC 24

// Create and update share every argument; the strings replace any the instance already owns.
static void read_text_input_props(JSContext *ctx, TextInputProps *t, JSValueConst *argv)
{
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]);
    t->x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]);
    t->y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]);
    t->width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]);
    t->height = tmp;
    t->color = read_color(ctx, argv + 5);
    t->text_color = read_color(ctx, argv + 9);
    t->has_focus_color = JS_ToBool(ctx, argv[13]);
    t->focus_color = read_color(ctx, argv + 14);
    replace_string(ctx, &t->font_name, argv[18], "default");
    JS_ToInt32(ctx, &tmp, argv[19]);
    t->font_size = tmp;
    double border_radius;
    JS_ToFloat64(ctx, &border_radius, argv[20]);
    t->border_radius = (float)border_radius;
    JS_ToInt32(ctx, &tmp, argv[21]);
    t->max_length = tmp;
    replace_string(ctx, &t->value, argv[22], "");
    replace_string(ctx, &t->placeholder, argv[23], "");
}

static JSValue native_create_text_input(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < TEXT_INPUT_ARGC) return JS_UNDEFINED;

    const char *id = to_new_instance_id(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = NT_TEXT_INPUT;
    read_text_input_props(ctx, &inst->props.text_input, argv);

    instance_back_put(inst);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_update_text_input(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    instance_back_mark_dirty();
    if (argc < TEXT_INPUT_ARGC) return JS_UNDEFINED;

    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = instance_back_find(id);
    if (inst && inst->type == NT_TEXT_INPUT) read_text_input_props(ctx, &inst->props.text_input, argv);

    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

#define SCROLL_ARGC 12
#define STACK_ARGC 14

// Scroll and Stack share their first SCROLL_ARGC arguments; a Scroll is always a start-aligned column.
static void read_stack_props(JSContext *ctx, StackProps *stack, NodeType type, JSValueConst *argv)
{
    int32_t tmp;
    JS_ToInt32(ctx, &tmp, argv[1]);
    stack->x = tmp;
    JS_ToInt32(ctx, &tmp, argv[2]);
    stack->y = tmp;
    JS_ToInt32(ctx, &tmp, argv[3]);
    stack->width = tmp;
    JS_ToInt32(ctx, &tmp, argv[4]);
    stack->height = tmp;
    stack->has_fill = JS_ToBool(ctx, argv[5]);
    if (stack->has_fill) stack->fill_color = read_color(ctx, argv + 6);
    JS_ToInt32(ctx, &tmp, argv[10]);
    stack->gap = tmp;
    JS_ToInt32(ctx, &tmp, argv[11]);
    stack->padding = tmp;
    stack->direction = STACK_COLUMN;
    stack->align = STACK_ALIGN_START;
    if (type == NT_SCROLL) return;
    JS_ToInt32(ctx, &tmp, argv[12]);
    stack->direction = tmp == STACK_ROW ? STACK_ROW : STACK_COLUMN;
    JS_ToInt32(ctx, &tmp, argv[13]);
    stack->align = tmp == STACK_ALIGN_CENTER ? STACK_ALIGN_CENTER
                   : tmp == STACK_ALIGN_END  ? STACK_ALIGN_END
                                             : STACK_ALIGN_START;
}

static JSValue create_stack(JSContext *ctx, int argc, JSValueConst *argv, NodeType type, int min_argc)
{
    instance_back_mark_dirty();
    if (argc < min_argc) return JS_UNDEFINED;

    const char *id = to_new_instance_id(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = calloc(1, sizeof(ReactInstance));
    inst->id = strdup(id);
    inst->type = type;
    read_stack_props(ctx, &inst->props.stack, type, argv);

    instance_back_put(inst);
    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue update_stack(JSContext *ctx, int argc, JSValueConst *argv, NodeType type, int min_argc)
{
    instance_back_mark_dirty();
    if (argc < min_argc) return JS_UNDEFINED;

    const char *id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    ReactInstance *inst = instance_back_find(id);
    if (inst && inst->type == type) read_stack_props(ctx, &inst->props.stack, type, argv);

    JS_FreeCString(ctx, id);
    return JS_UNDEFINED;
}

static JSValue native_create_scroll(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    return create_stack(ctx, argc, argv, NT_SCROLL, SCROLL_ARGC);
}

static JSValue native_update_scroll(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    return update_stack(ctx, argc, argv, NT_SCROLL, SCROLL_ARGC);
}

static JSValue native_create_stack(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    return create_stack(ctx, argc, argv, NT_STACK, STACK_ARGC);
}

static JSValue native_update_stack(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    return update_stack(ctx, argc, argv, NT_STACK, STACK_ARGC);
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

// Takes child out of its parent's children, or the root's. React moves an attached child by appending or
// inserting it again, so every link change starts here and a child is never listed twice.
static void unlink_child(ReactInstance *child)
{
    if (!child->parent) {
        instance_back_root_remove(child);
        return;
    }
    ReactInstance *parent = child->parent;
    for (int i = 0; i < arrlen(parent->children); i++) {
        if (parent->children[i] == child) {
            arrdel(parent->children, i);
            break;
        }
    }
    child->parent = NULL;
}

// React only calls detachDeletedInstance for host components, never for text instances, so raw text is freed
// here: when React removes it directly, or when its parent is destroyed.
static void destroy_raw_text(ReactInstance *inst)
{
    instance_back_del(inst->id);
    instance_back_free(inst);
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
        unlink_child(child);
        instance_back_root_append(child);
    } else {
        ReactInstance *parent = instance_back_find(parent_id);
        if (parent) {
            unlink_child(child);
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
        unlink_child(child);
        instance_back_root_insert(child, before);
    } else {
        ReactInstance *parent = instance_back_find(parent_id);
        if (!parent) {
            JS_FreeCString(ctx, parent_id);
            JS_FreeCString(ctx, child_id);
            JS_FreeCString(ctx, before_id);
            return JS_UNDEFINED;
        }
        unlink_child(child);
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

    const char *child_id = JS_ToCString(ctx, argv[1]);
    if (!child_id) return JS_UNDEFINED;

    ReactInstance *child = instance_back_find(child_id);
    if (child) {
        unlink_child(child);
        if (child->type == NT_RAW_TEXT) destroy_raw_text(child);
    }
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

    // React destroys a deleted subtree in no particular order, so leave no links to inst behind.
    unlink_child(inst);
    for (int i = 0; i < arrlen(inst->children); i++) {
        ReactInstance *child = inst->children[i];
        if (child->type == NT_RAW_TEXT) {
            destroy_raw_text(child);
        } else {
            child->parent = NULL;
        }
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
    js_set_global_function(ctx, "nativeCreateButton", native_create_button, BUTTON_ARGC);
    js_set_global_function(ctx, "nativeCreateTextInput", native_create_text_input, TEXT_INPUT_ARGC);
    js_set_global_function(ctx, "nativeCreateScroll", native_create_scroll, SCROLL_ARGC);
    js_set_global_function(ctx, "nativeCreateStack", native_create_stack, STACK_ARGC);
    js_set_global_function(ctx, "nativeCreateImage", native_create_image, 6);
    js_set_global_function(ctx, "nativeCreateRawText", native_create_raw_text, 2);
    js_set_global_function(ctx, "nativeAppendChild", native_append_child, 2);
    js_set_global_function(ctx, "nativeInsertBefore", native_insert_before, 3);
    js_set_global_function(ctx, "nativeRemoveChild", native_remove_child, 2);
    js_set_global_function(ctx, "nativeDestroyInstance", native_destroy_instance, 1);
    js_set_global_function(ctx, "nativeUpdateRect", native_update_rect, 16);
    js_set_global_function(ctx, "nativeUpdateText", native_update_text, 15);
    js_set_global_function(ctx, "nativeUpdateButton", native_update_button, BUTTON_ARGC);
    js_set_global_function(ctx, "nativeUpdateTextInput", native_update_text_input, TEXT_INPUT_ARGC);
    js_set_global_function(ctx, "nativeUpdateScroll", native_update_scroll, SCROLL_ARGC);
    js_set_global_function(ctx, "nativeUpdateStack", native_update_stack, STACK_ARGC);
    js_set_global_function(ctx, "nativeUpdateImage", native_update_image, 6);
    js_set_global_function(ctx, "nativeUpdateRawText", native_update_raw_text, 2);
    js_set_global_function(ctx, "nativeRequestFocus", native_request_focus, 1);
    js_set_global_function(ctx, "nativeClearContainer", native_clear_container, 0);
}
