#ifndef INSTANCE_TREE_H
#define INSTANCE_TREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <raylib.h>

// Longest instance id including its terminator; native create calls reject longer ids.
#define VD_INSTANCE_ID_MAX 64

typedef enum {
    NT_RECT = 1,
    NT_TEXT = 2,
    NT_BUTTON = 3,
    NT_RAW_TEXT = 4,
    NT_SCROLL = 5,
    NT_IMAGE = 6,
    NT_TEXT_INPUT = 7,
    NT_STACK = 8,
} NodeType;

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
    char *font_name;
    int font_size;
    float border_radius;
    bool has_focus_color;
    Color focus_color;
    TextAlign align;
} ButtonProps;

typedef struct {
    int x, y, width, height;
    Color color;
    Color text_color;
    bool has_focus_color;
    Color focus_color;
    char *font_name;
    int font_size;
    float border_radius;
    int max_length;
    char *value;
    char *placeholder;
} TextInputProps;

typedef enum { STACK_COLUMN = 0, STACK_ROW = 1 } StackDirection;

typedef enum { STACK_ALIGN_START = 0, STACK_ALIGN_CENTER = 1, STACK_ALIGN_END = 2 } StackAlign;

// A Stack, and a Scroll, which is a column Stack that scrolls. A width or height of 0 fits the content.
typedef struct {
    int x, y, width, height;
    bool has_fill;
    Color fill_color;
    StackDirection direction;
    StackAlign align;
    int gap;
    int padding;
} StackProps;

typedef struct {
    char *image_name;
    int x, y, width, height;
} ImageProps;

// Bounds relative to the parent's top-left, computed from props by layout_tree once per swap, on its first read.
// A Scroll's offset is not part of it: the offset changes without a swap, so readers subtract it.
typedef struct {
    int x, y, width, height;
    // Flowed content height of a Stack or Scroll, including padding; a Scroll's scroll range comes from it.
    int content_height;
} InstanceLayout;

typedef struct ReactInstance ReactInstance;

struct ReactInstance {
    char *id;
    NodeType type;
    union {
        RectProps rect;
        TextProps text;
        ButtonProps button;
        TextInputProps text_input;
        StackProps stack;
        ImageProps image;
        char *raw_text;
    } props;
    InstanceLayout layout;
    ReactInstance **children;
    ReactInstance *parent;
};

// Look up a scroll container in the front snapshot (thread-safe).
// Returns false when id is not a scroll container.
bool instance_scroll_metrics(const char *id, int *viewport_height, int *content_height);

// Nearest scroll container containing this instance, or the instance itself if it is a scroll.
// Copies its id into out_id and returns true, or returns false when there is none.
bool instance_scroll_for_descendant(const char *id, char out_id[VD_INSTANCE_ID_MAX]);

// Scrolls every Scroll containing id, innermost first, by the least that shows id's whole layout bounds, clamped
// to the content. UI thread only, like the offsets.
void instance_scroll_into_view(const char *id);

// Deepest scroll container whose viewport contains (x, y).
// Copies its id into out_id and returns true, or returns false when there is none.
bool instance_scroll_at(int x, int y, char out_id[VD_INSTANCE_ID_MAX]);

// Initialize the instance tree (call once at startup before any threads)
void instance_tree_init(void);

// Publish the back buffer as the front snapshot if a mutation marked it dirty since the last swap.
// Call from the JS thread. Returns whether it swapped.
bool instance_tree_swap(void);
void instance_tree_clear(void);

// Render lock/unlock - UI thread must hold lock during entire render
void instance_tree_render_lock(void);
void instance_tree_render_unlock(void);

// Get root children for rendering (must hold render lock)
ReactInstance **instance_get_root_children(void);

// Hit testing (thread-safe, acquires lock internally). Copies the hit id into out_id and returns true on a hit.
bool instance_hit_test(int x, int y, char out_id[VD_INSTANCE_ID_MAX]);
bool instance_exists(const char *id);
// Buttons and Text Inputs take Focus.
bool instance_is_focusable(const char *id);

// What a Text Input edit starts from, as heap copies the caller frees. Returns false when id is not a Text Input.
bool instance_text_input_session(const char *id, char **value, char **placeholder, int *max_length);

// autoFocus: the JS thread asks for Focus on a back-tree instance; the request is published with the next swap.
// The UI thread takes it once, copying the id into out_id. The last request before a swap wins.
void instance_back_request_focus(const char *id);
bool instance_take_focus_request(char out_id[VD_INSTANCE_ID_MAX]);

// Focusable elements for gamepad navigation
typedef struct {
    char *id;
    int x, y, width, height;
} FocusableElement;

FocusableElement *get_focusable_elements(int *count);
// Like get_focusable_elements, but a Scroll's scrolled-out descendants stay candidates only when focused_id is
// inside that Scroll, so the D-pad can reach them from within and hidden rows never compete outside it.
FocusableElement *get_focusable_elements_for(const char *focused_id, int *count);
void free_focusable_elements(FocusableElement *elems, int count);

// Back-buffer operations (called from JS thread only)
void instance_back_mark_dirty(void);
// Native mutation calls so far; each one marks the back buffer dirty.
uint32_t instance_back_mutation_count(void);
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
