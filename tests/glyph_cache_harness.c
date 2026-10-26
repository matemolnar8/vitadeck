#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "ui/glyph_cache.h"

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DejaVuSans-ascii.ttf is assets/fonts/DejaVuSans.ttf subset to U+0020-007E, under the same licence.
#define FULL_FONT_PATH VD_SOURCE_DIR "/assets/fonts/DejaVuSans.ttf"
#define ASCII_FONT_PATH VD_SOURCE_DIR "/tests/fixtures/DejaVuSans-ascii.ttf"
#define FALLBACK_FONT_PATH VD_SOURCE_DIR "/assets/fonts/DroidSansFallbackFull.ttf"
#define KOREAN_FONT_PATH VD_SOURCE_DIR "/assets/fonts/NotoSansKR-Regular.otf"

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "glyph_cache_harness: %s\n", message);
    failures++;
}

static void expect_utf8(const char *text, uint32_t codepoint, int length, const char *message)
{
    int got_length = 0;
    uint32_t got = vd_utf8_next(text, &got_length);
    if (got == codepoint && got_length == length) return;
    fprintf(stderr, "glyph_cache_harness: decoded U+%04X in %d bytes, want U+%04X in %d\n", (unsigned)got, got_length,
            (unsigned)codepoint, length);
    expect(false, message);
}

static bool same_glyph(VdGlyph a, VdGlyph b)
{
    return a.page == b.page && a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height &&
           a.offset_x == b.offset_x && a.offset_y == b.offset_y && a.advance == b.advance;
}

static bool same_shape(VdGlyph a, VdGlyph b)
{
    return a.width == b.width && a.height == b.height && a.offset_x == b.offset_x && a.offset_y == b.offset_y &&
           a.advance == b.advance;
}

static void test_utf8(void)
{
    expect_utf8("A", 'A', 1, "ASCII decodes as itself");
    expect_utf8("\xc5\x91", 0x151, 2, "two-byte sequence decodes");
    expect_utf8("\xe2\x82\xac", 0x20ac, 3, "three-byte sequence decodes");
    expect_utf8("\xf0\x9f\x91\x8d", 0x1f44d, 4, "four-byte sequence decodes");
    expect_utf8("\x80", VD_UTF8_REPLACEMENT, 1, "a lone continuation byte is replaced");
    expect_utf8("\xc5", VD_UTF8_REPLACEMENT, 1, "a sequence cut by the terminator is replaced");
    expect_utf8("\xe2\x82(", VD_UTF8_REPLACEMENT, 1, "a sequence cut by ASCII is replaced");
    expect_utf8("\xc0\xaf", VD_UTF8_REPLACEMENT, 1, "an overlong sequence is replaced");
    expect_utf8("\xed\xa0\x80", VD_UTF8_REPLACEMENT, 1, "a surrogate is replaced");
    expect_utf8("\xf4\x90\x80\x80", VD_UTF8_REPLACEMENT, 1, "a codepoint past U+10FFFF is replaced");
    expect_utf8("\xff", VD_UTF8_REPLACEMENT, 1, "an invalid lead byte is replaced");
}

static void test_hits_and_sizes(const VdGlyphFont *font)
{
    glyph_cache_reset();

    VdGlyph a = glyph_cache_get(font, 30, 'A');
    expect(a.page == 0 && a.width > 0 && a.height > 0 && a.advance > 0.0f, "a miss rasterises into the first page");
    expect(a.offset_y < 0, "a capital sits above the baseline");
    expect(glyph_cache_stats().rasterised == 1, "the first lookup rasterises");

    VdGlyph again = glyph_cache_get(font, 30, 'A');
    expect(same_glyph(a, again), "a hit returns the cached glyph");
    expect(glyph_cache_stats().rasterised == 1, "a hit does not rasterise");

    VdGlyph large = glyph_cache_get(font, 60, 'A');
    expect(glyph_cache_stats().rasterised == 2, "another size is another glyph");
    expect(!same_glyph(a, large), "sizes do not share a glyph");
    float ratio = (float)large.height / (float)a.height;
    expect(ratio > 1.8f && ratio < 2.2f, "a glyph at twice the size is about twice as tall");

    VdGlyph space = glyph_cache_get(font, 30, ' ');
    expect(space.page == -1 && space.advance > 0.0f, "a space advances without pixels");

    const unsigned char *pixels = glyph_cache_take_dirty_page(0);
    expect(pixels != NULL, "rasterising marks the page dirty");
    expect(glyph_cache_take_dirty_page(0) == NULL, "taking a dirty page clears it");
    bool covered = false;
    for (int row = 0; pixels && row < a.height; row++) {
        for (int col = 0; col < a.width; col++) {
            const unsigned char *pixel =
                pixels + ((size_t)(a.y + row) * VD_GLYPH_ATLAS_PAGE_SIZE + (size_t)(a.x + col)) * 4;
            if (pixel[3] > 0) {
                covered = true;
                expect(pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 255, "glyph pixels are white");
            }
        }
    }
    expect(covered, "the glyph's rect holds coverage");

    VdGlyphFontMetrics metrics = glyph_font_metrics(font, 30);
    expect(fabsf(metrics.ascent + metrics.descent - 30.0f) < 0.01f, "ascent plus descent is the Font Size");
}

static void test_fallback(const VdGlyphFont *full, const VdGlyphFont *ascii)
{
    glyph_cache_reset();
    const uint32_t accented = 0x151;   // ő
    const uint32_t ideograph = 0x65e5; // 日, in neither font
    const uint32_t private_use = 0xe000;

    glyph_cache_set_fallback(NULL);
    VdGlyph notdef = glyph_cache_get(ascii, 30, private_use);
    VdGlyph no_fallback = glyph_cache_get(ascii, 30, accented);
    expect(same_shape(no_fallback, notdef), "without a fallback a missing glyph is the font's missing-glyph box");

    glyph_cache_set_fallback(full);
    VdGlyph from_full = glyph_cache_get(full, 30, accented);
    VdGlyph fallback = glyph_cache_get(ascii, 30, 0x153); // œ
    VdGlyph full_oe = glyph_cache_get(full, 30, 0x153);
    expect(same_shape(fallback, full_oe), "a glyph the font lacks comes from the fallback");
    expect(!same_shape(fallback, notdef), "a fallback glyph is not the missing-glyph box");
    expect(from_full.advance > 0.0f, "the fallback has the accented glyph");

    VdGlyph own = glyph_cache_get(ascii, 30, 'g');
    VdGlyph full_g = glyph_cache_get(full, 30, 'g');
    expect(same_shape(own, full_g), "a glyph the font has comes from the font");

    VdGlyph neither = glyph_cache_get(ascii, 30, ideograph);
    expect(same_shape(neither, notdef), "a glyph neither font has is the requested font's missing-glyph box");
    VdGlyph self = glyph_cache_get(full, 30, ideograph);
    expect(self.width > 0, "the fallback font's own missing glyph still draws a box");
}

static VdEmojiSequence cluster_of(const char *text)
{
    VdEmojiSequence sequence = {0};
    vd_emoji_cluster(text, &sequence);
    return sequence;
}

static int missing_loads = 0;

static unsigned char *missing_emoji_loader(const VdEmojiSequence *sequence, int *width, int *height)
{
    (void)sequence;
    (void)width;
    (void)height;
    missing_loads++;
    return NULL;
}

static int fallback_loads = 0;
static VdGlyphFont *fallback_font = NULL;

static VdGlyphFont *counting_fallback_loader(void)
{
    fallback_loads++;
    fallback_font = glyph_font_load(FALLBACK_FONT_PATH);
    return fallback_font;
}

static VdGlyphFont *failing_fallback_loader(void)
{
    fallback_loads++;
    return NULL;
}

static int korean_loads = 0;
static VdGlyphFont *korean_font = NULL;

static VdGlyphFont *counting_korean_loader(void)
{
    korean_loads++;
    korean_font = glyph_font_load(KOREAN_FONT_PATH);
    return korean_font;
}

static void set_lazy_font(VdGlyphFontLoader load)
{
    const VdLazyFont lazy = {load, vd_in_cjk_blocks};
    glyph_cache_set_lazy_fonts(&lazy, load ? 1 : 0);
}

static void test_blocks(void)
{
    expect(vd_in_cjk_blocks(0x65e5) && vd_in_cjk_blocks(0x3042) && vd_in_cjk_blocks(0x3001) &&
               vd_in_cjk_blocks(0xff01) && vd_in_cjk_blocks(0x20000),
           "ideographs, kana, CJK punctuation and fullwidth forms are in the CJK blocks");
    expect(!vd_in_cjk_blocks(0xd55c), "Hangul syllables are not in the CJK blocks");
    expect(vd_in_hangul_blocks(0xd55c) && vd_in_hangul_blocks(0x3131) && vd_in_hangul_blocks(0x1100),
           "syllables and jamo are in the Hangul blocks");
    expect(!vd_in_hangul_blocks(0x65e5), "ideographs are not in the Hangul blocks");
    expect(!vd_in_cjk_blocks('A') && !vd_in_cjk_blocks(0x151) && !vd_in_cjk_blocks(0xe000) &&
               !vd_in_hangul_blocks(0xe000) && !vd_in_cjk_blocks(0x1f3ae),
           "Latin, private use and emoji codepoints are in neither");
}

static void test_fallback_chain(const VdGlyphFont *full, const VdGlyphFont *ascii)
{
    const uint32_t ideograph = 0x65e5; // 日
    const uint32_t kana = 0x3042;      // あ
    const uint32_t private_use = 0xe000;

    glyph_cache_reset();
    glyph_cache_set_fallback(NULL);
    set_lazy_font(NULL);
    VdGlyph ascii_notdef = glyph_cache_get(ascii, 30, private_use);
    VdGlyph full_notdef = glyph_cache_get(full, 30, private_use);
    glyph_cache_reset();

    fallback_loads = 0;
    glyph_cache_set_fallback(full);
    set_lazy_font(counting_fallback_loader);
    VdGlyph own = glyph_cache_get(ascii, 30, 'g');
    VdGlyph latin = glyph_cache_get(ascii, 30, 0x153); // œ
    expect(own.width > 0 && latin.width > 0 && !same_shape(latin, ascii_notdef),
           "Latin text resolves from the requested font and the Runtime Default Font");
    expect(fallback_loads == 0, "Latin text does not open the Runtime Fallback Font");
    VdGlyph stray = glyph_cache_get(ascii, 30, private_use);
    expect(same_shape(stray, ascii_notdef) && fallback_loads == 0,
           "a character outside the CJK blocks is the missing-glyph box without opening the Runtime Fallback Font");

    VdGlyph from_default = glyph_cache_get(full, 30, ideograph);
    expect(fallback_loads == 1 && fallback_font != NULL,
           "the first character both fonts lack opens the Runtime Fallback Font");
    VdGlyph from_deck_font = glyph_cache_get(ascii, 30, kana);
    expect(fallback_loads == 1, "the Runtime Fallback Font opens once");
    if (fallback_font) {
        expect(from_default.width > 0 && !same_shape(from_default, full_notdef),
               "an ideograph the Runtime Default Font lacks comes from the Runtime Fallback Font");
        expect(same_shape(from_deck_font, glyph_cache_get(full, 30, kana)) && !same_shape(from_deck_font, ascii_notdef),
               "kana a Deck App Font lacks comes from the Runtime Fallback Font");
        float em = glyph_cache_get(full, 30, 0x2014).advance;
        expect(fabsf(from_default.advance - em) < 0.01f,
               "a fallback ideograph, one em wide in its font, is one em of the requested font wide");
    }

    VdGlyph unknown = glyph_cache_get(ascii, 30, private_use);
    expect(same_shape(unknown, ascii_notdef), "a character no font has is the requested font's missing-glyph box");
    expect(fallback_loads == 1, "the Runtime Fallback Font is not opened again");

    if (fallback_font) {
        glyph_cache_set_emoji_loader(missing_emoji_loader);
        VdEmojiSequence ideograph_emoji = cluster_of("\xe6\x97\xa5\xef\xb8\x8f"); // 日 U+FE0F
        VdGlyph emoji = glyph_cache_get_emoji(ascii, 30, &ideograph_emoji);
        expect(same_shape(emoji, glyph_cache_get(ascii, 30, ideograph)) && !same_shape(emoji, ascii_notdef),
               "an emoji without an image draws its first character from the Runtime Fallback Font");
        glyph_cache_set_emoji_loader(NULL);
    }

    glyph_cache_reset();
    set_lazy_font(NULL);
    glyph_font_free(fallback_font);
    fallback_font = NULL;
}

static void test_korean(const VdGlyphFont *full, const VdGlyphFont *ascii)
{
    const uint32_t han = 0xd55c; // 한
    const uint32_t gul = 0xae00; // 글
    const uint32_t private_use = 0xe000;

    glyph_cache_reset();
    glyph_cache_set_fallback(NULL);
    set_lazy_font(NULL);
    VdGlyph ascii_notdef = glyph_cache_get(ascii, 30, private_use);
    glyph_cache_reset();

    fallback_loads = korean_loads = 0;
    glyph_cache_set_fallback(full);
    const VdLazyFont lazy[] = {{counting_fallback_loader, vd_in_cjk_blocks}, {counting_korean_loader, vd_in_hangul_blocks}};
    glyph_cache_set_lazy_fonts(lazy, 2);

    glyph_cache_get(ascii, 30, private_use);
    glyph_cache_get(ascii, 30, 0x10fffd);
    expect(fallback_loads == 0 && korean_loads == 0, "a character outside every fallback's blocks opens none");

    VdGlyph from_deck_font = glyph_cache_get(ascii, 30, han);
    VdGlyph from_default = glyph_cache_get(full, 30, gul);
    expect(korean_loads == 1, "the first Hangul syllable opens the Korean font once");
    expect(fallback_loads == 0, "Hangul syllables do not open the CJK font");
    if (korean_font) {
        expect(from_deck_font.width > 0 && !same_shape(from_deck_font, ascii_notdef),
               "Hangul a Deck App Font lacks comes from the Korean font");
        // Noto Sans KR's syllables are 0.92 em wide.
        float em = glyph_cache_get(full, 30, 0x2014).advance;
        expect(from_default.width > 0 && fabsf(from_default.advance - 0.92f * em) < 0.01f,
               "Hangul the Runtime Default Font lacks comes from the Korean font at the requested font's em size");
    }

    glyph_cache_get(ascii, 30, 0x65e5);
    expect(fallback_loads == 1 && korean_loads == 1, "an ideograph opens the CJK font, not the Korean font again");

    glyph_cache_reset();
    glyph_cache_set_lazy_fonts(NULL, 0);
    glyph_font_free(fallback_font);
    glyph_font_free(korean_font);
    fallback_font = korean_font = NULL;
}

static void test_failing_fallback_loader(const VdGlyphFont *font)
{
    glyph_cache_reset();
    fallback_loads = 0;
    set_lazy_font(failing_fallback_loader);
    glyph_cache_get(font, 30, 0x65e5);
    glyph_cache_get(font, 30, 0x3042);
    expect(fallback_loads == 1, "a Runtime Fallback Font that fails to open is not retried");

    glyph_cache_reset();
    set_lazy_font(NULL);
}

static void test_hungarian(const VdGlyphFont *full)
{
    glyph_cache_reset();
    glyph_cache_set_fallback(NULL);
    VdGlyph notdef = glyph_cache_get(full, 22, 0x65e5);
    const uint32_t letters[] = {0xc1, 0xe1, 0xc9, 0xe9, 0xcd, 0xed, 0xd3, 0xf3, 0xd6, 0xf6,
                                0x150, 0x151, 0xda, 0xfa, 0xdc, 0xfc, 0x170, 0x171};
    for (size_t i = 0; i < sizeof(letters) / sizeof(letters[0]); i++) {
        VdGlyph glyph = glyph_cache_get(full, 22, letters[i]);
        if (glyph.width > 0 && !same_shape(glyph, notdef)) continue;
        fprintf(stderr, "glyph_cache_harness: U+%04X\n", (unsigned)letters[i]);
        expect(false, "every Hungarian letter, ő and ű included, is a real glyph in the Runtime Default Font");
    }
}

static void expect_cluster(const char *text, int bytes, const uint32_t *codepoints, int count, const char *message)
{
    VdEmojiSequence sequence = {0};
    int got = vd_emoji_cluster(text, &sequence);
    bool same = got == bytes && (bytes == 0 || sequence.length == count);
    for (int i = 0; same && bytes > 0 && i < count; i++) {
        same = sequence.codepoints[i] == codepoints[i];
    }
    if (same) return;
    fprintf(stderr, "glyph_cache_harness: cluster of %d bytes and %d codepoints, want %d and %d\n", got,
            sequence.length, bytes, count);
    expect(false, message);
}

static void test_emoji_clusters(void)
{
    expect_cluster("A", 0, NULL, 0, "a letter is not an emoji");
    expect_cluster("\xe2\x9d\xa4", 0, NULL, 0, "a text-style symbol without U+FE0F is not an emoji");
    expect_cluster("1", 0, NULL, 0, "a digit is not an emoji");
    expect_cluster("\xf0\x9f\x8e\xae!", 4, (const uint32_t[]){0x1f3ae}, 1, "a single emoji is one codepoint");
    expect_cluster("\xe2\x9d\xa4\xef\xb8\x8f", 6, (const uint32_t[]){0x2764}, 1,
                   "U+FE0F makes a symbol an emoji and is left out of the sequence");
    expect_cluster("\xf0\x9f\x91\x8d\xf0\x9f\x8f\xbd", 8, (const uint32_t[]){0x1f44d, 0x1f3fd}, 2,
                   "a skin tone joins its emoji");
    expect_cluster("\xf0\x9f\x87\xad\xf0\x9f\x87\xba\xf0\x9f\x87\xad\xf0\x9f\x87\xba", 8,
                   (const uint32_t[]){0x1f1ed, 0x1f1fa}, 2, "a flag is two regional indicators");
    expect_cluster("1\xef\xb8\x8f\xe2\x83\xa3", 7, (const uint32_t[]){0x31, 0x20e3}, 2, "a keycap keeps its mark");
    expect_cluster("\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x91\xa7 x", 18,
                   (const uint32_t[]){0x1f468, 0x200d, 0x1f469, 0x200d, 0x1f467}, 5, "a ZWJ sequence is one cluster");
    expect_cluster("\xf0\x9f\x91\xa8\xe2\x80\x8d", 4, (const uint32_t[]){0x1f468}, 1,
                   "a trailing ZWJ is not part of the cluster");
    expect_cluster("\xe2\x9c\xa8!", 3, (const uint32_t[]){0x2728}, 1, "an emoji-style symbol needs no U+FE0F");
    expect_cluster("\xe2\x9c\xa8\xef\xb8\x8e", 0, NULL, 0, "U+FE0E keeps an emoji-style symbol text");
    expect_cluster("\xe2\x86\x92", 0, NULL, 0, "an arrow is not an emoji");
}

// Pretends assets/emoji holds 1f3ae, 2764, 2728 and 1f468: 8x8 images, opaque red inside a transparent border.
static int emoji_loads = 0;
static int emoji_last_length = 0;

static unsigned char *fake_emoji_loader(const VdEmojiSequence *sequence, int *width, int *height)
{
    emoji_loads++;
    emoji_last_length = sequence->length;
    uint32_t first = sequence->codepoints[0];
    bool held = first == 0x1f3ae || first == 0x2764 || first == 0x2728 || first == 0x1f468;
    if (sequence->length != 1 || !held) return NULL;
    *width = *height = 8;
    unsigned char *pixels = calloc(8 * 8, 4);
    for (int row = 1; pixels && row < 7; row++) {
        for (int col = 1; col < 7; col++) {
            unsigned char *pixel = pixels + (row * 8 + col) * 4;
            pixel[0] = 255;
            pixel[3] = 255;
        }
    }
    return pixels;
}

static void test_emoji(const VdGlyphFont *font)
{
    glyph_cache_reset();
    glyph_cache_set_emoji_loader(fake_emoji_loader);
    const int size = 24;

    VdEmojiSequence controller = cluster_of("\xf0\x9f\x8e\xae");
    VdGlyph emoji = glyph_cache_get_emoji(font, size, &controller);
    expect(emoji.color && emoji.page == 0, "an emoji with an image is a colour glyph in an atlas page");
    expect(emoji.width == size && emoji.height == size && emoji.advance == (float)size,
           "an emoji is a Font Size square");
    expect(emoji.offset_y == -(int)roundf(glyph_font_metrics(font, size).ascent), "an emoji sits at the ascent");

    const unsigned char *pixels = glyph_cache_take_dirty_page(0);
    expect(pixels != NULL, "an emoji marks its page dirty");
    if (pixels) {
        const unsigned char *center =
            pixels + ((size_t)(emoji.y + size / 2) * VD_GLYPH_ATLAS_PAGE_SIZE + (size_t)(emoji.x + size / 2)) * 4;
        const unsigned char *corner = pixels + ((size_t)emoji.y * VD_GLYPH_ATLAS_PAGE_SIZE + (size_t)emoji.x) * 4;
        expect(center[0] == 255 && center[1] == 0 && center[2] == 0 && center[3] == 255,
               "an emoji keeps its colour in the RGBA page");
        expect(corner[3] == 0, "an emoji keeps its transparent border");
    }

    int loads = emoji_loads;
    VdGlyph again = glyph_cache_get_emoji(font, size, &controller);
    expect(same_glyph(emoji, again) && emoji_loads == loads, "a cached emoji does not load again");
    VdGlyph large = glyph_cache_get_emoji(font, size * 2, &controller);
    expect(large.width == size * 2 && !same_glyph(emoji, large), "another size is another emoji");

    VdEmojiSequence heart = cluster_of("\xe2\x9d\xa4\xef\xb8\x8f");
    expect(glyph_cache_get_emoji(font, size, &heart).color, "a symbol with U+FE0F resolves to its emoji image");
    VdEmojiSequence sparkles = cluster_of("\xe2\x9c\xa8");
    expect(glyph_cache_get_emoji(font, size, &sparkles).color, "an emoji-style symbol resolves to its emoji image");

    VdEmojiSequence family = cluster_of("\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x91\xa7");
    VdEmojiSequence man = cluster_of("\xf0\x9f\x91\xa8");
    loads = emoji_loads;
    VdGlyph from_family = glyph_cache_get_emoji(font, size, &family);
    expect(from_family.color, "a ZWJ sequence without an image resolves");
    expect(emoji_loads - loads == 3 && emoji_last_length == 1,
           "a ZWJ sequence tries itself and each shorter prefix that ends on an emoji");
    expect(same_glyph(from_family, glyph_cache_get_emoji(font, size, &man)), "a ZWJ sequence without an image draws as its first emoji");

    VdEmojiSequence missing = cluster_of("\xf0\x9f\x99\x82");
    VdGlyph fallback = glyph_cache_get(font, size, VD_EMOJI_FALLBACK);
    expect(same_glyph(glyph_cache_get_emoji(font, size, &missing), fallback),
           "an emoji without an image no font has draws the fallback glyph");
    loads = emoji_loads;
    expect(same_glyph(glyph_cache_get_emoji(font, size, &missing), fallback) && emoji_loads == loads,
           "a missing image is remembered");
    VdEmojiSequence keycap = cluster_of("1\xef\xb8\x8f\xe2\x83\xa3");
    expect(same_glyph(glyph_cache_get_emoji(font, size, &keycap), glyph_cache_get(font, size, '1')),
           "an emoji without an image draws its first codepoint when a font has it");

    glyph_cache_set_emoji_loader(NULL);
    glyph_cache_reset();
#ifdef VD_TWEMOJI_DOWNLOADED
    FILE *asset = fopen(VD_SOURCE_DIR "/assets/emoji/1f3f3-200d-1f308.png", "rb");
    expect(asset != NULL, "the Twemoji download names images without U+FE0F");
    if (asset) fclose(asset);
#endif
}

// A text-style symbol no font has draws its emoji image; one without an image keeps the missing-glyph box.
static void test_symbol_without_glyph(const VdGlyphFont *ascii)
{
    glyph_cache_reset();
    glyph_cache_set_fallback(NULL);
    glyph_cache_set_lazy_fonts(NULL, 0);
    glyph_cache_set_emoji_loader(fake_emoji_loader);
    const int size = 24;
    expect(glyph_cache_get(ascii, size, 0x2764).color, "a text-style symbol no font has draws its emoji image");
    VdGlyph arrow = glyph_cache_get(ascii, size, 0x2192);
    expect(!arrow.color && same_shape(arrow, glyph_cache_get(ascii, size, 0x2190)),
           "a symbol without an emoji image draws the missing-glyph box");
    glyph_cache_set_emoji_loader(NULL);
    glyph_cache_reset();
}

#define EMOJI_DIR VD_SOURCE_DIR "/assets/emoji"

// Names an image as load_emoji_image in src/ui/fonts.c does; false when it does not fit.
static bool emoji_file_name(const VdEmojiSequence *sequence, char *name, size_t size)
{
    size_t used = 0;
    for (int i = 0; i < sequence->length && used < size; i++) {
        used += (size_t)snprintf(name + used, size - used, i == 0 ? "%x" : "-%x", (unsigned)sequence->codepoints[i]);
    }
    return used < size;
}

static int file_loader_found = 0;

// Reads only whether the image exists: an opaque 1x1 image when it does.
static unsigned char *file_emoji_loader(const VdEmojiSequence *sequence, int *width, int *height)
{
    char name[128];
    char path[sizeof(EMOJI_DIR) + sizeof(name) + 8];
    if (!emoji_file_name(sequence, name, sizeof(name))) return NULL;
    snprintf(path, sizeof(path), "%s/%s.png", EMOJI_DIR, name);
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;
    fclose(file);
    file_loader_found = sequence->length;
    *width = *height = 1;
    unsigned char *pixels = malloc(4);
    if (pixels) memset(pixels, 255, 4);
    return pixels;
}

static int append_utf8(char *text, int used, uint32_t codepoint)
{
    unsigned char *out = (unsigned char *)text + used;
    if (codepoint < 0x80) {
        out[0] = (unsigned char)codepoint;
        return used + 1;
    }
    if (codepoint < 0x800) {
        out[0] = (unsigned char)(0xc0 | codepoint >> 6);
        out[1] = (unsigned char)(0x80 | (codepoint & 0x3f));
        return used + 2;
    }
    if (codepoint < 0x10000) {
        out[0] = (unsigned char)(0xe0 | codepoint >> 12);
        out[1] = (unsigned char)(0x80 | (codepoint >> 6 & 0x3f));
        out[2] = (unsigned char)(0x80 | (codepoint & 0x3f));
        return used + 3;
    }
    out[0] = (unsigned char)(0xf0 | codepoint >> 18);
    out[1] = (unsigned char)(0x80 | (codepoint >> 12 & 0x3f));
    out[2] = (unsigned char)(0x80 | (codepoint >> 6 & 0x3f));
    out[3] = (unsigned char)(0x80 | (codepoint & 0x3f));
    return used + 4;
}

// As text.c draws it: the emoji cluster at text, else its first codepoint.
static VdGlyph draw_text(const VdGlyphFont *font, int size, const char *text, int *bytes)
{
    VdEmojiSequence sequence;
    *bytes = vd_emoji_cluster(text, &sequence);
    if (*bytes > 0) return glyph_cache_get_emoji(font, size, &sequence);
    return glyph_cache_get(font, size, vd_utf8_next(text, bytes));
}

// Whether text draws as the image for the whole of sequence, from one cluster. A cached image does not load, so each
// pass over the same sequences takes its own size.
static bool draws_image(const VdGlyphFont *font, int size, const char *text, const VdEmojiSequence *sequence)
{
    file_loader_found = 0;
    int bytes = 0;
    VdGlyph glyph = draw_text(font, size, text, &bytes);
    return glyph.color && bytes == (int)strlen(text) && file_loader_found == sequence->length;
}

// Every Twemoji image, written with U+FE0F after each symbol below U+1F000 as fully qualified text has it, draws as
// itself; a single codepoint does without U+FE0F too, from the font that lacks it.
static void test_twemoji_files(const VdGlyphFont *ascii)
{
    DIR *dir = opendir(EMOJI_DIR);
    if (!dir) {
        printf("glyph_cache_harness: skipping Twemoji file cases, %s is missing\n", EMOJI_DIR);
        return;
    }
    glyph_cache_reset();
    glyph_cache_set_fallback(NULL);
    glyph_cache_set_lazy_fonts(NULL, 0);
    glyph_cache_set_emoji_loader(file_emoji_loader);

    int files = 0;
    for (struct dirent *entry; (entry = readdir(dir)) != NULL;) {
        size_t length = strlen(entry->d_name);
        if (length < 5 || strcmp(entry->d_name + length - 4, ".png") != 0) continue;
        files++;
        char stem[128];
        snprintf(stem, sizeof(stem), "%.*s", (int)(length - 4), entry->d_name);

        VdEmojiSequence sequence = {0};
        bool parsed = true;
        for (char *part = stem; parsed && *part != '\0';) {
            char *end = NULL;
            unsigned long codepoint = strtoul(part, &end, 16);
            parsed = end != part && (*end == '-' || *end == '\0') && sequence.length < VD_EMOJI_SEQUENCE_MAX;
            if (parsed) sequence.codepoints[sequence.length++] = (uint32_t)codepoint;
            part = *end == '-' ? end + 1 : end;
        }
        char name[128];
        if (!parsed || !emoji_file_name(&sequence, name, sizeof(name)) || strcmp(name, stem) != 0) {
            fprintf(stderr, "glyph_cache_harness: %s\n", entry->d_name);
            expect(false, "a Twemoji image is named by its lowercase hex codepoints, as the loader names it");
            continue;
        }

        char text[VD_EMOJI_SEQUENCE_MAX * 7 + 1] = {0};
        int used = 0;
        for (int i = 0; i < sequence.length; i++) {
            uint32_t codepoint = sequence.codepoints[i];
            used = append_utf8(text, used, codepoint);
            if (codepoint < 0x1f000 && codepoint != 0x200d && codepoint != 0x20e3) used = append_utf8(text, used, 0xfe0f);
        }
        if (!draws_image(ascii, 4, text, &sequence)) {
            fprintf(stderr, "glyph_cache_harness: %s\n", entry->d_name);
            expect(false, "fully qualified text draws its Twemoji image");
        }

        char bare[8] = {0};
        append_utf8(bare, 0, sequence.codepoints[0]);
        if (sequence.length == 1 && !draws_image(ascii, 5, bare, &sequence)) {
            fprintf(stderr, "glyph_cache_harness: %s\n", entry->d_name);
            expect(false, "a single codepoint without U+FE0F draws its Twemoji image when no font has it");
        }
    }
    closedir(dir);

    if (files == 0) {
        printf("glyph_cache_harness: skipping Twemoji file cases, %s has no images\n", EMOJI_DIR);
    } else {
        struct {
            const char *text;
            VdEmojiSequence sequence;
            const char *message;
        } cases[] = {
            {"\xe2\x9c\xa8", {{0x2728}, 1}, "sparkles draws its image"},
            {"\xe2\x9d\xa4\xef\xb8\x8f", {{0x2764}, 1}, "a heart with U+FE0F draws its image"},
            {"\xf0\x9f\x8e\xae", {{0x1f3ae}, 1}, "a video game controller draws its image"},
            {"\xf0\x9f\x8f\xb3\xef\xb8\x8f\xe2\x80\x8d\xf0\x9f\x8c\x88", {{0x1f3f3, 0x200d, 0x1f308}, 3},
             "a rainbow flag ZWJ sequence draws its own image"},
        };
        for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
            expect(draws_image(ascii, 6, cases[i].text, &cases[i].sequence), cases[i].message);
        }
        int bytes = 0;
        expect(!draw_text(ascii, 4, "\xe2\x86\x92", &bytes).color, "an arrow does not draw an image");
    }
    glyph_cache_set_emoji_loader(NULL);
    glyph_cache_reset();
}

// assets/emoji without images, as after a failed Twemoji download.
static void test_emoji_without_assets(const VdGlyphFont *full)
{
    glyph_cache_reset();
    glyph_cache_set_emoji_loader(missing_emoji_loader);
    const int size = 24;
    VdGlyph fallback = glyph_cache_get(full, size, VD_EMOJI_FALLBACK);
    expect(fallback.page >= 0 && fallback.width > 0 && !fallback.color, "the fallback glyph is a full glyph");

    const char *texts[] = {
        "\xf0\x9f\x8e\xae",
        "\xf0\x9f\x91\x8d\xf0\x9f\x8f\xbd",
        "\xf0\x9f\x87\xad\xf0\x9f\x87\xba",
        "\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x91\xa7",
    };
    int first_frame_loads = 0;
    for (int frame = 0; frame < 3; frame++) {
        glyph_cache_begin_frame();
        for (size_t i = 0; i < sizeof(texts) / sizeof(texts[0]); i++) {
            VdEmojiSequence sequence = cluster_of(texts[i]);
            expect(same_glyph(glyph_cache_get_emoji(full, size, &sequence), fallback),
                   "an emoji cluster without its image draws one fallback glyph");
        }
        if (frame == 0) first_frame_loads = missing_loads;
    }
    expect(first_frame_loads > 0 && missing_loads == first_frame_loads, "missing images load once, not every frame");

    VdEmojiSequence heart = cluster_of("\xe2\x9d\xa4\xef\xb8\x8f");
    expect(same_glyph(glyph_cache_get_emoji(full, size, &heart), glyph_cache_get(full, size, 0x2764)),
           "a symbol with U+FE0F and no image draws the full's symbol");
    glyph_cache_set_emoji_loader(NULL);
}

static void test_pages(const VdGlyphFont *full)
{
    glyph_cache_reset();
    glyph_cache_set_fallback(NULL);
    const int size = 300;

    uint32_t first = 0;
    uint32_t refused = 0;
    int last_page = -1;
    bool pages_in_order = true;
    for (uint32_t codepoint = 0x21; codepoint < 0x250 && !refused; codepoint++) {
        if (codepoint >= 0x7f && codepoint < 0xa1) continue;
        int before = glyph_cache_stats().rasterised;
        VdGlyph glyph = glyph_cache_get(full, size, codepoint);
        if (glyph.page < 0 && glyph.advance > 0.0f && glyph_cache_stats().rasterised == before) {
            refused = codepoint;
            break;
        }
        if (!first) first = codepoint;
        if (glyph.page < last_page || glyph.page > last_page + 1) pages_in_order = false;
        if (glyph.page > last_page) last_page = glyph.page;
    }
    expect(pages_in_order, "pages fill one after another");
    expect(glyph_cache_stats().pages == VD_GLYPH_ATLAS_PAGE_MAX, "pages grow up to the cap");
    expect(refused != 0, "with every page used this frame, a new glyph waits for a later frame");
    VdGlyph box = glyph_cache_get(full, size, refused);
    expect(box.page == -1 && box.width > 0 && box.height > 0, "a glyph with no room is a box to draw in its place");
    expect(glyph_cache_stats().overflowed == 2, "each lookup with no room counts as overflow");
    int glyphs_before = glyph_cache_stats().glyphs;

    glyph_cache_begin_frame();
    expect(glyph_cache_stats().overflowed == 0, "a frame starts without overflow");
    int rasterised_before = glyph_cache_stats().rasterised;
    for (uint32_t codepoint = first; codepoint <= refused; codepoint++) {
        if (codepoint >= 0x7f && codepoint < 0xa1) continue;
        glyph_cache_get(full, size, codepoint);
    }
    expect(glyph_cache_stats().rasterised == rasterised_before,
           "repeating an overflowing frame keeps its glyphs instead of clearing pages it draws from");
    expect(glyph_cache_stats().overflowed == 1, "repeating an overflowing frame overflows the same way");

    glyph_cache_begin_frame();
    VdGlyph placed = glyph_cache_get(full, size, refused);
    expect(placed.page == 0, "a later frame clears the least recently used page");
    expect(glyph_cache_stats().pages == VD_GLYPH_ATLAS_PAGE_MAX, "clearing a page keeps the cap");
    expect(glyph_cache_stats().glyphs < glyphs_before, "clearing a page drops its glyphs");

    int before = glyph_cache_stats().rasterised;
    glyph_cache_get(full, size, first);
    expect(glyph_cache_stats().rasterised == before + 1, "a glyph from a cleared page rasterises again");
}

static bool file_exists(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file) fclose(file);
    return file != NULL;
}

int main(void)
{
    test_utf8();
    test_emoji_clusters();
    test_blocks();

    VdGlyphFont *ascii = glyph_font_load(ASCII_FONT_PATH);
    expect(ascii != NULL, "the ASCII test font loads");
    expect(glyph_font_load(VD_SOURCE_DIR "/tests/fixtures/missing.ttf") == NULL, "a missing font file fails");
    expect(glyph_font_load(VD_SOURCE_DIR "/CMakeLists.txt") == NULL, "a file that is not a font fails");

    // The runtime fonts download at configure time, so an offline build tests what it can with the ASCII fixture.
    VdGlyphFont *full = glyph_font_load(FULL_FONT_PATH);
    if (!full) printf("glyph_cache_harness: skipping Runtime Default Font cases, %s is missing\n", FULL_FONT_PATH);
    bool cjk = file_exists(FALLBACK_FONT_PATH);
    if (!cjk) printf("glyph_cache_harness: skipping Runtime Fallback Font cases, %s is missing\n", FALLBACK_FONT_PATH);
    bool korean = file_exists(KOREAN_FONT_PATH);
    if (!korean) printf("glyph_cache_harness: skipping Korean font cases, %s is missing\n", KOREAN_FONT_PATH);

    if (ascii) {
        const VdGlyphFont *font = full ? full : ascii;
        test_hits_and_sizes(font);
        test_failing_fallback_loader(font);
        test_emoji(font);
        test_symbol_without_glyph(ascii);
        test_twemoji_files(ascii);
    }
    if (ascii && full) {
        test_fallback(full, ascii);
        test_hungarian(full);
        test_emoji_without_assets(full);
        test_pages(full);
        if (cjk) test_fallback_chain(full, ascii);
        if (korean) test_korean(full, ascii);
    }

    glyph_cache_reset();
    glyph_font_free(full);
    glyph_font_free(ascii);

    if (failures > 0) {
        fprintf(stderr, "glyph_cache_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("glyph_cache_harness: ok\n");
    return 0;
}
