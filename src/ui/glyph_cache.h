#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H

#include <stdbool.h>
#include <stdint.h>

// Glyphs rasterise on first use at the exact pixel size into RGBA8 atlas pages: white, with coverage in alpha.
// When every page is full, the least recently used one is cleared; one used this frame is never cleared.
#define VD_GLYPH_ATLAS_PAGE_SIZE 1024
#define VD_GLYPH_ATLAS_PAGE_BYTES (VD_GLYPH_ATLAS_PAGE_SIZE * VD_GLYPH_ATLAS_PAGE_SIZE * 4)
#define VD_GLYPH_ATLAS_PAGE_MAX 4

#define VD_UTF8_REPLACEMENT 0xfffd

typedef struct VdGlyphFont VdGlyphFont;

typedef struct {
    float ascent;
    // Below the baseline, positive.
    float descent;
    float line_gap;
} VdGlyphFontMetrics;

typedef struct {
    // -1 when the glyph has no pixels, like a space, or found no room this frame.
    int page;
    int x, y, width, height;
    // From the pen position on the baseline to the bitmap's top-left corner.
    int offset_x, offset_y;
    float advance;
} VdGlyph;

typedef struct {
    int pages;
    int glyphs;
    int rasterised;
} VdGlyphCacheStats;

// TrueType or OpenType; NULL when the file is missing or not a font.
VdGlyphFont *glyph_font_load(const char *path);
// Call glyph_cache_reset first: cached glyphs are keyed by font.
void glyph_font_free(VdGlyphFont *font);
VdGlyphFontMetrics glyph_font_metrics(const VdGlyphFont *font, int size);

// Codepoints a font lacks come from the fallback; ones neither has draw the requested font's missing-glyph box.
void glyph_cache_set_fallback(const VdGlyphFont *font);
VdGlyph glyph_cache_get(const VdGlyphFont *font, int size, uint32_t codepoint);
void glyph_cache_begin_frame(void);
// Drops every glyph and page.
void glyph_cache_reset(void);
// The page's pixels if they changed since the last call, else NULL.
const unsigned char *glyph_cache_take_dirty_page(int page);
VdGlyphCacheStats glyph_cache_stats(void);

// Decodes the codepoint at text, which must not be at its terminator, and sets length to the bytes it used.
// Malformed UTF-8 decodes one byte at a time as VD_UTF8_REPLACEMENT.
uint32_t vd_utf8_next(const char *text, int *length);

#endif /* GLYPH_CACHE_H */
