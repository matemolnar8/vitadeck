#ifndef VD_TESTS_INSTANCE_FIXTURE_H
#define VD_TESTS_INSTANCE_FIXTURE_H

// Builds back-tree instances the way src/jslib/instance.c does, without QuickJS.
// The including harness defines STB_DS_IMPLEMENTATION.

#include <stdlib.h>
#include <string.h>
#include "stb_ds.h"
#include "ui/instance_tree.h"

static inline ReactInstance *fixture_new(ReactInstance *parent, NodeType type, const char *id)
{
    ReactInstance *inst = calloc(1, sizeof(*inst));
    inst->id = strdup(id);
    inst->type = type;
    instance_back_put(inst);
    if (parent) {
        inst->parent = parent;
        arrput(parent->children, inst);
    } else {
        instance_back_root_append(inst);
    }
    return inst;
}

static inline ReactInstance *fixture_rect(ReactInstance *parent, const char *id, int x, int y, int width, int height)
{
    ReactInstance *inst = fixture_new(parent, NT_RECT, id);
    inst->props.rect = (RectProps){.x = x, .y = y, .width = width, .height = height};
    return inst;
}

static inline ReactInstance *fixture_button(ReactInstance *parent, const char *id, int x, int y, int width, int height)
{
    ReactInstance *inst = fixture_new(parent, NT_BUTTON, id);
    inst->props.button = (ButtonProps){.x = x, .y = y, .width = width, .height = height};
    return inst;
}

static inline ReactInstance *fixture_scroll(ReactInstance *parent, const char *id, int x, int y, int width, int height,
                                            int gap, int padding)
{
    ReactInstance *inst = fixture_new(parent, NT_SCROLL, id);
    inst->props.scroll =
        (ScrollProps){.x = x, .y = y, .width = width, .height = height, .gap = gap, .padding = padding};
    return inst;
}

// Detach from parent (or root) and free, like removeChild + destroyInstance.
static inline void fixture_destroy(ReactInstance *inst)
{
    ReactInstance *parent = inst->parent;
    if (parent) {
        for (ptrdiff_t i = 0; i < arrlen(parent->children); i++) {
            if (parent->children[i] == inst) {
                arrdel(parent->children, i);
                break;
            }
        }
    } else {
        instance_back_root_remove(inst);
    }
    instance_back_del(inst->id);
    instance_back_free(inst);
}

static inline void fixture_publish(void)
{
    instance_back_mark_dirty();
    instance_tree_swap();
}

#endif /* VD_TESTS_INSTANCE_FIXTURE_H */
