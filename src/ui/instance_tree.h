#ifndef INSTANCE_TREE_H
#define INSTANCE_TREE_H

#include <stdbool.h>
#include <raylib.h>

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
    float border_radius;
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
    Color text_color;
    char *label;
    int font_size;
    float border_radius;
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

// Initialize the instance tree (call once at startup before any threads)
void instance_tree_init(void);

// Swap back buffer to front buffer (call from JS thread after mutations)
void instance_tree_swap(void);

// Render lock/unlock - UI thread must hold lock during entire render
void instance_tree_render_lock(void);
void instance_tree_render_unlock(void);

// Get root children for rendering (must hold render lock)
ReactInstance **instance_get_root_children(void);

// Hit testing (thread-safe, acquires lock internally)
const char *instance_hit_test(int x, int y);
bool instance_exists(const char *id);

// Focusable elements for gamepad navigation
typedef struct {
    char *id;
    int x, y, width, height;
} FocusableElement;

FocusableElement *get_focusable_elements(int *count);
void free_focusable_elements(FocusableElement *elems, int count);

// Back-buffer operations (called from JS thread only)
ReactInstance *instance_back_find(const char *id);
void instance_back_put(ReactInstance *inst);
void instance_back_del(const char *id);
void instance_back_free(ReactInstance *inst);
void instance_back_root_append(ReactInstance *child);
void instance_back_root_insert(ReactInstance *child, ReactInstance *before);
void instance_back_root_remove(ReactInstance *child);
void instance_back_root_clear(void);

#endif /* INSTANCE_TREE_H */

