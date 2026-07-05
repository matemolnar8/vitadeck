#define JIMP_IMPLEMENTATION
#include "jimp/jimp.h"

#include "core/package_manifest.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void set_error(char *error, size_t error_size, const char *message)
{
    if (!error || error_size == 0) return;
    snprintf(error, error_size, "%s", message);
}

static bool has_suffix(const char *value, const char *suffix)
{
    size_t value_len = strlen(value);
    size_t suffix_len = strlen(suffix);
    return value_len >= suffix_len && strcmp(value + value_len - suffix_len, suffix) == 0;
}

static bool join_path(char *out, size_t out_size, const char *a, const char *b)
{
    if (!out || out_size == 0 || !a || !b) return false;
    size_t a_len = strlen(a);
    size_t b_len = strlen(b);
    if (a_len >= out_size || b_len >= out_size - a_len - 1) return false;
    memcpy(out, a, a_len);
    out[a_len] = '/';
    memcpy(out + a_len + 1, b, b_len + 1);
    return true;
}

static bool path_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static bool is_dir(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static char *read_text_file(const char *path, size_t *out_len)
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
    if (out_len) *out_len = read_len;
    return buffer;
}

static bool copy_string(char *out, size_t out_size, const char *value)
{
    if (!value || value[0] == '\0' || strlen(value) >= out_size) return false;
    snprintf(out, out_size, "%s", value);
    return true;
}

static bool valid_semverish(const char *version)
{
    int dots = 0;
    bool saw_digit = false;
    for (const char *p = version; *p; p++) {
        if (isdigit((unsigned char)*p)) {
            saw_digit = true;
            continue;
        }
        if (*p == '.') {
            dots++;
            continue;
        }
        if (*p == '-' || *p == '+') break;
        return false;
    }
    return saw_digit && dots >= 2;
}

static bool safe_asset_name(const char *name, bool allow_default)
{
    if (!name || name[0] == '\0') return false;
    if (!allow_default && strcmp(name, "default") == 0) return false;
    size_t len = strlen(name);
    if (len >= VD_PACKAGE_ASSET_NAME_MAX || !isalpha((unsigned char)name[0])) return false;
    for (const char *p = name; *p; p++) {
        if (!isalnum((unsigned char)*p) && *p != '_' && *p != '-') return false;
    }
    return true;
}

static bool safe_relative_path(const char *path)
{
    return path && path[0] != '\0' && path[0] != '/' && !strstr(path, "\\") && !strstr(path, "..");
}

static bool supported_path_extension(const char *path, const char *const *extensions, int extension_count)
{
    const char *dot = strrchr(path, '.');
    if (!dot) return false;
    char ext[8];
    size_t len = strlen(dot);
    if (len >= sizeof(ext)) return false;
    for (size_t i = 0; i <= len; i++)
        ext[i] = (char)tolower((unsigned char)dot[i]);

    for (int i = 0; i < extension_count; i++) {
        if (strcmp(ext, extensions[i]) == 0) return true;
    }
    return false;
}

static bool safe_package_name(const char *package_name)
{
    return package_name && package_name[0] != '\0' && strlen(package_name) < VD_PACKAGE_NAME_MAX &&
           has_suffix(package_name, ".vdapp") && !strstr(package_name, "/") && !strstr(package_name, "\\") &&
           !strstr(package_name, "..");
}

static bool skip_json_value(Jimp *jimp)
{
    if (jimp_is_string_ahead(jimp)) return jimp_string(jimp);
    if (jimp_is_number_ahead(jimp)) return jimp_number(jimp);
    if (jimp_is_bool_ahead(jimp)) return jimp_bool(jimp);
    if (jimp_is_null_ahead(jimp)) return jimp__get_and_expect_token(jimp, JIMP_NULL);
    if (jimp_is_array_ahead(jimp)) {
        if (!jimp_array_begin(jimp)) return false;
        while (jimp_array_item(jimp)) {
            if (!skip_json_value(jimp)) return false;
        }
        return jimp_array_end(jimp);
    }
    if (jimp_is_object_ahead(jimp)) {
        if (!jimp_object_begin(jimp)) return false;
        while (jimp_object_member(jimp)) {
            if (!skip_json_value(jimp)) return false;
        }
        return jimp_object_end(jimp);
    }
    return false;
}

static bool read_json_string(Jimp *jimp, char *out, size_t out_size)
{
    if (!jimp_string(jimp)) return false;
    return copy_string(out, out_size, jimp->string);
}

static bool read_schema_version(Jimp *jimp, int *out)
{
    if (!jimp_number(jimp)) return false;
    int value = (int)jimp->number;
    if ((double)value != jimp->number) return false;
    *out = value;
    return true;
}

static bool validate_asset_file(const char *package_path, const char *rel_path, const char *invalid_message,
                                const char *missing_message, char *error, size_t error_size)
{
    char asset_path[VD_PATH_MAX];
    if (!join_path(asset_path, sizeof(asset_path), package_path, rel_path)) {
        set_error(error, error_size, invalid_message);
        return false;
    }
    if (!path_exists(asset_path)) {
        set_error(error, error_size, missing_message);
        return false;
    }
    return true;
}

static bool parse_asset_object(Jimp *jimp, const char *package_path, VdPackageAsset *assets, int *out_count,
                               int max_count, bool is_font, char *error, size_t error_size)
{
    static const char *font_extensions[] = {".ttf", ".otf", ".fnt", ".bdf"};
    static const char *image_extensions[] = {".png", ".jpg", ".jpeg", ".bmp", ".tga",
                                             ".gif", ".psd", ".hdr",  ".pic", ".qoi"};

    *out_count = 0;
    if (!jimp_object_begin(jimp)) {
        set_error(error, error_size,
                  is_font ? "Deck App Package Manifest fonts are invalid."
                          : "Deck App Package Manifest images are invalid.");
        return false;
    }

    while (jimp_object_member(jimp)) {
        if (*out_count >= max_count) {
            set_error(error, error_size,
                      is_font ? "Deck App Package declares too many fonts."
                              : "Deck App Package declares too many images.");
            return false;
        }

        char name[VD_PACKAGE_ASSET_NAME_MAX];
        if (!copy_string(name, sizeof(name), jimp->string) || !safe_asset_name(name, !is_font)) {
            set_error(error, error_size,
                      is_font ? "Deck App Package declares an invalid font name."
                              : "Deck App Package declares an invalid image name.");
            return false;
        }

        char rel_path[VD_PATH_MAX];
        const char *const *extensions = is_font ? font_extensions : image_extensions;
        int extension_count = is_font ? (int)(sizeof(font_extensions) / sizeof(font_extensions[0]))
                                      : (int)(sizeof(image_extensions) / sizeof(image_extensions[0]));
        if (!read_json_string(jimp, rel_path, sizeof(rel_path)) || !safe_relative_path(rel_path) ||
            !supported_path_extension(rel_path, extensions, extension_count)) {
            set_error(error, error_size,
                      is_font ? "Deck App Package declares an invalid font path."
                              : "Deck App Package declares an invalid image path.");
            return false;
        }

        if (!validate_asset_file(package_path, rel_path,
                                 is_font ? "Deck App Package declares an invalid font path."
                                         : "Deck App Package declares an invalid image path.",
                                 is_font ? "Deck App Package Font is missing." : "Deck App Package Image is missing.",
                                 error, error_size)) {
            return false;
        }

        snprintf(assets[*out_count].name, sizeof(assets[*out_count].name), "%s", name);
        snprintf(assets[*out_count].path, sizeof(assets[*out_count].path), "%s", rel_path);
        (*out_count)++;
    }

    if (!jimp_object_end(jimp)) {
        set_error(error, error_size,
                  is_font ? "Deck App Package Manifest fonts are invalid."
                          : "Deck App Package Manifest images are invalid.");
        return false;
    }
    return true;
}

bool package_manifest_read(const char *package_path, const char *package_name, VdPackageManifest *out_manifest,
                           char *error, size_t error_size)
{
    if (error && error_size > 0) error[0] = '\0';
    if (!out_manifest) {
        set_error(error, error_size, "Deck App Package Manifest is invalid.");
        return false;
    }
    memset(out_manifest, 0, sizeof(*out_manifest));

    if (!safe_package_name(package_name)) {
        set_error(error, error_size, "Invalid Deck App Package Name.");
        return false;
    }
    if (!is_dir(package_path)) {
        set_error(error, error_size, "Deck App Package Directory is missing.");
        return false;
    }

    char manifest_path[VD_PATH_MAX];
    if (!join_path(manifest_path, sizeof(manifest_path), package_path, "manifest.json")) {
        set_error(error, error_size, "Deck App Package Manifest is missing.");
        return false;
    }
    size_t manifest_len = 0;
    char *manifest_json = read_text_file(manifest_path, &manifest_len);
    if (!manifest_json) {
        set_error(error, error_size, "Deck App Package Manifest is missing.");
        return false;
    }

    Jimp jimp = {0};
    jimp_begin(&jimp, manifest_path, manifest_json, manifest_len);

    bool saw_schema_version = false;
    bool saw_name = false;
    bool saw_version = false;
    bool saw_entry = false;
    int schema_version = 0;

    bool ok = jimp_object_begin(&jimp);
    while (ok && jimp_object_member(&jimp)) {
        char key[64];
        if (!copy_string(key, sizeof(key), jimp.string)) {
            ok = false;
            break;
        }

        if (strcmp(key, "schemaVersion") == 0) {
            ok = read_schema_version(&jimp, &schema_version);
            saw_schema_version = ok;
        } else if (strcmp(key, "name") == 0) {
            ok = read_json_string(&jimp, out_manifest->display_name, sizeof(out_manifest->display_name));
            saw_name = ok;
        } else if (strcmp(key, "version") == 0) {
            ok = read_json_string(&jimp, out_manifest->version, sizeof(out_manifest->version));
            saw_version = ok;
        } else if (strcmp(key, "entry") == 0) {
            ok = read_json_string(&jimp, out_manifest->entry, sizeof(out_manifest->entry));
            saw_entry = ok;
        } else if (strcmp(key, "fonts") == 0) {
            ok = parse_asset_object(&jimp, package_path, out_manifest->fonts, &out_manifest->font_count,
                                    VD_PACKAGE_FONT_MAX, true, error, error_size);
        } else if (strcmp(key, "images") == 0) {
            ok = parse_asset_object(&jimp, package_path, out_manifest->images, &out_manifest->image_count,
                                    VD_PACKAGE_IMAGE_MAX, false, error, error_size);
        } else {
            ok = skip_json_value(&jimp);
        }
    }
    if (ok) ok = jimp_object_end(&jimp);

    if (!ok) {
        if (!error || error_size == 0 || error[0] == '\0') {
            set_error(error, error_size, "Deck App Package Manifest is invalid.");
        }
        free(jimp.string);
        free(manifest_json);
        return false;
    }

    if (!saw_schema_version || !saw_name || !saw_version || !saw_entry || schema_version != 1 ||
        strcmp(out_manifest->entry, "app.js") != 0 || !valid_semverish(out_manifest->version)) {
        set_error(error, error_size, "Deck App Package Manifest is invalid.");
        free(jimp.string);
        free(manifest_json);
        return false;
    }

    char entry_path[VD_PATH_MAX];
    if (!join_path(entry_path, sizeof(entry_path), package_path, out_manifest->entry)) {
        set_error(error, error_size, "Deck App Package Entry is missing.");
        free(jimp.string);
        free(manifest_json);
        return false;
    }
    if (!path_exists(entry_path)) {
        set_error(error, error_size, "Deck App Package Entry is missing.");
        free(jimp.string);
        free(manifest_json);
        return false;
    }

    snprintf(out_manifest->package_name, sizeof(out_manifest->package_name), "%s", package_name);

    free(jimp.string);
    free(manifest_json);
    return true;
}
