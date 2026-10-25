#include "text.h"

#include <math.h>
#include <string.h>
#include "glyph_cache.h"

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
