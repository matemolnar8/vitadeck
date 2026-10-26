#include "layout.h"
#include "stb_ds.h"
#include "text.h"

// Bounds as the props state them; a Stack's 0 width or height is fitted later.
static InstanceLayout own_bounds(const ReactInstance *inst)
{
    switch (inst->type) {
    case NT_RECT: {
        const RectProps *p = &inst->props.rect;
        return (InstanceLayout){.x = p->x, .y = p->y, .width = p->width, .height = p->height};
    }
    case NT_BUTTON: {
        const ButtonProps *p = &inst->props.button;
        return (InstanceLayout){.x = p->x, .y = p->y, .width = p->width, .height = p->height};
    }
    case NT_TEXT_INPUT: {
        const TextInputProps *p = &inst->props.text_input;
        return (InstanceLayout){.x = p->x, .y = p->y, .width = p->width, .height = p->height};
    }
    case NT_IMAGE: {
        const ImageProps *p = &inst->props.image;
        return (InstanceLayout){.x = p->x, .y = p->y, .width = p->width, .height = p->height};
    }
    case NT_STACK:
    case NT_SCROLL: {
        const StackProps *p = &inst->props.stack;
        return (InstanceLayout){.x = p->x, .y = p->y, .width = p->width, .height = p->height};
    }
    case NT_TEXT: {
        const TextProps *p = &inst->props.text;
        const TextLines *lines = text_lines(inst);
        return (InstanceLayout){
            .x = p->x, .y = p->y, .width = lines->width, .height = lines->count * text_props_line_height(p)};
    }
    case NT_RAW_TEXT:
        break;
    }
    return (InstanceLayout){0};
}

// A last-child Scroll with no main-axis size of its own takes what its parent's stated main-axis size leaves.
static bool fills_rest(const ReactInstance *stack, const ReactInstance *child, bool row)
{
    if (child->type != NT_SCROLL || child != stack->children[arrlen(stack->children) - 1]) return false;
    const StackProps *s = &stack->props.stack;
    const StackProps *c = &child->props.stack;
    return row ? s->width > 0 && c->width <= 0 : s->height > 0 && c->height <= 0;
}

// Children are already sized and still at their own x/y, so each child's margin and offset are read from its
// layout before being replaced: the first pass places the main axis, the second the cross axis.
// A Scroll is a start-aligned column, so resizing a filling one does not move its own children.
static void layout_flow(ReactInstance *stack)
{
    const StackProps *s = &stack->props.stack;
    const bool row = s->direction == STACK_ROW;
    const int count = (int)arrlen(stack->children);
    const int inner_main = (row ? s->width : s->height) - s->padding * 2;

    int flow = 0, cross = 0;
    bool any = false;
    for (int i = 0; i < count; i++) {
        if (stack->children[i]->type == NT_RAW_TEXT) continue;
        InstanceLayout *c = &stack->children[i]->layout;
        int *main_pos = row ? &c->x : &c->y;
        const int margin = *main_pos;
        *main_pos = s->padding + flow + margin;
        if (fills_rest(stack, stack->children[i], row)) {
            const int rest = inner_main - flow - margin;
            *(row ? &c->width : &c->height) = rest > 0 ? rest : 0;
        }
        flow += margin + (row ? c->width : c->height) + s->gap;

        const int extent = (row ? c->y + c->height : c->x + c->width);
        if (extent > cross) cross = extent;
        any = true;
    }

    const int content_main = (any ? flow - s->gap : 0) + s->padding * 2;
    const int content_cross = cross + s->padding * 2;
    InstanceLayout *self = &stack->layout;
    if (self->width <= 0) self->width = row ? content_main : content_cross;
    if (self->height <= 0) self->height = row ? content_cross : content_main;
    self->content_height = row ? content_cross : content_main;

    const int inner_cross = (row ? self->height : self->width) - s->padding * 2;
    for (int i = 0; i < count; i++) {
        if (stack->children[i]->type == NT_RAW_TEXT) continue;
        InstanceLayout *c = &stack->children[i]->layout;
        int *cross_pos = row ? &c->y : &c->x;
        const int slack = inner_cross - (row ? c->height : c->width);
        const int aligned = s->align == STACK_ALIGN_CENTER ? slack / 2 : s->align == STACK_ALIGN_END ? slack : 0;
        *cross_pos = s->padding + aligned + *cross_pos;
    }
}

// Each Text child of a Rect sits one of its own line heights lower for every Text child before it.
static void stack_rect_texts(ReactInstance *rect)
{
    int before = 0;
    for (ptrdiff_t i = 0; i < arrlen(rect->children); i++) {
        ReactInstance *child = rect->children[i];
        if (child->type != NT_TEXT) continue;
        child->layout.y += before * text_props_line_height(&child->props.text);
        before++;
    }
}

static void layout_instance(ReactInstance *inst)
{
    inst->layout = own_bounds(inst);
    const int count = (int)arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        layout_instance(inst->children[i]);
    }
    if (inst->type == NT_STACK || inst->type == NT_SCROLL) layout_flow(inst);
    if (inst->type == NT_RECT) stack_rect_texts(inst);
}

void layout_tree(ReactInstance **roots)
{
    const int count = (int)arrlen(roots);
    for (int i = 0; i < count; i++) {
        layout_instance(roots[i]);
    }
}
