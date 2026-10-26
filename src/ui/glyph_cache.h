#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H

#include <stdbool.h>
#include <stdint.h>

// Glyphs rasterise on first use at the exact pixel size into RGBA8 atlas pages: white, with coverage in alpha.
// Colour emoji share the pages, resampled to a Font Size square.
// When every page is full, the least recently used one is cleared; one used this frame is never cleared, so a glyph
// that finds no room waits for a later frame and draws as a missing-glyph box until then.
// Every page starts with an opaque white square at (0, 0), so shapes can draw from the same texture as text.
#define VD_GLYPH_ATLAS_PAGE_SIZE 1024
#define VD_GLYPH_ATLAS_PAGE_BYTES (VD_GLYPH_ATLAS_PAGE_SIZE * VD_GLYPH_ATLAS_PAGE_SIZE * 4)
#define VD_GLYPH_ATLAS_PAGE_MAX 4
#define VD_GLYPH_ATLAS_WHITE_SIZE 4

#define VD_UTF8_REPLACEMENT 0xfffd
#define VD_EMOJI_FALLBACK VD_UTF8_REPLACEMENT

// The longest Twemoji sequence, a couple kissing with skin tones, has 9 codepoints once U+FE0F is left out.
#define VD_EMOJI_SEQUENCE_MAX 10

typedef struct VdGlyphFont VdGlyphFont;

typedef struct {
    float ascent;
    // Below the baseline, positive.
    float descent;
    float line_gap;
} VdGlyphFontMetrics;

typedef struct {
    // -1 when the glyph has no pixels, like a space, or found no room this frame; then width and height, if set, are
    // the box to draw in its place.
    int page;
    int x, y, width, height;
    // From the pen position on the baseline to the bitmap's top-left corner.
    int offset_x, offset_y;
    float advance;
    // Drawn untinted: a colour emoji.
    bool color;
} VdGlyph;

typedef struct {
    uint32_t codepoints[VD_EMOJI_SEQUENCE_MAX];
    int length;
} VdEmojiSequence;

// NULL when the font cannot be opened.
typedef VdGlyphFont *(*VdGlyphFontLoader)(void);

// A fallback font opened on the first codepoint in its blocks that every earlier font lacks, so a stray codepoint
// outside them does not load it. A font that fails to open is not tried again.
typedef struct {
    VdGlyphFontLoader load;
    bool (*covers)(uint32_t codepoint);
} VdLazyFont;

#define VD_LAZY_FONT_MAX 2

// The RGBA8 image for a codepoint sequence, U+FE0F left out, in memory from malloc; NULL when there is none.
typedef unsigned char *(*VdEmojiLoader)(const VdEmojiSequence *sequence, int *width, int *height);

typedef struct {
    int pages;
    int glyphs;
    int rasterised;
    // Lookups this frame that found no room in the atlas.
    int overflowed;
} VdGlyphCacheStats;

// TrueType or OpenType; NULL when the file is missing or not a font.
VdGlyphFont *glyph_font_load(const char *path);
// Call glyph_cache_reset first: cached glyphs are keyed by font.
void glyph_font_free(VdGlyphFont *font);
VdGlyphFontMetrics glyph_font_metrics(const VdGlyphFont *font, int size);

// Codepoints a font lacks come from the fallback, then from the lazy fonts in order; ones none has draw their emoji
// image, like a text-style symbol such as U+2764, else the requested font's missing-glyph box.
void glyph_cache_set_fallback(const VdGlyphFont *font);
// At most VD_LAZY_FONT_MAX; none are open yet. The caller owns the fonts they load.
void glyph_cache_set_lazy_fonts(const VdLazyFont *fonts, int count);
// CJK ideographs, kana, bopomofo, CJK symbols, fullwidth forms and Hangul jamo; not Hangul syllables.
bool vd_in_cjk_blocks(uint32_t codepoint);
// Hangul syllables and jamo.
bool vd_in_hangul_blocks(uint32_t codepoint);
VdGlyph glyph_cache_get(const VdGlyphFont *font, int size, uint32_t codepoint);
void glyph_cache_set_emoji_loader(VdEmojiLoader loader);
// The emoji for the longest prefix of sequence that has an image, as a Font Size square on the font's line, so a ZWJ
// sequence without its own image draws as its first emoji. With no image, as when assets/emoji is missing, the first
// codepoint's glyph from the font or a fallback, else VD_EMOJI_FALLBACK's. Missing images are not loaded again.
VdGlyph glyph_cache_get_emoji(const VdGlyphFont *font, int size, const VdEmojiSequence *sequence);
void glyph_cache_begin_frame(void);
// Drops every glyph and page.
void glyph_cache_reset(void);
// The page's pixels if they changed since the last call, else NULL.
const unsigned char *glyph_cache_take_dirty_page(int page);
VdGlyphCacheStats glyph_cache_stats(void);

// Decodes the codepoint at text, which must not be at its terminator, and sets length to the bytes it used.
// Malformed UTF-8 decodes one byte at a time as VD_UTF8_REPLACEMENT.
uint32_t vd_utf8_next(const char *text, int *length);
// The bytes of the emoji cluster at text, with its codepoints in sequence, or 0 when text does not start one. A
// cluster starts at a codepoint from U+1F000 up, an emoji-style symbol like U+2728, or one followed by U+FE0F or a
// keycap mark; U+FE0E after it keeps it text. It takes skin tones, tags, a second regional indicator and ZWJ-joined
// codepoints.
int vd_emoji_cluster(const char *text, VdEmojiSequence *sequence);

#endif /* GLYPH_CACHE_H */
