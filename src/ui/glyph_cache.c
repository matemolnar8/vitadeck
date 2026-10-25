#include "glyph_cache.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "core/util.h"
#include "stb_ds.h"

// raylib compiles its own static copy; this one is ours, so rasterising does not depend on raylib internals.
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define GLYPH_PADDING 1

#define ZWJ 0x200d
#define VS16 0xfe0f
#define KEYCAP 0x20e3

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

// Emoji sit at the font's ascent, so the key holds the font.
typedef struct {
    const VdGlyphFont *font;
    int size;
    VdEmojiSequence sequence;
} EmojiKey;

// A glyph that is not color marks a sequence with no image.
typedef struct {
    EmojiKey key;
    VdGlyph value;
} EmojiEntry;

// Shelf packing: glyphs fill a row left to right, and a row is as tall as its tallest glyph.
typedef struct {
    unsigned char *pixels;
    bool dirty;
    unsigned last_used;
    int cursor_x, cursor_y, row_height;
} AtlasPage;

static GlyphEntry *g_glyphs = NULL;
static EmojiEntry *g_emoji = NULL;
static VdEmojiLoader g_emoji_loader = NULL;
static AtlasPage g_pages[VD_GLYPH_ATLAS_PAGE_MAX];
static int g_page_count = 0;
static int g_current_page = -1;
static unsigned g_frame = 1;
static int g_rasterised = 0;
static const VdGlyphFont *g_fallback = NULL;
static VdGlyphFontLoader g_fallback_loader = NULL;
static const VdGlyphFont *g_last_fallback = NULL;

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
    if (g_last_fallback == font) g_last_fallback = NULL;
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

void glyph_cache_set_fallback_loader(VdGlyphFontLoader loader)
{
    g_fallback_loader = loader;
    g_last_fallback = NULL;
}

static const VdGlyphFont *last_fallback(void)
{
    if (g_fallback_loader) {
        g_last_fallback = g_fallback_loader();
        g_fallback_loader = NULL;
    }
    return g_last_fallback;
}

// The font's glyph index for codepoint, setting source to the font when it has one; 0 when it has none.
static int find_glyph(const VdGlyphFont *font, uint32_t codepoint, const VdGlyphFont **source)
{
    int index = font ? stbtt_FindGlyphIndex(&font->info, (int)codepoint) : 0;
    if (index != 0) *source = font;
    return index;
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
    for (ptrdiff_t i = hmlen(g_emoji) - 1; i >= 0; i--) {
        if (g_emoji[i].value.page != index) continue;
        EmojiKey key = g_emoji[i].key;
        hmdel(g_emoji, key);
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
    *out = (VdGlyph){page, x, y, width, height, x0, y0, out->advance, false};
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
    int glyph_index = find_glyph(font, codepoint, &source);
    if (glyph_index == 0) glyph_index = find_glyph(g_fallback, codepoint, &source);
    if (glyph_index == 0) glyph_index = find_glyph(last_fallback(), codepoint, &source);

    VdGlyph glyph;
    if (rasterise(source, glyph_index, size, &glyph)) {
        hmput(g_glyphs, key, glyph);
        g_rasterised++;
    }
    return glyph;
}

void glyph_cache_set_emoji_loader(VdEmojiLoader loader)
{
    g_emoji_loader = loader;
}

// Area-averages the image into a size x size square, weighting colour by alpha so edges do not darken.
static bool rasterise_emoji(const VdGlyphFont *font, int size, const unsigned char *image, int image_width,
                            int image_height, VdGlyph *out)
{
    float ascent = glyph_font_metrics(font, size).ascent;
    *out = (VdGlyph){.page = -1, .offset_y = -(int)roundf(ascent), .advance = (float)size, .color = true};
    int x = 0, y = 0;
    int page = atlas_allocate(size, size, &x, &y);
    if (page < 0) return false;

    unsigned char *pixels = g_pages[page].pixels;
    for (int row = 0; row < size; row++) {
        int y0 = row * image_height / size;
        int y1 = (row + 1) * image_height / size;
        if (y1 <= y0) y1 = y0 + 1;
        for (int col = 0; col < size; col++) {
            int x0 = col * image_width / size;
            int x1 = (col + 1) * image_width / size;
            if (x1 <= x0) x1 = x0 + 1;
            unsigned long r = 0, g = 0, b = 0, a = 0;
            for (int sy = y0; sy < y1; sy++) {
                for (int sx = x0; sx < x1; sx++) {
                    const unsigned char *source = image + ((size_t)sy * (size_t)image_width + (size_t)sx) * 4;
                    r += (unsigned long)source[0] * source[3];
                    g += (unsigned long)source[1] * source[3];
                    b += (unsigned long)source[2] * source[3];
                    a += source[3];
                }
            }
            unsigned char *pixel = pixels + ((size_t)(y + row) * VD_GLYPH_ATLAS_PAGE_SIZE + (size_t)(x + col)) * 4;
            unsigned long count = (unsigned long)(x1 - x0) * (unsigned long)(y1 - y0);
            pixel[0] = a ? (unsigned char)(r / a) : 0;
            pixel[1] = a ? (unsigned char)(g / a) : 0;
            pixel[2] = a ? (unsigned char)(b / a) : 0;
            pixel[3] = (unsigned char)(a / count);
        }
    }

    g_pages[page].dirty = true;
    g_pages[page].last_used = g_frame;
    out->page = page;
    out->x = x;
    out->y = y;
    out->width = out->height = size;
    return true;
}

static bool find_emoji(const VdGlyphFont *font, int size, const VdEmojiSequence *sequence, VdGlyph *glyph)
{
    for (int length = sequence->length; length > 0; length--) {
        if (sequence->codepoints[length - 1] == ZWJ) continue;
        EmojiKey key;
        memset(&key, 0, sizeof(key));
        key.font = font;
        key.size = size;
        key.sequence.length = length;
        memcpy(key.sequence.codepoints, sequence->codepoints, (size_t)length * sizeof(uint32_t));

        ptrdiff_t found = hmgeti(g_emoji, key);
        if (found >= 0) {
            if (!g_emoji[found].value.color) continue;
            *glyph = g_emoji[found].value;
            if (glyph->page >= 0) g_pages[glyph->page].last_used = g_frame;
            return true;
        }

        int width = 0, height = 0;
        unsigned char *image = g_emoji_loader ? g_emoji_loader(&key.sequence, &width, &height) : NULL;
        if (!image || width <= 0 || height <= 0) {
            free(image);
            hmput(g_emoji, key, ((VdGlyph){.page = -1}));
            continue;
        }
        bool placed = true;
        if (size > VD_GLYPH_ATLAS_PAGE_SIZE) {
            // It keeps its advance and draws nothing, rather than loading again every frame.
            *glyph = (VdGlyph){.page = -1, .advance = (float)size, .color = true};
        } else {
            placed = rasterise_emoji(font, size, image, width, height, glyph);
        }
        free(image);
        if (placed) {
            hmput(g_emoji, key, *glyph);
            g_rasterised++;
        }
        return true;
    }
    return false;
}

// Asks the same fonts, in the same order, as glyph_cache_get.
static bool has_glyph(const VdGlyphFont *font, uint32_t codepoint)
{
    const VdGlyphFont *source = NULL;
    return find_glyph(font, codepoint, &source) != 0 || find_glyph(g_fallback, codepoint, &source) != 0 ||
           find_glyph(last_fallback(), codepoint, &source) != 0;
}

VdGlyph glyph_cache_get_emoji(const VdGlyphFont *font, int size, const VdEmojiSequence *sequence)
{
    VdGlyph glyph;
    if (find_emoji(font, size, sequence, &glyph)) return glyph;
    uint32_t first = sequence->codepoints[0];
    return glyph_cache_get(font, size, has_glyph(font, first) ? first : VD_EMOJI_FALLBACK);
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
    hmfree(g_emoji);
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

static bool is_regional_indicator(uint32_t codepoint)
{
    return codepoint >= 0x1f1e6 && codepoint <= 0x1f1ff;
}

static bool joins_emoji(uint32_t codepoint)
{
    bool skin_tone = codepoint >= 0x1f3fb && codepoint <= 0x1f3ff;
    bool tag = codepoint >= 0xe0020 && codepoint <= 0xe007f;
    return skin_tone || tag || codepoint == KEYCAP;
}

static void append(VdEmojiSequence *sequence, uint32_t codepoint)
{
    sequence->codepoints[sequence->length++] = codepoint;
}

int vd_emoji_cluster(const char *text, VdEmojiSequence *sequence)
{
    int bytes = 0;
    uint32_t first = vd_utf8_next(text, &bytes);
    int next_length = 0;
    uint32_t next = text[bytes] != '\0' ? vd_utf8_next(text + bytes, &next_length) : 0;
    if (first < 0x1f000 && next != VS16 && next != KEYCAP) return 0;

    memset(sequence, 0, sizeof(*sequence));
    append(sequence, first);
    while (text[bytes] != '\0') {
        int length = 0;
        uint32_t codepoint = vd_utf8_next(text + bytes, &length);
        if (codepoint == VS16) {
            bytes += length;
            continue;
        }
        if (codepoint == ZWJ) {
            if (text[bytes + length] == '\0' || sequence->length + 2 > VD_EMOJI_SEQUENCE_MAX) break;
            int joined_length = 0;
            uint32_t joined = vd_utf8_next(text + bytes + length, &joined_length);
            append(sequence, ZWJ);
            append(sequence, joined);
            bytes += length + joined_length;
            continue;
        }
        bool flag = sequence->length == 1 && is_regional_indicator(first) && is_regional_indicator(codepoint);
        if ((!flag && !joins_emoji(codepoint)) || sequence->length == VD_EMOJI_SEQUENCE_MAX) break;
        append(sequence, codepoint);
        bytes += length;
    }
    return bytes;
}
