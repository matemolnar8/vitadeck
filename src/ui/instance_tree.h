#ifndef INSTANCE_TREE_H
#define INSTANCE_TREE_H

#include <stdbool.h>
#include <raylib.h>

typedef enum { NT_RECT = 1, NT_TEXT = 2, NT_BUTTON = 3, NT_RAW_TEXT = 4, NT_SCROLL = 5, NT_IMAGE = 6 } NodeType;

typedef struct {
    int x, y, width, height;
    bool has_fill, has_outline;
    Color fill_color, border_color;
    float border_radius;
} RectProps;

typedef enum { TEXT_ALIGN_LEFT = 0, TEXT_ALIGN_CENTER = 1, TEXT_ALIGN_RIGHT = 2 } TextAlign;

typedef enum { TEXT_WRAP_NONE = 0, TEXT_WRAP_WORD = 1 } TextWrap;

typedef struct {
    char *font_name;
    int font_size;
    bool has_color;
    Color color;
    bool border;
    int x;
    int y;
    int width;
    int line_height;
    TextAlign align;
    TextWrap wrap;
} TextProps;

typedef struct {
    int x, y, width, height;
    Color color;
    Color text_color;
    char *label;
    int font_size;
    float border_radius;
} ButtonProps;

typedef struct {
    int x, y, width, height;
    bool has_fill;
    Color fill_color;
    int gap;
    int padding;
} ScrollProps;

typedef struct {
    char *image_name;
    int x, y, width, height;
} ImageProps;

typedef struct ReactInstance ReactInstance;

struct ReactInstance {
    char *id;
    NodeType type;
    union {
        RectProps rect;
        TextProps text;
        ButtonProps button;
        ScrollProps scroll;
        ImageProps image;
        char *raw_text;
    } props;
    ReactInstance **children;
    ReactInstance *parent;
};

// Minimal column layout for scroll containers. Rect, button, and image children
// participate in the flow: each is placed at the current flow position
// (its own y acts as a top margin) and advances the flow by margin +
// height + gap. Returns false for children that are not part of the flow.
bool scroll_flow_step(const ReactInstance *child, int gap, int *flow_y, int *base_y_out);

// Total stacked content height of a scroll container, including padding.
int scroll_content_height(const ReactInstance *scroll);

// Look up a scroll container in the front snapshot (thread-safe).
// Returns false when id is not a scroll container.
bool instance_scroll_metrics(const char *id, int *viewport_height, int *content_height);

// Nearest scroll container containing this instance, or the instance itself if it is a scroll.
// Returns a malloc'd id (caller frees) or NULL.
char *instance_scroll_for_descendant(const char *id);

// Deepest scroll container whose viewport contains (x, y).
// Returns a malloc'd id (caller frees) or NULL.
char *instance_scroll_at(int x, int y);

// Initialize the instance tree (call once at startup before any threads)
void instance_tree_init(void);

// Swap back buffer to front buffer (call from JS thread after mutations)
void instance_tree_swap(void);
void instance_tree_clear(void);

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

// Read-only inspection of the front snapshot (thread-safe, for integration tests and diagnostics).
void instance_tree_collect_text(char *buffer, size_t buffer_size);
bool instance_tree_contains_text(const char *needle);
int instance_tree_count_nodes(NodeType type);

#endif /* INSTANCE_TREE_H */
