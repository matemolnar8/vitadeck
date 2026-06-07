#include "fonts.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/package_library.h"

#define VD_FONT_LOAD_SIZE 64
#define VD_FONT_NAME_MAX 64
#define VD_FONT_MAX 32
#define VD_DEFAULT_FONT_PATH "assets/fonts/DejaVuSans.ttf"

typedef struct {
    char name[VD_FONT_NAME_MAX];
    Font font;
} VdLoadedFont;

typedef struct {
    char name[VD_FONT_NAME_MAX];
    char path[VD_PATH_MAX];
} VdManifestFont;

static Font g_default_font = {0};
static bool g_default_loaded = false;
static VdLoadedFont g_package_fonts[VD_FONT_MAX];
static int g_package_font_count = 0;

static void set_error(char *error, size_t error_size, const char *message)
{
    if (!error || error_size == 0) return;
    snprintf(error, error_size, "%s", message);
}

static char *read_text_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long len = ftell(f);
    if (len < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    char *buffer = malloc((size_t)len + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    size_t read_len = fread(buffer, 1, (size_t)len, f);
    buffer[read_len] = '\0';
    fclose(f);
    return buffer;
}

static void join_path(char *out, size_t out_size, const char *a, const char *b)
{
    snprintf(out, out_size, "%s/%s", a, b);
}

static const char *skip_ws(const char *p)
{
    while (*p && isspace((unsigned char)*p))
        p++;
    return p;
}

static bool parse_json_string(const char **cursor, char *out, size_t out_size)
{
    const char *p = skip_ws(*cursor);
    if (*p != '"') return false;
    p++;

    size_t len = 0;
    while (*p && *p != '"') {
        if (*p == '\\' || len + 1 >= out_size) return false;
        out[len++] = *p++;
    }
    if (*p != '"') return false;
    out[len] = '\0';
    *cursor = p + 1;
    return true;
}

static const char *find_fonts_object(const char *json)
{
    const char *p = strstr(json, "\"fonts\"");
    if (!p) return NULL;
    p = strchr(p + strlen("\"fonts\""), ':');
    if (!p) return NULL;
    p = skip_ws(p + 1);
    return *p == '{' ? p + 1 : NULL;
}

static bool parse_manifest_fonts(const char *json, VdManifestFont *fonts, int *out_count)
{
    *out_count = 0;
    const char *p = find_fonts_object(json);
    if (!p) return true;

    p = skip_ws(p);
    if (*p == '}') return true;

    while (*p) {
        if (*out_count >= VD_FONT_MAX) return false;
        VdManifestFont font = {0};
        if (!parse_json_string(&p, font.name, sizeof(font.name))) return false;
        p = skip_ws(p);
        if (*p != ':') return false;
        p++;
        if (!parse_json_string(&p, font.path, sizeof(font.path))) return false;
        fonts[*out_count] = font;
        (*out_count)++;

        p = skip_ws(p);
        if (*p == ',') {
            p++;
            continue;
        }
        if (*p == '}') return true;
        return false;
    }

    return false;
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

    char manifest_path[VD_PATH_MAX];
    join_path(manifest_path, sizeof(manifest_path), package_path, "manifest.json");
    char *manifest = read_text_file(manifest_path);
    if (!manifest) {
        set_error(error, error_size, "Deck App Package Manifest is missing.");
        return false;
    }

    VdManifestFont manifest_fonts[VD_FONT_MAX];
    int manifest_font_count = 0;
    bool ok = parse_manifest_fonts(manifest, manifest_fonts, &manifest_font_count);
    free(manifest);
    if (!ok) {
        set_error(error, error_size, "Deck App Package Manifest fonts are invalid.");
        return false;
    }

    for (int i = 0; i < manifest_font_count; i++) {
        char font_path[VD_PATH_MAX];
        join_path(font_path, sizeof(font_path), package_path, manifest_fonts[i].path);
        Font font = {0};
        if (!load_font_file(font_path, &font, error, error_size)) {
            unload_package_fonts();
            return false;
        }
        snprintf(g_package_fonts[g_package_font_count].name, sizeof(g_package_fonts[g_package_font_count].name), "%s",
                 manifest_fonts[i].name);
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
