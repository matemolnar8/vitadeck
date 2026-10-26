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
// The largest glyph a page has room for next to its white square.
#define GLYPH_SIZE_MAX (VD_GLYPH_ATLAS_PAGE_SIZE - VD_GLYPH_ATLAS_WHITE_SIZE - GLYPH_PADDING)

#define ZWJ 0x200d
#define VS15 0xfe0e
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
static int g_overflowed = 0;
static const VdGlyphFont *g_fallback = NULL;

typedef struct {
    VdLazyFont lazy;
    bool tried;
    const VdGlyphFont *font;
} LazySlot;

static LazySlot g_lazy[VD_LAZY_FONT_MAX];
static int g_lazy_count = 0;

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
    for (int i = 0; i < g_lazy_count; i++) {
        if (g_lazy[i].font == font) g_lazy[i].font = NULL;
    }
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

void glyph_cache_set_lazy_fonts(const VdLazyFont *fonts, int count)
{
    memset(g_lazy, 0, sizeof(g_lazy));
    g_lazy_count = count < VD_LAZY_FONT_MAX ? count : VD_LAZY_FONT_MAX;
    for (int i = 0; i < g_lazy_count; i++) {
        g_lazy[i].lazy = fonts[i];
    }
}

static bool in_range(uint32_t codepoint, uint32_t first, uint32_t last)
{
    return codepoint >= first && codepoint <= last;
}

bool vd_in_hangul_blocks(uint32_t codepoint)
{
    return in_range(codepoint, 0x1100, 0x11ff) || in_range(codepoint, 0x3130, 0x318f) ||
           in_range(codepoint, 0xa960, 0xa97f) || in_range(codepoint, 0xac00, 0xd7ff) ||
           in_range(codepoint, 0xffa0, 0xffdf);
}

// Hangul syllables are left out: Droid Sans Fallback Full has 3 of the 11,172, so Korean text would open it for
// nothing and mix two faces in a word.
bool vd_in_cjk_blocks(uint32_t codepoint)
{
    return in_range(codepoint, 0x1100, 0x11ff) || in_range(codepoint, 0x2e80, 0x9fff) ||
           in_range(codepoint, 0xf900, 0xfaff) || in_range(codepoint, 0xfe30, 0xfe4f) ||
           in_range(codepoint, 0xff00, 0xffef) || in_range(codepoint, 0x20000, 0x323af);
}

// The font's glyph index for codepoint, setting source to the font when it has one; 0 when it has none.
static int find_glyph(const VdGlyphFont *font, uint32_t codepoint, const VdGlyphFont **source)
{
    int index = font ? stbtt_FindGlyphIndex(&font->info, (int)codepoint) : 0;
    if (index != 0) *source = font;
    return index;
}

// The glyph index from the first font in the chain that has codepoint, setting source to it; 0 when none has it.
static int resolve_glyph(const VdGlyphFont *font, uint32_t codepoint, const VdGlyphFont **source)
{
    int index = find_glyph(font, codepoint, source);
    if (index == 0) index = find_glyph(g_fallback, codepoint, source);
    for (int i = 0; index == 0 && i < g_lazy_count; i++) {
        LazySlot *slot = &g_lazy[i];
        if (!slot->lazy.covers(codepoint)) continue;
        if (!slot->tried) {
            slot->font = slot->lazy.load();
            slot->tried = true;
        }
        index = find_glyph(slot->font, codepoint, source);
    }
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

// Paints the white square into empty pixels; glyphs start after it.
static void start_page(AtlasPage *page)
{
    for (int row = 0; row < VD_GLYPH_ATLAS_WHITE_SIZE; row++) {
        memset(page->pixels + (size_t)row * VD_GLYPH_ATLAS_PAGE_SIZE * 4, 255, VD_GLYPH_ATLAS_WHITE_SIZE * 4);
    }
    page->dirty = true;
    page->cursor_x = VD_GLYPH_ATLAS_WHITE_SIZE + GLYPH_PADDING;
    page->cursor_y = 0;
    page->row_height = VD_GLYPH_ATLAS_WHITE_SIZE;
}

static void clear_page(int index)
{
    AtlasPage *page = &g_pages[index];
    memset(page->pixels, 0, VD_GLYPH_ATLAS_PAGE_BYTES);
    start_page(page);
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
    if (width > GLYPH_SIZE_MAX || height > GLYPH_SIZE_MAX) return -1;
    if (g_current_page >= 0 && page_place(&g_pages[g_current_page], width, height, x, y)) return g_current_page;

    int index;
    if (g_page_count < VD_GLYPH_ATLAS_PAGE_MAX) {
        unsigned char *pixels = calloc(1, VD_GLYPH_ATLAS_PAGE_BYTES);
        if (!pixels) return -1;
        index = g_page_count++;
        g_pages[index] = (AtlasPage){.pixels = pixels};
        start_page(&g_pages[index]);
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

// A fallback glyph takes the requested font's em size: scaling each font to the Font Size by its own line height
// would draw a font with taller line metrics, like Noto Sans KR, smaller than the text around it.
static float glyph_scale(const VdGlyphFont *font, const VdGlyphFont *source, int size)
{
    float scale = stbtt_ScaleForPixelHeight(&font->info, (float)size);
    if (source == font) return scale;
    float em = scale / stbtt_ScaleForMappingEmToPixels(&font->info, 1.0f);
    return stbtt_ScaleForMappingEmToPixels(&source->info, em);
}

// Returns false when the glyph has pixels but no room, so the caller retries on a later frame.
static bool rasterise(const VdGlyphFont *font, int glyph_index, float scale, VdGlyph *out)
{
    const stbtt_fontinfo *info = &font->info;
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
    if (!coverage) {
        *out = (VdGlyph){-1, 0, 0, width, height, x0, y0, out->advance, false};
        g_overflowed++;
        return false;
    }
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

static bool find_emoji(const VdGlyphFont *font, int size, const VdEmojiSequence *sequence, VdGlyph *glyph);

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
    int glyph_index = resolve_glyph(font, codepoint, &source);

    VdGlyph glyph;
    VdEmojiSequence alone = {{codepoint}, 1};
    if (glyph_index == 0 && find_emoji(font, size, &alone, &glyph)) return glyph;
    if (rasterise(source, glyph_index, glyph_scale(font, source, size), &glyph)) {
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
    if (page < 0) {
        out->width = out->height = size;
        out->color = false;
        g_overflowed++;
        return false;
    }

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
        if (size > GLYPH_SIZE_MAX) {
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

VdGlyph glyph_cache_get_emoji(const VdGlyphFont *font, int size, const VdEmojiSequence *sequence)
{
    VdGlyph glyph;
    if (find_emoji(font, size, sequence, &glyph)) return glyph;
    uint32_t first = sequence->codepoints[0];
    const VdGlyphFont *source = NULL;
    return glyph_cache_get(font, size, resolve_glyph(font, first, &source) != 0 ? first : VD_EMOJI_FALLBACK);
}

void glyph_cache_begin_frame(void)
{
    g_frame++;
    g_overflowed = 0;
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
    g_overflowed = 0;
}

const unsigned char *glyph_cache_take_dirty_page(int page)
{
    if (page < 0 || page >= g_page_count || !g_pages[page].dirty) return NULL;
    g_pages[page].dirty = false;
    return g_pages[page].pixels;
}

VdGlyphCacheStats glyph_cache_stats(void)
{
    return (VdGlyphCacheStats){g_page_count, (int)hmlen(g_glyphs), g_rasterised, g_overflowed};
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

// Emoji_Presentation from Unicode 17's emoji-data.txt, sorted: these draw as emoji without U+FE0F.
static const uint32_t EMOJI_PRESENTATION[][2] = {
    {0x231a, 0x231b},   {0x23e9, 0x23ec},   {0x23f0, 0x23f0},   {0x23f3, 0x23f3},   {0x25fd, 0x25fe},
    {0x2614, 0x2615},   {0x2648, 0x2653},   {0x267f, 0x267f},   {0x2693, 0x2693},   {0x26a1, 0x26a1},
    {0x26aa, 0x26ab},   {0x26bd, 0x26be},   {0x26c4, 0x26c5},   {0x26ce, 0x26ce},   {0x26d4, 0x26d4},
    {0x26ea, 0x26ea},   {0x26f2, 0x26f3},   {0x26f5, 0x26f5},   {0x26fa, 0x26fa},   {0x26fd, 0x26fd},
    {0x2705, 0x2705},   {0x270a, 0x270b},   {0x2728, 0x2728},   {0x274c, 0x274c},   {0x274e, 0x274e},
    {0x2753, 0x2755},   {0x2757, 0x2757},   {0x2795, 0x2797},   {0x27b0, 0x27b0},   {0x27bf, 0x27bf},
    {0x2b1b, 0x2b1c},   {0x2b50, 0x2b50},   {0x2b55, 0x2b55},   {0x1f004, 0x1f004}, {0x1f0cf, 0x1f0cf},
    {0x1f18e, 0x1f18e}, {0x1f191, 0x1f19a}, {0x1f1e6, 0x1f1ff}, {0x1f201, 0x1f201}, {0x1f21a, 0x1f21a},
    {0x1f22f, 0x1f22f}, {0x1f232, 0x1f236}, {0x1f238, 0x1f23a}, {0x1f250, 0x1f251}, {0x1f300, 0x1f320},
    {0x1f32d, 0x1f335}, {0x1f337, 0x1f37c}, {0x1f37e, 0x1f393}, {0x1f3a0, 0x1f3ca}, {0x1f3cf, 0x1f3d3},
    {0x1f3e0, 0x1f3f0}, {0x1f3f4, 0x1f3f4}, {0x1f3f8, 0x1f43e}, {0x1f440, 0x1f440}, {0x1f442, 0x1f4fc},
    {0x1f4ff, 0x1f53d}, {0x1f54b, 0x1f54e}, {0x1f550, 0x1f567}, {0x1f57a, 0x1f57a}, {0x1f595, 0x1f596},
    {0x1f5a4, 0x1f5a4}, {0x1f5fb, 0x1f64f}, {0x1f680, 0x1f6c5}, {0x1f6cc, 0x1f6cc}, {0x1f6d0, 0x1f6d2},
    {0x1f6d5, 0x1f6d8}, {0x1f6dc, 0x1f6df}, {0x1f6eb, 0x1f6ec}, {0x1f6f4, 0x1f6fc}, {0x1f7e0, 0x1f7eb},
    {0x1f7f0, 0x1f7f0}, {0x1f90c, 0x1f93a}, {0x1f93c, 0x1f945}, {0x1f947, 0x1f9ff}, {0x1fa70, 0x1fa7c},
    {0x1fa80, 0x1fa8a}, {0x1fa8e, 0x1fac6}, {0x1fac8, 0x1fac8}, {0x1facd, 0x1fadc}, {0x1fadf, 0x1faea},
    {0x1faef, 0x1faf8},
};

static bool emoji_by_default(uint32_t codepoint)
{
    for (size_t i = 0; i < sizeof(EMOJI_PRESENTATION) / sizeof(EMOJI_PRESENTATION[0]); i++) {
        if (codepoint < EMOJI_PRESENTATION[i][0]) return false;
        if (codepoint <= EMOJI_PRESENTATION[i][1]) return true;
    }
    return false;
}

static bool is_skin_tone(uint32_t codepoint)
{
    return codepoint >= 0x1f3fb && codepoint <= 0x1f3ff;
}

static bool is_regional_indicator(uint32_t codepoint)
{
    return codepoint >= 0x1f1e6 && codepoint <= 0x1f1ff;
}

static bool joins_emoji(uint32_t codepoint)
{
    bool tag = codepoint >= 0xe0020 && codepoint <= 0xe007f;
    return is_skin_tone(codepoint) || tag || codepoint == KEYCAP;
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
    bool qualified = emoji_by_default(first) || next == VS16 || next == KEYCAP || is_skin_tone(next);
    if (next == VS15 || !qualified) return 0;

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
