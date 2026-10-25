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
#define VD_FALLBACK_FONT_PATH "assets/fonts/DroidSansFallbackFull.ttf"
#define VD_EMOJI_DIR "assets/emoji"

_Static_assert(VD_FONT_MAX >= VD_PACKAGE_FONT_MAX, "font registry must hold every manifest font");

typedef struct {
    char name[VD_FONT_NAME_MAX];
    VdFont font;
} VdLoadedFont;

static VdFont g_default_font = {0};
static bool g_default_loaded = false;
static VdGlyphFont *g_fallback_font = NULL;
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

// The Runtime Fallback Font is 4 MB, so it loads on the first character the other fonts lack, not at startup.
static VdGlyphFont *load_fallback_font(void)
{
    g_fallback_font = glyph_font_load(VD_FALLBACK_FONT_PATH);
    if (g_fallback_font) {
        TraceLog(LOG_DEBUG, "FONT: Runtime Fallback Font loaded from %s", VD_FALLBACK_FONT_PATH);
    } else {
        TraceLog(LOG_WARNING, "FONT: Could not load the Runtime Fallback Font from %s", VD_FALLBACK_FONT_PATH);
    }
    return g_fallback_font;
}

bool font_registry_init(char *error, size_t error_size)
{
    if (g_default_loaded) return true;
    if (!load_font_file(VD_DEFAULT_FONT_PATH, &g_default_font, error, error_size)) return false;
    glyph_cache_set_fallback(g_default_font.outline);
    glyph_cache_set_fallback_loader(load_fallback_font);
    glyph_cache_set_emoji_loader(load_emoji_image);
    g_default_loaded = true;
    return true;
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
    glyph_font_free(g_fallback_font);
    g_fallback_font = NULL;
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
