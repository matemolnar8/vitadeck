#include "fonts.h"

#include <stdio.h>
#include <string.h>
#include "core/package_manifest.h"
#include "core/package_library.h"

#define VD_FONT_LOAD_SIZE 64
#define VD_FONT_NAME_MAX 64
#define VD_FONT_MAX 32
#define VD_DEFAULT_FONT_PATH "assets/fonts/DejaVuSans.ttf"

typedef struct {
    char name[VD_FONT_NAME_MAX];
    Font font;
} VdLoadedFont;

static Font g_default_font = {0};
static bool g_default_loaded = false;
static VdLoadedFont g_package_fonts[VD_FONT_MAX];
static int g_package_font_count = 0;

static void set_error(char *error, size_t error_size, const char *message)
{
    if (!error || error_size == 0) return;
    snprintf(error, error_size, "%s", message);
}

static void join_path(char *out, size_t out_size, const char *a, const char *b)
{
    snprintf(out, out_size, "%s/%s", a, b);
}

static void unload_package_fonts(void)
{
    for (int i = 0; i < g_package_font_count; i++) {
        UnloadFont(g_package_fonts[i].font);
    }
    g_package_font_count = 0;
}

static bool load_font_file(const char *path, Font *out, char *error, size_t error_size)
{
    Font font = LoadFontEx(path, VD_FONT_LOAD_SIZE, NULL, 0);
    if (!IsFontValid(font)) {
        set_error(error, error_size, "Could not load font file.");
        return false;
    }
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    *out = font;
    return true;
}

bool font_registry_init(char *error, size_t error_size)
{
    if (g_default_loaded) return true;
    if (!load_font_file(VD_DEFAULT_FONT_PATH, &g_default_font, error, error_size)) return false;
    g_default_loaded = true;
    return true;
}

bool font_registry_load_package(const char *package_path, char *error, size_t error_size)
{
    unload_package_fonts();
    if (!package_path || package_path[0] == '\0') return true;

    VdPackageManifest manifest;
    if (!package_manifest_read(package_path, package_library_active_package_name(), &manifest, error, error_size)) {
        return false;
    }

    for (int i = 0; i < manifest.font_count; i++) {
        char font_path[VD_PATH_MAX];
        join_path(font_path, sizeof(font_path), package_path, manifest.fonts[i].path);
        Font font = {0};
        if (!load_font_file(font_path, &font, error, error_size)) {
            unload_package_fonts();
            return false;
        }
        snprintf(g_package_fonts[g_package_font_count].name, sizeof(g_package_fonts[g_package_font_count].name), "%s",
                 manifest.fonts[i].name);
        g_package_fonts[g_package_font_count].font = font;
        g_package_font_count++;
    }

    return true;
}

void font_registry_shutdown(void)
{
    unload_package_fonts();
    if (g_default_loaded) {
        UnloadFont(g_default_font);
        g_default_loaded = false;
    }
}

Font font_registry_default(void)
{
    return g_default_loaded ? g_default_font : GetFontDefault();
}

Font font_registry_get(const char *name)
{
    if (!name || name[0] == '\0' || strcmp(name, VD_FONT_DEFAULT_NAME) == 0) return font_registry_default();
    for (int i = 0; i < g_package_font_count; i++) {
        if (strcmp(g_package_fonts[i].name, name) == 0) return g_package_fonts[i].font;
    }
    return font_registry_default();
}
