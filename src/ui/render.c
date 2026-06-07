#include <math.h>
#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include "stb_ds.h"
#include "fonts.h"
#include "instance_tree.h"
#include "input.h"

#define TEXT_LAYOUT_MAX_LINES 128
#define TEXT_LAYOUT_MAX_LINE_CHARS 512
#define TEXT_SOURCE_MAX 4096

typedef struct {
    char lines[TEXT_LAYOUT_MAX_LINES][TEXT_LAYOUT_MAX_LINE_CHARS];
    int count;
    int block_width;
    int block_height;
} TextLayout;

static Color mix_color(Color c, Color mix_with, float amount)
{
    Color out = {0};
    out.r = (unsigned char)(c.r + (mix_with.r - c.r) * amount);
    out.g = (unsigned char)(c.g + (mix_with.g - c.g) * amount);
    out.b = (unsigned char)(c.b + (mix_with.b - c.b) * amount);
    out.a = c.a;
    return out;
}

static float border_radius_to_roundness(Rectangle rect, float radius_px)
{
    float shorter = (rect.width > rect.height) ? rect.height : rect.width;
    if (shorter <= 0.0f || radius_px <= 0.0f) return 0.0f;
    float clamped = fminf(radius_px, shorter / 2.0f);
    return (2.0f * clamped) / shorter;
}

typedef struct {
    int x, y;
    int text_index;
} RenderContext;

static void render_instance(ReactInstance *inst, RenderContext ctx);

static int measure_text_width(Font font, const char *text, int font_size)
{
    Vector2 size = MeasureTextEx(font, text, (float)font_size, 1.0f);
    return (int)(size.x + 0.5f);
}

static int text_glyph_height(Font font, int font_size)
{
    Vector2 size = MeasureTextEx(font, "Ay", (float)font_size, 1.0f);
    int height = (int)(size.y + 0.5f);
    return height > 0 ? height : font_size;
}

static int text_line_height(const TextProps *t)
{
    int font_size = t->font_size > 0 ? t->font_size : 30;
    if (t->line_height > 0) return t->line_height;
    return text_glyph_height(font_registry_get(t->font_name), font_size) + 4;
}

static int text_block_height(int line_count, int line_height, Font font, int font_size)
{
    if (line_count <= 0) return 0;
    if (line_count == 1) return text_glyph_height(font, font_size);
    return (line_count - 1) * line_height + text_glyph_height(font, font_size);
}

static void text_layout_add_line(TextLayout *layout, const char *line, Font font, int font_size)
{
    if (layout->count >= TEXT_LAYOUT_MAX_LINES) return;

    strncpy(layout->lines[layout->count], line, TEXT_LAYOUT_MAX_LINE_CHARS - 1);
    layout->lines[layout->count][TEXT_LAYOUT_MAX_LINE_CHARS - 1] = '\0';

    int line_width = measure_text_width(font, layout->lines[layout->count], font_size);
    if (line_width > layout->block_width) layout->block_width = line_width;

    layout->count++;
}

static void text_layout_wrap_word(const char *paragraph, Font font, int font_size, int max_width, TextLayout *layout)
{
    const char *cursor = paragraph;
    char current[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
    int current_len = 0;

    while (*cursor != '\0') {
        while (*cursor == ' ')
            cursor++;
        if (*cursor == '\0') break;

        const char *word_start = cursor;
        while (*cursor != '\0' && *cursor != ' ')
            cursor++;
        int word_len = (int)(cursor - word_start);

        char word[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
        if (word_len >= TEXT_LAYOUT_MAX_LINE_CHARS) word_len = TEXT_LAYOUT_MAX_LINE_CHARS - 1;
        memcpy(word, word_start, (size_t)word_len);
        word[word_len] = '\0';

        int word_width = measure_text_width(font, word, font_size);
        if (word_width > max_width) {
            if (current_len > 0) {
                text_layout_add_line(layout, current, font, font_size);
                current[0] = '\0';
                current_len = 0;
            }

            int chunk_start = 0;
            while (chunk_start < word_len) {
                int chunk_end = chunk_start + 1;
                while (chunk_end <= word_len) {
                    char probe[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
                    int probe_len = chunk_end - chunk_start;
                    memcpy(probe, word + chunk_start, (size_t)probe_len);
                    if (measure_text_width(font, probe, font_size) > max_width && chunk_end - chunk_start > 1) {
                        chunk_end--;
                        break;
                    }
                    if (chunk_end == word_len) break;
                    chunk_end++;
                }

                char chunk[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
                int chunk_len = chunk_end - chunk_start;
                memcpy(chunk, word + chunk_start, (size_t)chunk_len);
                text_layout_add_line(layout, chunk, font, font_size);
                chunk_start = chunk_end;
            }
            continue;
        }

        char candidate[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
        if (current_len == 0) {
            strncpy(candidate, word, sizeof(candidate) - 1);
        } else {
            snprintf(candidate, sizeof(candidate), "%s %s", current, word);
        }

        if (measure_text_width(font, candidate, font_size) <= max_width) {
            strncpy(current, candidate, sizeof(current) - 1);
            current_len = (int)strlen(current);
        } else {
            if (current_len > 0) text_layout_add_line(layout, current, font, font_size);
            strncpy(current, word, sizeof(current) - 1);
            current_len = word_len;
        }
    }

    if (current_len > 0 || layout->count == 0) text_layout_add_line(layout, current, font, font_size);
}

static void text_layout_build(const char *text, const TextProps *t, TextLayout *layout)
{
    memset(layout, 0, sizeof(*layout));

    int font_size = t->font_size > 0 ? t->font_size : 30;
    Font font = font_registry_get(t->font_name);
    int line_height = text_line_height(t);
    int max_width = t->width;

    if (max_width <= 0 || t->wrap != TEXT_WRAP_WORD) {
        const char *start = text;
        for (const char *p = text;; p++) {
            if (*p == '\n' || *p == '\0') {
                char line[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
                int len = (int)(p - start);
                if (len >= TEXT_LAYOUT_MAX_LINE_CHARS) len = TEXT_LAYOUT_MAX_LINE_CHARS - 1;
                memcpy(line, start, (size_t)len);
                text_layout_add_line(layout, line, font, font_size);
                if (*p == '\0') break;
                start = p + 1;
            }
        }
    } else {
        const char *start = text;
        for (const char *p = text;; p++) {
            if (*p == '\n' || *p == '\0') {
                char paragraph[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
                int len = (int)(p - start);
                if (len >= TEXT_LAYOUT_MAX_LINE_CHARS) len = TEXT_LAYOUT_MAX_LINE_CHARS - 1;
                memcpy(paragraph, start, (size_t)len);
                if (len == 0) {
                    text_layout_add_line(layout, "", font, font_size);
                } else {
                    text_layout_wrap_word(paragraph, font, font_size, max_width, layout);
                }
                if (*p == '\0') break;
                start = p + 1;
            }
        }
    }

    if (layout->count == 0) text_layout_add_line(layout, "", font, font_size);

    if (t->width > 0 && layout->block_width < t->width) layout->block_width = t->width;
    layout->block_height = text_block_height(layout->count, line_height, font, font_size);
}

static int text_layout_line_x(int base_x, int box_width, int line_width, TextAlign align)
{
    if (box_width <= 0) return base_x;
    switch (align) {
    case TEXT_ALIGN_CENTER:
        return base_x + (box_width - line_width) / 2;
    case TEXT_ALIGN_RIGHT:
        return base_x + (box_width - line_width);
    default:
        return base_x;
    }
}

static void render_rect_instance(ReactInstance *inst, RenderContext ctx)
{
    RectProps *r = &inst->props.rect;
    int abs_x = ctx.x + r->x;
    int abs_y = ctx.y + r->y;
    Rectangle rect = {abs_x, abs_y, r->width, r->height};

    if (r->border_radius > 0.0f) {
        float roundness = border_radius_to_roundness(rect, r->border_radius);
        if (r->has_fill) {
            DrawRectangleRounded(rect, roundness, 8, r->fill_color);
        }
        if (r->has_outline) {
            DrawRectangleRoundedLinesEx(rect, roundness, 8, 2.0f, r->border_color);
        }
    } else {
        if (r->has_fill) {
            DrawRectangle(abs_x, abs_y, r->width, r->height, r->fill_color);
        }
        if (r->has_outline) {
            DrawRectangleLines(abs_x, abs_y, r->width, r->height, r->border_color);
        }
    }

    RenderContext child_ctx = {abs_x, abs_y, 0};
    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        ReactInstance *child = inst->children[i];
        if (!child) continue;
        render_instance(child, child_ctx);
        if (child->type == NT_TEXT) {
            child_ctx.text_index++;
        }
    }
}

static void render_text_instance(ReactInstance *inst, RenderContext ctx)
{
    TextProps *t = &inst->props.text;

    char text_buffer[TEXT_SOURCE_MAX] = {0};
    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        ReactInstance *child = inst->children[i];
        if (child && child->type == NT_RAW_TEXT && child->props.raw_text) {
            strncat(text_buffer, child->props.raw_text, sizeof(text_buffer) - strlen(text_buffer) - 1);
        }
    }

    int font_size = t->font_size > 0 ? t->font_size : 30;
    Font font = font_registry_get(t->font_name);
    int line_height = text_line_height(t);
    int base_x = ctx.x + t->x;
    int base_y = ctx.y + t->y + ctx.text_index * line_height;
    Color color = t->has_color ? t->color : BLACK;

    TextLayout layout;
    text_layout_build(text_buffer, t, &layout);

    int align_box_width = t->width > 0 ? t->width : layout.block_width;

    if (t->border) {
        int border_padding = 4;
        Rectangle rect = {base_x - border_padding, base_y - border_padding, layout.block_width + border_padding * 2,
                          layout.block_height + border_padding * 2};
        DrawRectangleLinesEx(rect, 2, color);
    }

    for (int i = 0; i < layout.count; i++) {
        int line_width = measure_text_width(font, layout.lines[i], font_size);
        int line_x = text_layout_line_x(base_x, align_box_width, line_width, t->align);
        int line_y = base_y + i * line_height;
        DrawTextEx(font, layout.lines[i], (Vector2){line_x, line_y}, (float)font_size, 1.0f, color);
    }
}

static void render_button_instance(ReactInstance *inst, RenderContext ctx)
{
    ButtonProps *b = &inst->props.button;
    int abs_x = ctx.x + b->x;
    int abs_y = ctx.y + b->y;

    bool hovered = input_is_hovered(inst->id);
    bool pressed = input_is_pressed(inst->id);

    Color visual = b->color;
    if (pressed)
        visual = mix_color(visual, BLACK, 0.5f);
    else if (hovered)
        visual = mix_color(visual, WHITE, 0.4f);

    if (b->border_radius > 0.0f) {
        Rectangle rect = {abs_x, abs_y, b->width, b->height};
        float roundness = border_radius_to_roundness(rect, b->border_radius);
        DrawRectangleRounded(rect, roundness, 8, visual);
    } else {
        DrawRectangle(abs_x, abs_y, b->width, b->height, visual);
    }

    const int padding = 8;
    int font_size = b->font_size > 0 ? b->font_size : 20;
    DrawTextEx(font_registry_default(), b->label, (Vector2){abs_x + padding, abs_y + padding}, (float)font_size, 1.0f,
               b->text_color);
}

static void render_instance(ReactInstance *inst, RenderContext ctx)
{
    if (!inst) return;

    switch (inst->type) {
    case NT_RECT:
        render_rect_instance(inst, ctx);
        break;
    case NT_TEXT:
        render_text_instance(inst, ctx);
        break;
    case NT_BUTTON:
        render_button_instance(inst, ctx);
        break;
    case NT_RAW_TEXT:
        break;
    }
}

void render_draw_list(void)
{
    instance_tree_render_lock();
    ReactInstance **root = instance_get_root_children();
    RenderContext ctx = {0, 0, 0};
    int count = arrlen(root);
    for (int i = 0; i < count; i++) {
        if (root[i]) {
            render_instance(root[i], ctx);
        }
    }
    instance_tree_render_unlock();
}
