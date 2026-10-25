#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "ui/glyph_cache.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DejaVuSans-ascii.ttf is assets/fonts/DejaVuSans.ttf subset to U+0020-007E, under the same licence.
#define FULL_FONT_PATH VD_SOURCE_DIR "/assets/fonts/DejaVuSans.ttf"
#define ASCII_FONT_PATH VD_SOURCE_DIR "/tests/fixtures/DejaVuSans-ascii.ttf"
#define FALLBACK_FONT_PATH VD_SOURCE_DIR "/assets/fonts/DroidSansFallbackFull.ttf"

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

static void test_hits_and_sizes(const VdGlyphFont *full)
{
    glyph_cache_reset();

    VdGlyph a = glyph_cache_get(full, 30, 'A');
    expect(a.page == 0 && a.width > 0 && a.height > 0 && a.advance > 0.0f, "a miss rasterises into the first page");
    expect(a.offset_y < 0, "a capital sits above the baseline");
    expect(glyph_cache_stats().rasterised == 1, "the first lookup rasterises");

    VdGlyph again = glyph_cache_get(full, 30, 'A');
    expect(same_glyph(a, again), "a hit returns the cached glyph");
    expect(glyph_cache_stats().rasterised == 1, "a hit does not rasterise");

    VdGlyph large = glyph_cache_get(full, 60, 'A');
    expect(glyph_cache_stats().rasterised == 2, "another size is another glyph");
    expect(!same_glyph(a, large), "sizes do not share a glyph");
    float ratio = (float)large.height / (float)a.height;
    expect(ratio > 1.8f && ratio < 2.2f, "a glyph at twice the size is about twice as tall");

    VdGlyph space = glyph_cache_get(full, 30, ' ');
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

    VdGlyphFontMetrics metrics = glyph_font_metrics(full, 30);
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

static void test_fallback_chain(const VdGlyphFont *full, const VdGlyphFont *ascii)
{
    const uint32_t ideograph = 0x65e5; // 日
    const uint32_t kana = 0x3042;      // あ
    const uint32_t private_use = 0xe000;

    glyph_cache_reset();
    glyph_cache_set_fallback(NULL);
    glyph_cache_set_fallback_loader(NULL);
    VdGlyph ascii_notdef = glyph_cache_get(ascii, 30, private_use);
    VdGlyph full_notdef = glyph_cache_get(full, 30, private_use);
    glyph_cache_reset();

    fallback_loads = 0;
    glyph_cache_set_fallback(full);
    glyph_cache_set_fallback_loader(counting_fallback_loader);
    VdGlyph own = glyph_cache_get(ascii, 30, 'g');
    VdGlyph latin = glyph_cache_get(ascii, 30, 0x153); // œ
    expect(own.width > 0 && latin.width > 0 && !same_shape(latin, ascii_notdef),
           "Latin text resolves from the requested font and the Runtime Default Font");
    expect(fallback_loads == 0, "Latin text does not open the Runtime Fallback Font");

    VdGlyph from_default = glyph_cache_get(full, 30, ideograph);
    expect(fallback_loads == 1 && fallback_font != NULL,
           "the first character both fonts lack opens the Runtime Fallback Font");
    VdGlyph from_deck_font = glyph_cache_get(ascii, 30, kana);
    expect(fallback_loads == 1, "the Runtime Fallback Font opens once");
    if (fallback_font) {
        VdGlyph droid_ideograph = glyph_cache_get(fallback_font, 30, ideograph);
        VdGlyph droid_kana = glyph_cache_get(fallback_font, 30, kana);
        expect(same_shape(from_default, droid_ideograph) && !same_shape(from_default, full_notdef),
               "an ideograph the Runtime Default Font lacks comes from the Runtime Fallback Font");
        expect(same_shape(from_deck_font, droid_kana) && !same_shape(from_deck_font, ascii_notdef),
               "kana a Deck App Font lacks comes from the Runtime Fallback Font");
    }

    VdGlyph unknown = glyph_cache_get(ascii, 30, private_use);
    expect(same_shape(unknown, ascii_notdef), "a character no font has is the requested font's missing-glyph box");
    expect(fallback_loads == 1, "a character no font has does not open the Runtime Fallback Font again");

    glyph_cache_reset();
    fallback_loads = 0;
    glyph_cache_set_fallback_loader(failing_fallback_loader);
    glyph_cache_get(full, 30, ideograph);
    glyph_cache_get(full, 30, kana);
    expect(fallback_loads == 1, "a Runtime Fallback Font that fails to open is not retried");

    glyph_cache_reset();
    glyph_cache_set_fallback_loader(NULL);
    glyph_font_free(fallback_font);
    fallback_font = NULL;
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
}

// Pretends assets/emoji holds 1f3ae, 2764 and 1f468: 8x8 images, opaque red inside a transparent border.
static int emoji_loads = 0;
static int emoji_last_length = 0;

static unsigned char *fake_emoji_loader(const VdEmojiSequence *sequence, int *width, int *height)
{
    emoji_loads++;
    emoji_last_length = sequence->length;
    uint32_t first = sequence->codepoints[0];
    if (sequence->length != 1 || (first != 0x1f3ae && first != 0x2764 && first != 0x1f468)) return NULL;
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

static VdEmojiSequence cluster_of(const char *text)
{
    VdEmojiSequence sequence = {0};
    vd_emoji_cluster(text, &sequence);
    return sequence;
}

static void test_emoji(const VdGlyphFont *full)
{
    glyph_cache_reset();
    glyph_cache_set_emoji_loader(fake_emoji_loader);
    const int size = 24;

    VdEmojiSequence controller = cluster_of("\xf0\x9f\x8e\xae");
    VdGlyph emoji = {0};
    expect(glyph_cache_get_emoji(full, size, &controller, &emoji), "an emoji with an image resolves");
    expect(emoji.color && emoji.page == 0, "an emoji is a colour glyph in an atlas page");
    expect(emoji.width == size && emoji.height == size && emoji.advance == (float)size,
           "an emoji is a Font Size square");
    expect(emoji.offset_y == -(int)roundf(glyph_font_metrics(full, size).ascent), "an emoji sits at the ascent");

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
    VdGlyph again = {0};
    glyph_cache_get_emoji(full, size, &controller, &again);
    expect(same_glyph(emoji, again) && emoji_loads == loads, "a cached emoji does not load again");
    VdGlyph large = {0};
    glyph_cache_get_emoji(full, size * 2, &controller, &large);
    expect(large.width == size * 2 && !same_glyph(emoji, large), "another size is another emoji");

    VdEmojiSequence heart = cluster_of("\xe2\x9d\xa4\xef\xb8\x8f");
    VdGlyph red_heart = {0};
    expect(glyph_cache_get_emoji(full, size, &heart, &red_heart) && red_heart.color,
           "a symbol with U+FE0F resolves to its emoji image");

    VdEmojiSequence family = cluster_of("\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x91\xa7");
    VdEmojiSequence man = cluster_of("\xf0\x9f\x91\xa8");
    VdGlyph from_family = {0}, from_man = {0};
    loads = emoji_loads;
    expect(glyph_cache_get_emoji(full, size, &family, &from_family), "a ZWJ sequence without an image resolves");
    expect(emoji_loads - loads == 3 && emoji_last_length == 1,
           "a ZWJ sequence tries itself and each shorter prefix that ends on an emoji");
    glyph_cache_get_emoji(full, size, &man, &from_man);
    expect(same_glyph(from_family, from_man), "a ZWJ sequence without an image draws as its first emoji");

    VdEmojiSequence missing = cluster_of("\xf0\x9f\x99\x82");
    VdGlyph none = {0};
    expect(!glyph_cache_get_emoji(full, size, &missing, &none), "an emoji without an image does not resolve");
    loads = emoji_loads;
    expect(!glyph_cache_get_emoji(full, size, &missing, &none) && emoji_loads == loads,
           "a missing image is remembered");

    glyph_cache_set_emoji_loader(NULL);
    glyph_cache_reset();
    FILE *asset = fopen(VD_SOURCE_DIR "/assets/emoji/1f3ae.png", "rb");
    expect(asset != NULL, "Twemoji images ship under assets/emoji, named without U+FE0F");
    if (asset) fclose(asset);
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
    int glyphs_before = glyph_cache_stats().glyphs;

    glyph_cache_begin_frame();
    VdGlyph placed = glyph_cache_get(full, size, refused);
    expect(placed.page == 0, "a later frame clears the least recently used page");
    expect(glyph_cache_stats().pages == VD_GLYPH_ATLAS_PAGE_MAX, "clearing a page keeps the cap");
    expect(glyph_cache_stats().glyphs < glyphs_before, "clearing a page drops its glyphs");

    int before = glyph_cache_stats().rasterised;
    glyph_cache_get(full, size, first);
    expect(glyph_cache_stats().rasterised == before + 1, "a glyph from a cleared page rasterises again");
}

int main(void)
{
    test_utf8();
    test_emoji_clusters();

    VdGlyphFont *full = glyph_font_load(FULL_FONT_PATH);
    VdGlyphFont *ascii = glyph_font_load(ASCII_FONT_PATH);
    expect(full && ascii, "test fonts load");
    expect(glyph_font_load(VD_SOURCE_DIR "/tests/fixtures/missing.ttf") == NULL, "a missing font file fails");
    expect(glyph_font_load(VD_SOURCE_DIR "/CMakeLists.txt") == NULL, "a file that is not a font fails");

    if (full && ascii) {
        test_hits_and_sizes(full);
        test_fallback(full, ascii);
        test_fallback_chain(full, ascii);
        test_hungarian(full);
        test_emoji(full);
        test_pages(full);
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
