#include "text.h"

#include <math.h>
#include <string.h>
#include "glyph_cache.h"

// raylib's DrawTextEx put 1px between glyphs; keeping it keeps every layout's text widths.
#define TEXT_SPACING 1.0f

static Texture2D g_page_textures[VD_GLYPH_ATLAS_PAGE_MAX];

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

float text_codepoint_width(const VdFont *font, uint32_t codepoint, int font_size)
{
    if (codepoint < 0x20) return 0.0f;
    if (font->outline) return glyph_cache_get(font->outline, font_size, codepoint).advance + TEXT_SPACING;

    const Font *bitmap = &font->bitmap;
    int index = GetGlyphIndex(*bitmap, (int)codepoint);
    float advance = bitmap->glyphs[index].advanceX != 0
                        ? (float)bitmap->glyphs[index].advanceX
                        : bitmap->recs[index].width + (float)bitmap->glyphs[index].offsetX;
    float scale = bitmap->baseSize > 0 ? (float)font_size / (float)bitmap->baseSize : 1.0f;
    return advance * scale + TEXT_SPACING;
}

float text_measure(const VdFont *font, const char *text, int font_size)
{
    if (!font->outline) return MeasureTextEx(font->bitmap, text, (float)font_size, TEXT_SPACING).x;

    float width = 0.0f;
    for (const char *p = text; *p != '\0';) {
        int length = 0;
        width += text_codepoint_width(font, vd_utf8_next(p, &length), font_size);
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
        int length = 0;
        uint32_t codepoint = vd_utf8_next(p, &length);
        p += length;
        if (codepoint < 0x20) continue;

        VdGlyph glyph = glyph_cache_get(font->outline, font_size, codepoint);
        if (glyph.page >= 0 && IsTextureValid(g_page_textures[glyph.page])) {
            Rectangle source = {(float)glyph.x, (float)glyph.y, (float)glyph.width, (float)glyph.height};
            Vector2 at = {roundf(pen) + (float)glyph.offset_x, baseline + (float)glyph.offset_y};
            DrawTextureRec(g_page_textures[glyph.page], source, at, color);
        }
        pen += glyph.advance + TEXT_SPACING;
    }
}
