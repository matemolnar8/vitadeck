#include "glyph_cache.h"

#include <stdlib.h>
#include <string.h>
#include "core/util.h"
#include "stb_ds.h"

// raylib compiles its own static copy; this one is ours, so rasterising does not depend on raylib internals.
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define GLYPH_PADDING 1

struct VdGlyphFont {
    char *data;
    stbtt_fontinfo info;
};

typedef struct {
    const VdGlyphFont *font;
    int size;
    uint32_t codepoint;
} GlyphKey;

typedef struct {
    GlyphKey key;
    VdGlyph value;
} GlyphEntry;

// Shelf packing: glyphs fill a row left to right, and a row is as tall as its tallest glyph.
typedef struct {
    unsigned char *pixels;
    bool dirty;
    unsigned last_used;
    int cursor_x, cursor_y, row_height;
} AtlasPage;

static GlyphEntry *g_glyphs = NULL;
static AtlasPage g_pages[VD_GLYPH_ATLAS_PAGE_MAX];
static int g_page_count = 0;
static int g_current_page = -1;
static unsigned g_frame = 1;
static int g_rasterised = 0;
static const VdGlyphFont *g_fallback = NULL;

static uint32_t read_u32(const unsigned char *bytes)
{
    return (uint32_t)bytes[0] << 24 | (uint32_t)bytes[1] << 16 | (uint32_t)bytes[2] << 8 | bytes[3];
}

// stb_truetype trusts the file, so reject one whose table directory points outside it. Tables are not checked.
static bool tables_in_bounds(const unsigned char *data, size_t length)
{
    if (!data || length < 12) return false;
    size_t count = (size_t)data[4] << 8 | data[5];
    if (12 + count * 16 > length) return false;
    for (size_t i = 0; i < count; i++) {
        const unsigned char *record = data + 12 + i * 16;
        size_t offset = read_u32(record + 8);
        size_t size = read_u32(record + 12);
        if (offset > length || size > length - offset) return false;
    }
    return true;
}

VdGlyphFont *glyph_font_load(const char *path)
{
    VdGlyphFont *font = calloc(1, sizeof(*font));
    if (!font) return NULL;
    size_t length = 0;
    font->data = vd_read_file(path, &length);
    const unsigned char *data = (const unsigned char *)font->data;
    if (!tables_in_bounds(data, length) || !stbtt_InitFont(&font->info, data, 0)) {
        free(font->data);
        free(font);
        return NULL;
    }
    return font;
}

void glyph_font_free(VdGlyphFont *font)
{
    if (!font) return;
    if (g_fallback == font) g_fallback = NULL;
    free(font->data);
    free(font);
}

VdGlyphFontMetrics glyph_font_metrics(const VdGlyphFont *font, int size)
{
    float scale = stbtt_ScaleForPixelHeight(&font->info, (float)size);
    int ascent = 0, descent = 0, line_gap = 0;
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &line_gap);
    return (VdGlyphFontMetrics){ascent * scale, -descent * scale, line_gap * scale};
}

void glyph_cache_set_fallback(const VdGlyphFont *font)
{
    g_fallback = font;
}

static bool page_place(AtlasPage *page, int width, int height, int *x, int *y)
{
    if (page->cursor_x + width > VD_GLYPH_ATLAS_PAGE_SIZE) {
        page->cursor_x = 0;
        page->cursor_y += page->row_height + GLYPH_PADDING;
        page->row_height = 0;
    }
    if (page->cursor_y + height > VD_GLYPH_ATLAS_PAGE_SIZE) return false;
    *x = page->cursor_x;
    *y = page->cursor_y;
    page->cursor_x += width + GLYPH_PADDING;
    if (height > page->row_height) page->row_height = height;
    return true;
}

static void clear_page(int index)
{
    AtlasPage *page = &g_pages[index];
    memset(page->pixels, 0, VD_GLYPH_ATLAS_PAGE_BYTES);
    page->dirty = true;
    page->cursor_x = page->cursor_y = page->row_height = 0;
    // hmdel moves the last entry into the deleted slot, which a backwards walk has already visited.
    for (ptrdiff_t i = hmlen(g_glyphs) - 1; i >= 0; i--) {
        if (g_glyphs[i].value.page != index) continue;
        GlyphKey key = g_glyphs[i].key;
        hmdel(g_glyphs, key);
    }
}

// The page that now holds a width x height rect at (x, y), or -1 when no page can take it this frame.
static int atlas_allocate(int width, int height, int *x, int *y)
{
    if (width > VD_GLYPH_ATLAS_PAGE_SIZE || height > VD_GLYPH_ATLAS_PAGE_SIZE) return -1;
    if (g_current_page >= 0 && page_place(&g_pages[g_current_page], width, height, x, y)) return g_current_page;

    int index;
    if (g_page_count < VD_GLYPH_ATLAS_PAGE_MAX) {
        unsigned char *pixels = calloc(1, VD_GLYPH_ATLAS_PAGE_BYTES);
        if (!pixels) return -1;
        index = g_page_count++;
        g_pages[index] = (AtlasPage){.pixels = pixels};
    } else {
        index = 0;
        for (int i = 1; i < g_page_count; i++) {
            if (g_pages[i].last_used < g_pages[index].last_used) index = i;
        }
        // Text already drawn from it this frame would show the new glyphs.
        if (g_pages[index].last_used == g_frame) return -1;
        clear_page(index);
    }
    g_current_page = index;
    return page_place(&g_pages[index], width, height, x, y) ? index : -1;
}

// Returns false when the glyph has pixels but no room, so the caller retries on a later frame.
static bool rasterise(const VdGlyphFont *font, int glyph_index, int size, VdGlyph *out)
{
    const stbtt_fontinfo *info = &font->info;
    float scale = stbtt_ScaleForPixelHeight(info, (float)size);
    int advance = 0, left_bearing = 0;
    stbtt_GetGlyphHMetrics(info, glyph_index, &advance, &left_bearing);
    *out = (VdGlyph){.page = -1, .advance = advance * scale};

    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    stbtt_GetGlyphBitmapBox(info, glyph_index, scale, scale, &x0, &y0, &x1, &y1);
    int width = x1 - x0;
    int height = y1 - y0;
    if (width <= 0 || height <= 0) return true;

    int x = 0, y = 0;
    int page = atlas_allocate(width, height, &x, &y);
    unsigned char *coverage = page >= 0 ? malloc((size_t)width * (size_t)height) : NULL;
    if (!coverage) return false;
    stbtt_MakeGlyphBitmap(info, coverage, width, height, width, scale, scale, glyph_index);

    unsigned char *pixels = g_pages[page].pixels;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            unsigned char *pixel = pixels + ((size_t)(y + row) * VD_GLYPH_ATLAS_PAGE_SIZE + (size_t)(x + col)) * 4;
            pixel[0] = pixel[1] = pixel[2] = 255;
            pixel[3] = coverage[row * width + col];
        }
    }
    free(coverage);

    g_pages[page].dirty = true;
    g_pages[page].last_used = g_frame;
    *out = (VdGlyph){page, x, y, width, height, x0, y0, out->advance};
    return true;
}

VdGlyph glyph_cache_get(const VdGlyphFont *font, int size, uint32_t codepoint)
{
    GlyphKey key;
    memset(&key, 0, sizeof(key));
    key.font = font;
    key.size = size;
    key.codepoint = codepoint;

    ptrdiff_t found = hmgeti(g_glyphs, key);
    if (found >= 0) {
        VdGlyph glyph = g_glyphs[found].value;
        if (glyph.page >= 0) g_pages[glyph.page].last_used = g_frame;
        return glyph;
    }

    const VdGlyphFont *source = font;
    int glyph_index = stbtt_FindGlyphIndex(&font->info, (int)codepoint);
    if (glyph_index == 0 && g_fallback && g_fallback != font) {
        int fallback_index = stbtt_FindGlyphIndex(&g_fallback->info, (int)codepoint);
        if (fallback_index != 0) {
            source = g_fallback;
            glyph_index = fallback_index;
        }
    }

    VdGlyph glyph;
    if (rasterise(source, glyph_index, size, &glyph)) {
        hmput(g_glyphs, key, glyph);
        g_rasterised++;
    }
    return glyph;
}

void glyph_cache_begin_frame(void)
{
    g_frame++;
}

void glyph_cache_reset(void)
{
    for (int i = 0; i < g_page_count; i++) {
        free(g_pages[i].pixels);
    }
    memset(g_pages, 0, sizeof(g_pages));
    g_page_count = 0;
    g_current_page = -1;
    hmfree(g_glyphs);
    g_rasterised = 0;
}

const unsigned char *glyph_cache_take_dirty_page(int page)
{
    if (page < 0 || page >= g_page_count || !g_pages[page].dirty) return NULL;
    g_pages[page].dirty = false;
    return g_pages[page].pixels;
}

VdGlyphCacheStats glyph_cache_stats(void)
{
    return (VdGlyphCacheStats){g_page_count, (int)hmlen(g_glyphs), g_rasterised};
}

uint32_t vd_utf8_next(const char *text, int *length)
{
    const unsigned char *bytes = (const unsigned char *)text;
    *length = 1;
    if (bytes[0] < 0x80) return bytes[0];

    int count;
    uint32_t codepoint, min;
    if ((bytes[0] & 0xe0) == 0xc0) {
        count = 2;
        codepoint = bytes[0] & 0x1f;
        min = 0x80;
    } else if ((bytes[0] & 0xf0) == 0xe0) {
        count = 3;
        codepoint = bytes[0] & 0x0f;
        min = 0x800;
    } else if ((bytes[0] & 0xf8) == 0xf0) {
        count = 4;
        codepoint = bytes[0] & 0x07;
        min = 0x10000;
    } else {
        return VD_UTF8_REPLACEMENT;
    }

    // A terminator is not a continuation byte, so a truncated sequence stops here.
    for (int i = 1; i < count; i++) {
        if ((bytes[i] & 0xc0) != 0x80) return VD_UTF8_REPLACEMENT;
        codepoint = (codepoint << 6) | (bytes[i] & 0x3f);
    }
    if (codepoint < min || codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff)) {
        return VD_UTF8_REPLACEMENT;
    }
    *length = count;
    return codepoint;
}
