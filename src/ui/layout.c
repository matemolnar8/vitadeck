#include "layout.h"
#include "stb_ds.h"

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
    case NT_TEXT:
        return (InstanceLayout){.x = inst->props.text.x, .y = inst->props.text.y, .width = inst->props.text.width};
    case NT_RAW_TEXT:
        break;
    }
    return (InstanceLayout){0};
}

static bool flows(const ReactInstance *child)
{
    switch (child->type) {
    case NT_RECT:
    case NT_BUTTON:
    case NT_TEXT_INPUT:
    case NT_IMAGE:
    case NT_STACK:
        return true;
    default:
        return false;
    }
}

// Children are already sized and still at their own x/y, so each child's margin and offset are read from its
// layout before being replaced: the first pass places the main axis, the second the cross axis.
static void layout_flow(ReactInstance *stack)
{
    const StackProps *s = &stack->props.stack;
    const bool row = s->direction == STACK_ROW;
    const int count = (int)arrlen(stack->children);

    int flow = 0, cross = 0;
    bool any = false;
    for (int i = 0; i < count; i++) {
        InstanceLayout *c = &stack->children[i]->layout;
        if (!flows(stack->children[i])) {
            c->x += s->padding;
            c->y += s->padding;
            continue;
        }
        int *main_pos = row ? &c->x : &c->y;
        const int margin = *main_pos;
        *main_pos = s->padding + flow + margin;
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
        if (!flows(stack->children[i])) continue;
        InstanceLayout *c = &stack->children[i]->layout;
        int *cross_pos = row ? &c->y : &c->x;
        const int slack = inner_cross - (row ? c->height : c->width);
        const int aligned = s->align == STACK_ALIGN_CENTER ? slack / 2 : s->align == STACK_ALIGN_END ? slack : 0;
        *cross_pos = s->padding + aligned + *cross_pos;
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
}

void layout_tree(ReactInstance **roots)
{
    const int count = (int)arrlen(roots);
    for (int i = 0; i < count; i++) {
        layout_instance(roots[i]);
    }
}
