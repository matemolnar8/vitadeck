#include "fonts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/util.h"
#include "text.h"

#define VD_BITMAP_FONT_LOAD_SIZE 64
#define VD_FONT_NAME_MAX 64
#define VD_FONT_MAX 32
#define VD_DEFAULT_FONT_PATH "assets/fonts/DejaVuSans.ttf"
#define VD_CJK_FONT_PATH "assets/fonts/DroidSansFallbackFull.ttf"
#define VD_KOREAN_FONT_PATH "assets/fonts/NotoSansKR-Regular.otf"
#define VD_EMOJI_DIR "assets/emoji"

_Static_assert(VD_FONT_MAX >= VD_PACKAGE_FONT_MAX, "font registry must hold every manifest font");

typedef struct {
    char name[VD_FONT_NAME_MAX];
    VdFont font;
} VdLoadedFont;

static VdFont g_default_font = {0};
static bool g_default_loaded = false;
static VdGlyphFont *g_cjk_font = NULL;
static VdGlyphFont *g_korean_font = NULL;
static VdLoadedFont g_package_fonts[VD_FONT_MAX];
static int g_package_font_count = 0;

static void unload_font(VdFont *font)
{
    if (font->outline) {
        glyph_font_free(font->outline);
    } else {
        UnloadFont(font->bitmap);
    }
    *font = (VdFont){0};
}

static void unload_package_fonts(void)
{
    if (g_package_font_count == 0) return;
    // Cached glyphs are keyed by font.
    text_reset();
    for (int i = 0; i < g_package_font_count; i++) {
        unload_font(&g_package_fonts[i].font);
    }
    g_package_font_count = 0;
}

static bool load_font_file(const char *path, VdFont *out, char *error, size_t error_size)
{
    if (vd_has_suffix(path, ".ttf") || vd_has_suffix(path, ".otf")) {
        VdGlyphFont *outline = glyph_font_load(path);
        if (!outline) {
            vd_set_error(error, error_size, "Could not load font file.");
            return false;
        }
        *out = (VdFont){.outline = outline};
        return true;
    }

    Font font = LoadFontEx(path, VD_BITMAP_FONT_LOAD_SIZE, NULL, 0);
    if (!IsFontValid(font)) {
        vd_set_error(error, error_size, "Could not load font file.");
        return false;
    }
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    *out = (VdFont){.bitmap = font};
    return true;
}

// Twemoji names an image by its lowercase hex codepoints joined by '-', like 1f468-200d-1f469.png.
static unsigned char *load_emoji_image(const VdEmojiSequence *sequence, int *width, int *height)
{
    char path[VD_PATH_MAX];
    size_t used = (size_t)snprintf(path, sizeof(path), "%s/", VD_EMOJI_DIR);
    for (int i = 0; i < sequence->length && used < sizeof(path); i++) {
        used += (size_t)snprintf(path + used, sizeof(path) - used, i == 0 ? "%x" : "-%x",
                                 (unsigned)sequence->codepoints[i]);
    }
    if (used < sizeof(path)) used += (size_t)snprintf(path + used, sizeof(path) - used, ".png");
    if (used >= sizeof(path)) return NULL;

    size_t length = 0;
    char *data = vd_read_file(path, &length);
    if (!data) return NULL;
    Image image = LoadImageFromMemory(".png", (const unsigned char *)data, (int)length);
    free(data);
    if (!IsImageValid(image)) return NULL;
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    size_t bytes = (size_t)image.width * (size_t)image.height * 4;
    unsigned char *pixels = malloc(bytes);
    if (pixels) memcpy(pixels, image.data, bytes);
    *width = image.width;
    *height = image.height;
    UnloadImage(image);
    return pixels;
}

// Each Runtime Fallback Font is over 4 MB, so it loads on the first character in its blocks the other fonts lack, not
// at startup.
static VdGlyphFont *load_fallback_font(const char *path)
{
    VdGlyphFont *font = glyph_font_load(path);
    if (font) {
        TraceLog(LOG_DEBUG, "FONT: Runtime Fallback Font loaded from %s", path);
    } else {
        TraceLog(LOG_WARNING, "FONT: Could not load the Runtime Fallback Font from %s", path);
    }
    return font;
}

static VdGlyphFont *load_cjk_font(void)
{
    g_cjk_font = load_fallback_font(VD_CJK_FONT_PATH);
    return g_cjk_font;
}

static VdGlyphFont *load_korean_font(void)
{
    g_korean_font = load_fallback_font(VD_KOREAN_FONT_PATH);
    return g_korean_font;
}

static const VdLazyFont g_fallback_fonts[] = {
    {load_cjk_font, vd_in_cjk_blocks},
    {load_korean_font, vd_in_hangul_blocks},
};

enum { FALLBACK_FONT_COUNT = sizeof(g_fallback_fonts) / sizeof(g_fallback_fonts[0]) };
_Static_assert(FALLBACK_FONT_COUNT <= VD_LAZY_FONT_MAX, "the glyph cache must hold every Runtime Fallback Font");

// A build that could not download the Runtime Default Font draws it with raylib's built-in font.
void font_registry_init(void)
{
    if (g_default_loaded) return;
    g_default_font.outline = glyph_font_load(VD_DEFAULT_FONT_PATH);
    if (!g_default_font.outline) {
        TraceLog(LOG_WARNING, "FONT: Could not load the Runtime Default Font from %s; using raylib's built-in font",
                 VD_DEFAULT_FONT_PATH);
        g_default_font.bitmap = GetFontDefault();
    }
    glyph_cache_set_fallback(g_default_font.outline);
    glyph_cache_set_lazy_fonts(g_fallback_fonts, FALLBACK_FONT_COUNT);
    glyph_cache_set_emoji_loader(load_emoji_image);
    g_default_loaded = true;
}

bool font_registry_load_package(const char *package_path, const VdPackageManifest *manifest, char *error,
                                size_t error_size)
{
    unload_package_fonts();
    if (!manifest) return true;

    for (int i = 0; i < manifest->font_count; i++) {
        char font_path[VD_PATH_MAX];
        vd_join_path(font_path, sizeof(font_path), package_path, manifest->fonts[i].path);
        VdFont font = {0};
        if (!load_font_file(font_path, &font, error, error_size)) {
            unload_package_fonts();
            return false;
        }
        snprintf(g_package_fonts[g_package_font_count].name, sizeof(g_package_fonts[g_package_font_count].name), "%s",
                 manifest->fonts[i].name);
        g_package_fonts[g_package_font_count].font = font;
        g_package_font_count++;
    }

    return true;
}

void font_registry_shutdown(void)
{
    unload_package_fonts();
    text_reset();
    if (g_default_loaded) {
        unload_font(&g_default_font);
        g_default_loaded = false;
    }
    glyph_font_free(g_cjk_font);
    glyph_font_free(g_korean_font);
    g_cjk_font = g_korean_font = NULL;
}

const VdFont *font_registry_get(const char *name)
{
    if (name && name[0] != '\0' && strcmp(name, VD_FONT_DEFAULT_NAME) != 0) {
        for (int i = 0; i < g_package_font_count; i++) {
            if (strcmp(g_package_fonts[i].name, name) == 0) return &g_package_fonts[i].font;
        }
    }
    if (!g_default_loaded) g_default_font.bitmap = GetFontDefault();
    return &g_default_font;
}
