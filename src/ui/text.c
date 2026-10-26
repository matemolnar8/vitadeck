#include "text.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "glyph_cache.h"
#include "stb_ds.h"

// raylib's DrawTextEx put 1px between glyphs; keeping it keeps every layout's text widths.
#define TEXT_SPACING 1.0f

static Texture2D g_page_textures[VD_GLYPH_ATLAS_PAGE_MAX];
static bool g_overflow_warned = false;

void text_begin_frame(void)
{
    glyph_cache_begin_frame();
}

void text_reset(void)
{
    for (int i = 0; i < VD_GLYPH_ATLAS_PAGE_MAX; i++) {
        if (IsTextureValid(g_page_textures[i])) UnloadTexture(g_page_textures[i]);
    }
    memset(g_page_textures, 0, sizeof(g_page_textures));
    glyph_cache_reset();
}

// Once per frame, before drawing: vitaGL copies the whole texture when one already drawn this frame changes.
void text_upload_pages(void)
{
    int overflowed = glyph_cache_stats().overflowed;
    if (overflowed > 0 && !g_overflow_warned) {
        TraceLog(LOG_WARNING, "TEXT: This frame needs more glyphs than %d atlas pages hold; %d draw as boxes",
                 VD_GLYPH_ATLAS_PAGE_MAX, overflowed);
        g_overflow_warned = true;
    }
    for (int page = 0; page < VD_GLYPH_ATLAS_PAGE_MAX; page++) {
        const unsigned char *pixels = glyph_cache_take_dirty_page(page);
        if (!pixels) continue;
        if (IsTextureValid(g_page_textures[page])) {
            UpdateTexture(g_page_textures[page], pixels);
            continue;
        }
        Image image = {(void *)pixels, VD_GLYPH_ATLAS_PAGE_SIZE, VD_GLYPH_ATLAS_PAGE_SIZE, 1,
                       PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
        g_page_textures[page] = LoadTextureFromImage(image);
        TraceLog(LOG_DEBUG, "TEXT: Glyph atlas page %d of %d added (%d KiB)", page + 1, VD_GLYPH_ATLAS_PAGE_MAX,
                 VD_GLYPH_ATLAS_PAGE_BYTES / 1024);
    }
}

// Controls, zero width joiners and variation selectors.
static bool draws_nothing(uint32_t codepoint)
{
    return codepoint < 0x20 || codepoint == 0x200d || (codepoint >= 0xfe00 && codepoint <= 0xfe0f);
}

typedef struct {
    int length;
    bool emoji;
    VdGlyph glyph;
    uint32_t codepoint;
} Cluster;

// An emoji, or one codepoint.
static Cluster next_cluster(const VdFont *font, const char *text, int font_size)
{
    Cluster cluster = {0};
    VdEmojiSequence sequence;
    if (font->outline) {
        int length = vd_emoji_cluster(text, &sequence);
        if (length > 0) {
            cluster.length = length;
            cluster.emoji = true;
            cluster.glyph = glyph_cache_get_emoji(font->outline, font_size, &sequence);
            return cluster;
        }
    }
    cluster.codepoint = vd_utf8_next(text, &cluster.length);
    return cluster;
}

static float codepoint_width(const VdFont *font, uint32_t codepoint, int font_size)
{
    if (draws_nothing(codepoint)) return 0.0f;
    if (font->outline) return glyph_cache_get(font->outline, font_size, codepoint).advance + TEXT_SPACING;

    const Font *bitmap = &font->bitmap;
    int index = GetGlyphIndex(*bitmap, (int)codepoint);
    float advance = bitmap->glyphs[index].advanceX != 0
                        ? (float)bitmap->glyphs[index].advanceX
                        : bitmap->recs[index].width + (float)bitmap->glyphs[index].offsetX;
    float scale = bitmap->baseSize > 0 ? (float)font_size / (float)bitmap->baseSize : 1.0f;
    return advance * scale + TEXT_SPACING;
}

float text_cluster_width(const VdFont *font, const char *text, int font_size, int *length)
{
    Cluster cluster = next_cluster(font, text, font_size);
    *length = cluster.length;
    if (cluster.emoji) return cluster.glyph.advance + TEXT_SPACING;
    return codepoint_width(font, cluster.codepoint, font_size);
}

float text_measure(const VdFont *font, const char *text, int font_size)
{
    if (!font->outline) return MeasureTextEx(font->bitmap, text, (float)font_size, TEXT_SPACING).x;

    float width = 0.0f;
    for (const char *p = text; *p != '\0';) {
        int length = 0;
        width += text_cluster_width(font, p, font_size, &length);
        p += length;
    }
    return width > 0.0f ? width - TEXT_SPACING : 0.0f;
}

int text_line_height(const VdFont *font, int font_size)
{
    if (!font->outline) return font_size;
    VdGlyphFontMetrics metrics = glyph_font_metrics(font->outline, font_size);
    int height = (int)roundf(metrics.ascent + metrics.descent + metrics.line_gap);
    return height > 0 ? height : font_size;
}

void text_draw(const VdFont *font, const char *text, Vector2 position, int font_size, Color color)
{
    if (!font->outline) {
        DrawTextEx(font->bitmap, text, position, (float)font_size, TEXT_SPACING, color);
        return;
    }

    float baseline = roundf(position.y + glyph_font_metrics(font->outline, font_size).ascent);
    float pen = position.x;
    for (const char *p = text; *p != '\0';) {
        Cluster cluster = next_cluster(font, p, font_size);
        p += cluster.length;
        if (!cluster.emoji && draws_nothing(cluster.codepoint)) continue;

        VdGlyph glyph = cluster.emoji ? cluster.glyph : glyph_cache_get(font->outline, font_size, cluster.codepoint);
        Vector2 at = {roundf(pen) + (float)glyph.offset_x, baseline + (float)glyph.offset_y};
        if (glyph.page >= 0 && IsTextureValid(g_page_textures[glyph.page])) {
            Rectangle source = {(float)glyph.x, (float)glyph.y, (float)glyph.width, (float)glyph.height};
            Color tint = glyph.color ? (Color){255, 255, 255, color.a} : color;
            DrawTextureRec(g_page_textures[glyph.page], source, at, tint);
        } else if (glyph.page < 0 && glyph.width > 0) {
            DrawRectangleLines((int)at.x, (int)at.y, glyph.width, glyph.height, color);
        }
        pen += glyph.advance + TEXT_SPACING;
    }
}

#define TEXT_SOURCE_MAX 4096

// UI thread only, like every glyph cache read; one Text is built at a time.
static TextLines g_lines;

int text_props_font_size(const TextProps *text)
{
    return text->font_size > 0 ? text->font_size : VD_TEXT_FONT_SIZE_DEFAULT;
}

int text_props_line_height(const TextProps *text)
{
    if (text->line_height > 0) return text->line_height;
    return text_line_height(font_registry_get(text->font_name), text_props_font_size(text)) + 4;
}

static int measure_width(const VdFont *font, const char *text, int font_size)
{
    return (int)(text_measure(font, text, font_size) + 0.5f);
}

static void add_line(TextLines *lines, const char *line, const VdFont *font, int font_size)
{
    if (lines->count >= TEXT_LAYOUT_MAX_LINES) return;

    snprintf(lines->lines[lines->count], TEXT_LAYOUT_MAX_LINE_CHARS, "%s", line);
    int width = measure_width(font, lines->lines[lines->count], font_size);
    lines->widths[lines->count] = width;
    if (width > lines->width) lines->width = width;
    lines->count++;
}

static void wrap_words(const char *paragraph, const VdFont *font, int font_size, int max_width, TextLines *lines)
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

        int word_width = measure_width(font, word, font_size);
        if (word_width > max_width) {
            if (current_len > 0) {
                add_line(lines, current, font, font_size);
                current[0] = '\0';
                current_len = 0;
            }

            // Chunks hold whole clusters, and at least one even when it alone is too wide.
            int chunk_start = 0;
            while (chunk_start < word_len) {
                int length = 0;
                text_cluster_width(font, word + chunk_start, font_size, &length);
                int chunk_end = chunk_start + length;
                while (chunk_end < word_len) {
                    text_cluster_width(font, word + chunk_end, font_size, &length);
                    char probe[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
                    memcpy(probe, word + chunk_start, (size_t)(chunk_end + length - chunk_start));
                    if (measure_width(font, probe, font_size) > max_width) break;
                    chunk_end += length;
                }

                char chunk[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
                int chunk_len = chunk_end - chunk_start;
                memcpy(chunk, word + chunk_start, (size_t)chunk_len);
                add_line(lines, chunk, font, font_size);
                chunk_start = chunk_end;
            }
            continue;
        }

        char candidate[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
        if (current_len == 0) {
            snprintf(candidate, sizeof(candidate), "%s", word);
        } else {
            snprintf(candidate, sizeof(candidate), "%s %s", current, word);
        }

        if (measure_width(font, candidate, font_size) <= max_width) {
            snprintf(current, sizeof(current), "%s", candidate);
            current_len = (int)strlen(current);
        } else {
            if (current_len > 0) add_line(lines, current, font, font_size);
            snprintf(current, sizeof(current), "%s", word);
            current_len = word_len;
        }
    }

    if (current_len > 0 || lines->count == 0) add_line(lines, current, font, font_size);
}

const TextLines *text_lines(const ReactInstance *text)
{
    const TextProps *t = &text->props.text;
    TextLines *lines = &g_lines;
    lines->count = 0;
    lines->width = 0;

    char source[TEXT_SOURCE_MAX] = {0};
    for (ptrdiff_t i = 0; i < arrlen(text->children); i++) {
        const ReactInstance *child = text->children[i];
        if (child && child->type == NT_RAW_TEXT && child->props.raw_text) {
            strncat(source, child->props.raw_text, sizeof(source) - strlen(source) - 1);
        }
    }

    int font_size = text_props_font_size(t);
    const VdFont *font = font_registry_get(t->font_name);
    const bool wrap = t->width > 0 && t->wrap == TEXT_WRAP_WORD;

    const char *start = source;
    for (const char *p = source;; p++) {
        if (*p != '\n' && *p != '\0') continue;
        char line[TEXT_LAYOUT_MAX_LINE_CHARS] = {0};
        int len = (int)(p - start);
        if (len >= TEXT_LAYOUT_MAX_LINE_CHARS) len = TEXT_LAYOUT_MAX_LINE_CHARS - 1;
        memcpy(line, start, (size_t)len);
        if (wrap && len > 0) {
            wrap_words(line, font, font_size, t->width, lines);
        } else {
            add_line(lines, line, font, font_size);
        }
        if (*p == '\0') break;
        start = p + 1;
    }

    if (t->width > lines->width) lines->width = t->width;
    return lines;
}

int text_align_offset(int box_width, int line_width, TextAlign align)
{
    if (box_width <= 0) return 0;
    switch (align) {
    case TEXT_ALIGN_CENTER:
        return (box_width - line_width) / 2;
    case TEXT_ALIGN_RIGHT:
        return box_width - line_width;
    default:
        return 0;
    }
}
