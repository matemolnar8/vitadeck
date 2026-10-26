#include <math.h>
#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <rlgl.h>
#include "stb_ds.h"
#include "core/bench.h"
#include "fonts.h"
#include "glyph_cache.h"
#include "images.h"
#include "instance_tree.h"
#include "input.h"
#include "scroll.h"
#include "text.h"

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

#define CORNER_SEGMENTS_MAX 8

// DrawRectangleRounded with the corner directions computed once per segment count: it calls sinf and cosf 24 times a
// corner, which made rounded Buttons the Vita's most expensive draw. It emits the same vertices, so the same pixels.
// Odd segment counts, and the ones raylib picks itself below 4, go to raylib.
static void draw_rounded_rect(Rectangle rec, float roundness, int segments, Color color)
{
    if (roundness <= 0.0f || rec.width < 1 || rec.height < 1 || segments < 4 || segments > CORNER_SEGMENTS_MAX ||
        segments % 2 != 0) {
        DrawRectangleRounded(rec, roundness, segments, color);
        return;
    }
    if (roundness >= 1.0f) roundness = 1.0f;
    float radius = (rec.width > rec.height) ? (rec.height * roundness) / 2 : (rec.width * roundness) / 2;
    if (radius <= 0.0f) return;

    // Per corner, from its start angle in steps of 90 / segments degrees.
    static Vector2 directions[CORNER_SEGMENTS_MAX + 1][4][CORNER_SEGMENTS_MAX + 1];
    static bool ready[CORNER_SEGMENTS_MAX + 1];
    const float angles[4] = {180.0f, 270.0f, 0.0f, 90.0f};
    const float step = 90.0f / (float)segments;
    if (!ready[segments]) {
        for (int k = 0; k < 4; k++) {
            for (int i = 0; i <= segments; i++) {
                float angle = angles[k] + step * (float)i;
                directions[segments][k][i] = (Vector2){cosf(DEG2RAD * angle), sinf(DEG2RAD * angle)};
            }
        }
        ready[segments] = true;
    }

    const float left = rec.x, top = rec.y, right = rec.x + rec.width, bottom = rec.y + rec.height;
    const Vector2 point[12] = {
        {left + radius, top},
        {right - radius, top},
        {right, top + radius},
        {right, bottom - radius},
        {right - radius, bottom},
        {left + radius, bottom},
        {left, bottom - radius},
        {left, top + radius},
        {left + radius, top + radius},
        {right - radius, top + radius},
        {right - radius, bottom - radius},
        {left + radius, bottom - radius},
    };
    const Texture2D texture = GetShapesTexture();
    const Rectangle source = GetShapesTextureRectangle();
    const float u0 = source.x / texture.width, u1 = (source.x + source.width) / texture.width;
    const float v0 = source.y / texture.height, v1 = (source.y + source.height) / texture.height;

    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);
    for (int k = 0; k < 4; k++) {
        const Vector2 center = point[8 + k];
        const Vector2 *d = directions[segments][k];
        // Each quad is two segments.
        for (int i = 0; i < segments; i += 2) {
            rlTexCoord2f(u0, v0);
            rlVertex2f(center.x, center.y);
            rlTexCoord2f(u1, v0);
            rlVertex2f(center.x + d[i + 2].x * radius, center.y + d[i + 2].y * radius);
            rlTexCoord2f(u1, v1);
            rlVertex2f(center.x + d[i + 1].x * radius, center.y + d[i + 1].y * radius);
            rlTexCoord2f(u0, v1);
            rlVertex2f(center.x + d[i].x * radius, center.y + d[i].y * radius);
        }
    }
    // Top, right, bottom, left and middle, as point indices.
    static const int rects[5][4] = {{0, 8, 9, 1}, {2, 9, 10, 3}, {11, 5, 4, 10}, {7, 6, 11, 8}, {8, 11, 10, 9}};
    for (int r = 0; r < 5; r++) {
        rlTexCoord2f(u0, v0);
        rlVertex2f(point[rects[r][0]].x, point[rects[r][0]].y);
        rlTexCoord2f(u0, v1);
        rlVertex2f(point[rects[r][1]].x, point[rects[r][1]].y);
        rlTexCoord2f(u1, v1);
        rlVertex2f(point[rects[r][2]].x, point[rects[r][2]].y);
        rlTexCoord2f(u1, v0);
        rlVertex2f(point[rects[r][3]].x, point[rects[r][3]].y);
    }
    rlEnd();
    rlSetTexture(0);
}

typedef struct {
    int x, y;
} RenderContext;

static void render_instance(ReactInstance *inst, RenderContext ctx);

static int button_font_size(const ButtonProps *b)
{
    return b->font_size > 0 ? b->font_size : VD_BUTTON_FONT_SIZE_DEFAULT;
}

static int text_input_font_size(const TextInputProps *t)
{
    return t->font_size > 0 ? t->font_size : VD_TEXT_INPUT_FONT_SIZE_DEFAULT;
}

static int text_block_height(int line_count, int line_height, const VdFont *font, int font_size)
{
    if (line_count <= 0) return 0;
    return (line_count - 1) * line_height + text_line_height(font, font_size);
}

static void render_children(ReactInstance *inst, int x, int y)
{
    RenderContext child_ctx = {x, y};
    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        render_instance(inst->children[i], child_ctx);
    }
}

static void render_rect_instance(ReactInstance *inst, RenderContext ctx)
{
    RectProps *r = &inst->props.rect;
    int abs_x = ctx.x + inst->layout.x;
    int abs_y = ctx.y + inst->layout.y;
    Rectangle rect = {abs_x, abs_y, inst->layout.width, inst->layout.height};

    if (r->border_radius > 0.0f) {
        float roundness = border_radius_to_roundness(rect, r->border_radius);
        if (r->has_fill) {
            draw_rounded_rect(rect, roundness, 8, r->fill_color);
        }
        if (r->has_outline) {
            DrawRectangleRoundedLinesEx(rect, roundness, 8, 2.0f, r->border_color);
        }
    } else {
        if (r->has_fill) {
            DrawRectangle(abs_x, abs_y, inst->layout.width, inst->layout.height, r->fill_color);
        }
        if (r->has_outline) {
            DrawRectangleLines(abs_x, abs_y, inst->layout.width, inst->layout.height, r->border_color);
        }
    }

    render_children(inst, abs_x, abs_y);
}

static void render_stack_instance(ReactInstance *inst, RenderContext ctx)
{
    const StackProps *s = &inst->props.stack;
    int abs_x = ctx.x + inst->layout.x;
    int abs_y = ctx.y + inst->layout.y;
    if (s->has_fill) {
        DrawRectangle(abs_x, abs_y, inst->layout.width, inst->layout.height, s->fill_color);
    }
    render_children(inst, abs_x, abs_y);
}

static void render_text_instance(ReactInstance *inst, RenderContext ctx)
{
    TextProps *t = &inst->props.text;
    int font_size = text_props_font_size(t);
    const VdFont *font = font_registry_get(t->font_name);
    int line_height = text_props_line_height(t);
    int base_x = ctx.x + inst->layout.x;
    int base_y = ctx.y + inst->layout.y;
    Color color = t->has_color ? t->color : BLACK;

    const TextLines *lines = text_lines(inst);

    if (t->border) {
        int border_padding = 4;
        Rectangle rect = {base_x - border_padding, base_y - border_padding, lines->width + border_padding * 2,
                          text_block_height(lines->count, line_height, font, font_size) + border_padding * 2};
        DrawRectangleLinesEx(rect, 2, color);
    }

    for (int i = 0; i < lines->count; i++) {
        int line_x = base_x + text_align_offset(lines->width, lines->widths[i], t->align);
        int line_y = base_y + i * line_height;
        text_draw(font, lines->lines[i], (Vector2){line_x, line_y}, font_size, color);
    }
}

static void render_button_instance(ReactInstance *inst, RenderContext ctx)
{
    ButtonProps *b = &inst->props.button;
    int abs_x = ctx.x + inst->layout.x;
    int abs_y = ctx.y + inst->layout.y;
    int width = inst->layout.width;
    int height = inst->layout.height;

    bool hovered = input_is_hovered(inst->id);
    bool pressed = input_is_pressed(inst->id);

    Color visual = b->color;
    if (pressed)
        visual = mix_color(visual, BLACK, 0.5f);
    else if (b->has_focus_color && input_is_focused(inst->id))
        visual = b->focus_color;
    else if (hovered)
        visual = mix_color(visual, WHITE, 0.4f);

    if (b->border_radius > 0.0f) {
        Rectangle rect = {abs_x, abs_y, width, height};
        float roundness = border_radius_to_roundness(rect, b->border_radius);
        draw_rounded_rect(rect, roundness, 8, visual);
    } else {
        DrawRectangle(abs_x, abs_y, width, height, visual);
    }

    const int padding = 8;
    const VdFont *font = font_registry_get(b->font_name);
    int font_size = button_font_size(b);
    int label_width = (int)(text_measure(font, b->label, font_size) + 0.5f);
    int label_x = abs_x + padding + text_align_offset(width - padding * 2, label_width, b->align);
    int label_y = abs_y + (height - text_line_height(font, font_size)) / 2;
    text_draw(font, b->label, (Vector2){label_x, label_y}, font_size, b->text_color);
}

// The longest run of whole clusters from the start of text, or with keep_end from its end, that fits max_width
// and out_size. Copies it into out and returns its width.
static float fit_text(const VdFont *font, const char *text, int font_size, float max_width, bool keep_end, char *out,
                      size_t out_size)
{
    const size_t len = strlen(text);
    size_t start = 0, end = 0;
    float width = 0.0f;
    if (keep_end) {
        // Clusters only parse forwards, so drop them from the start until the rest fits.
        end = len;
        for (size_t at = 0; at < len;) {
            int size = 0;
            width += text_cluster_width(font, text + at, font_size, &size);
            at += (size_t)size;
        }
        while (start < len && (width > max_width || len - start >= out_size)) {
            int size = 0;
            width -= text_cluster_width(font, text + start, font_size, &size);
            start += (size_t)size;
        }
        if (start == len) width = 0.0f;
    } else {
        while (end < len) {
            int size = 0;
            float w = text_cluster_width(font, text + end, font_size, &size);
            if (width + w > max_width || end + (size_t)size >= out_size) break;
            width += w;
            end += (size_t)size;
        }
    }
    memcpy(out, text + start, end - start);
    out[end - start] = '\0';
    return width;
}

static void render_text_input_instance(ReactInstance *inst, RenderContext ctx)
{
    TextInputProps *t = &inst->props.text_input;
    Rectangle rect = {ctx.x + inst->layout.x, ctx.y + inst->layout.y, inst->layout.width, inst->layout.height};
    float roundness = border_radius_to_roundness(rect, t->border_radius);

    if (roundness > 0.0f)
        draw_rounded_rect(rect, roundness, 8, t->color);
    else
        DrawRectangleRec(rect, t->color);

    if (input_is_focused(inst->id)) {
        Color ring = t->has_focus_color ? t->focus_color : t->text_color;
        if (roundness > 0.0f)
            DrawRectangleRoundedLinesEx(rect, roundness, 8, 2.0f, ring);
        else
            DrawRectangleLinesEx(rect, 2.0f, ring);
    }

    const char *editing = input_editing_text(inst->id);
    const char *value = editing ? editing : t->value ? t->value : "";
    const bool show_placeholder = !editing && value[0] == '\0';
    const char *text = show_placeholder && t->placeholder ? t->placeholder : value;
    Color color = show_placeholder ? mix_color(t->text_color, t->color, 0.5f) : t->text_color;

    const int padding = 8;
    const int caret_width = 2;
    int font_size = text_input_font_size(t);
    const VdFont *font = font_registry_get(t->font_name);
    int glyph_height = text_line_height(font, font_size);
    float text_x = rect.x + padding;
    float text_y = rect.y + (rect.height - glyph_height) / 2.0f;
    float max_width = rect.width - padding * 2 - (editing ? caret_width + 1 : 0);

    // While editing, the end of the text stays visible so the caret does.
    char visible[1024];
    float width = fit_text(font, text, font_size, max_width, editing != NULL, visible, sizeof(visible));
    text_draw(font, visible, (Vector2){text_x, text_y}, font_size, color);

    if (editing && fmod(GetTime(), 1.0) < 0.5) {
        DrawRectangle((int)(text_x + width + 1), (int)text_y, caret_width, glyph_height, t->text_color);
    }
}

static void render_image_instance(ReactInstance *inst, RenderContext ctx)
{
    ImageProps *image = &inst->props.image;
    Texture2D texture = image_registry_get(image->image_name);
    if (!IsTextureValid(texture)) return;

    Rectangle source = {0.0f, 0.0f, (float)texture.width, (float)texture.height};
    Rectangle dest = {(float)(ctx.x + inst->layout.x), (float)(ctx.y + inst->layout.y), (float)inst->layout.width,
                      (float)inst->layout.height};
    DrawTexturePro(texture, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
}

static void render_scrollbar(Rectangle viewport, int content_height, int offset)
{
    const int track_margin = 4;
    const int bar_width = 4;
    float track_x = viewport.x + viewport.width - bar_width - track_margin;
    float track_y = viewport.y + track_margin;
    float track_height = viewport.height - track_margin * 2;
    if (track_height <= 0 || content_height <= 0) return;

    float thumb_height = track_height * viewport.height / (float)content_height;
    if (thumb_height < 16.0f) thumb_height = 16.0f;
    if (thumb_height > track_height) thumb_height = track_height;

    int max_scroll = content_height - (int)viewport.height;
    float thumb_y = track_y;
    if (max_scroll > 0) {
        thumb_y += (track_height - thumb_height) * ((float)offset / (float)max_scroll);
    }

    draw_rounded_rect((Rectangle){track_x, track_y, bar_width, track_height}, 1.0f, 4, (Color){255, 255, 255, 30});
    draw_rounded_rect((Rectangle){track_x, thumb_y, bar_width, thumb_height}, 1.0f, 4, (Color){255, 255, 255, 120});
}

static void render_scroll_instance(ReactInstance *inst, RenderContext ctx)
{
    const StackProps *s = &inst->props.stack;
    int abs_x = ctx.x + inst->layout.x;
    int abs_y = ctx.y + inst->layout.y;
    int width = inst->layout.width;
    int height = inst->layout.height;
    Rectangle viewport = {abs_x, abs_y, width, height};

    if (s->has_fill) {
        DrawRectangle(abs_x, abs_y, width, height, s->fill_color);
    }

    int content_height = inst->layout.content_height;
    int max_scroll = scroll_max_offset(height, content_height);

    int offset = scroll_get_offset(inst->id);
    if (offset > max_scroll) {
        offset = max_scroll;
        scroll_set_offset(inst->id, offset);
    }

    BeginScissorMode(abs_x, abs_y, width, height);
    render_children(inst, abs_x, abs_y - offset);
    EndScissorMode();

    if (max_scroll > 0) {
        render_scrollbar(viewport, content_height, offset);
    }

    if (input_is_focused(inst->id)) {
        DrawRectangleLinesEx(viewport, 2.0f, (Color){255, 255, 255, 200});
    }
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
    case NT_TEXT_INPUT:
        render_text_input_instance(inst, ctx);
        break;
    case NT_SCROLL:
        render_scroll_instance(inst, ctx);
        break;
    case NT_STACK:
        render_stack_instance(inst, ctx);
        break;
    case NT_IMAGE:
        render_image_instance(inst, ctx);
        break;
    case NT_RAW_TEXT:
        break;
    }
}

// Caches every glyph the frame draws, so the atlas pages upload once before anything draws from them.
static void cache_glyphs(ReactInstance *inst)
{
    if (!inst) return;
    int count = arrlen(inst->children);
    switch (inst->type) {
    case NT_TEXT: {
        const TextProps *t = &inst->props.text;
        for (int i = 0; i < count; i++) {
            ReactInstance *child = inst->children[i];
            if (child && child->type == NT_RAW_TEXT && child->props.raw_text) {
                text_measure(font_registry_get(t->font_name), child->props.raw_text, text_props_font_size(t));
            }
        }
        return;
    }
    case NT_BUTTON: {
        const ButtonProps *b = &inst->props.button;
        if (b->label) text_measure(font_registry_get(b->font_name), b->label, button_font_size(b));
        break;
    }
    case NT_TEXT_INPUT: {
        const TextInputProps *t = &inst->props.text_input;
        const VdFont *font = font_registry_get(t->font_name);
        const char *editing = input_editing_text(inst->id);
        if (editing) text_measure(font, editing, text_input_font_size(t));
        if (t->value) text_measure(font, t->value, text_input_font_size(t));
        if (t->placeholder) text_measure(font, t->placeholder, text_input_font_size(t));
        break;
    }
    default:
        break;
    }
    for (int i = 0; i < count; i++) {
        cache_glyphs(inst->children[i]);
    }
}

void render_draw_list(void)
{
    instance_tree_render_lock();
    ReactInstance **root = instance_get_root_children();
    RenderContext ctx = {0, 0};
    int count = arrlen(root);
    bench_frame_mark(VD_BENCH_FRAME_LAYOUT);

    text_begin_frame();
    for (int i = 0; i < count; i++) {
        cache_glyphs(root[i]);
    }
    text_upload_pages();
    bench_frame_mark(VD_BENCH_FRAME_GLYPHS);

    for (int i = 0; i < count; i++) {
        if (root[i]) {
            render_instance(root[i], ctx);
        }
    }
    instance_tree_render_unlock();
    bench_frame_mark(VD_BENCH_FRAME_DRAW);
}
