#include "package_library.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __vita__
#define VD_DATA_ROOT "ux0:data/vitadeck"
#else
#define VD_DATA_ROOT "vitadeck-data"
#endif

#define VD_FONT_NAME_MAX 64
#define VD_PACKAGE_FONT_MAX 32

static char g_root[VD_PATH_MAX] = VD_DATA_ROOT;
static char g_installed_root[VD_PATH_MAX];
static char g_staging_root[VD_PATH_MAX];
static char g_active_name[VD_PACKAGE_NAME_MAX];
static char g_active_path[VD_PATH_MAX];

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

static void join_path(char *out, size_t out_size, const char *a, const char *b)
{
    snprintf(out, out_size, "%s/%s", a, b);
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

static bool mkdir_p(const char *path)
{
    char tmp[VD_PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len == 0) return false;
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; p++) {
        if (*p != '/') continue;
        *p = '\0';
        if (mkdir(tmp, 0777) != 0 && errno != EEXIST) return false;
        *p = '/';
    }
    return mkdir(tmp, 0777) == 0 || errno == EEXIST;
}

static bool remove_tree(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir) return unlink(path) == 0 || errno == ENOENT;

    struct dirent *entry;
    bool ok = true;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char child[VD_PATH_MAX];
        join_path(child, sizeof(child), path, entry->d_name);
        if (!remove_tree(child)) {
            ok = false;
            break;
        }
    }
    closedir(dir);
    return ok && (rmdir(path) == 0 || errno == ENOENT);
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

static bool write_text_file(const char *path, const char *contents)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = fwrite(contents, 1, strlen(contents), f) == strlen(contents);
    fclose(f);
    return ok;
}

static bool json_string_value(const char *json, const char *key, char *out, size_t out_size)
{
    char needle[64];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(json, needle);
    if (!p) return false;
    p = strchr(p + strlen(needle), ':');
    if (!p) return false;
    p++;
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != '"') return false;
    p++;

    size_t len = 0;
    while (*p && *p != '"' && len + 1 < out_size) {
        if (*p == '\\') return false;
        out[len++] = *p++;
    }
    if (*p != '"') return false;
    out[len] = '\0';
    return len > 0;
}

static bool json_int_value(const char *json, const char *key, int *out)
{
    char needle[64];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(json, needle);
    if (!p) return false;
    p = strchr(p + strlen(needle), ':');
    if (!p) return false;
    p++;
    while (*p && isspace((unsigned char)*p))
        p++;
    char *end = NULL;
    long value = strtol(p, &end, 10);
    if (end == p) return false;
    *out = (int)value;
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

static bool safe_font_name(const char *name)
{
    if (!name || name[0] == '\0' || strcmp(name, "default") == 0) return false;
    size_t len = strlen(name);
    if (len >= VD_FONT_NAME_MAX || !isalpha((unsigned char)name[0])) return false;
    for (const char *p = name; *p; p++) {
        if (!isalnum((unsigned char)*p) && *p != '_' && *p != '-') return false;
    }
    return true;
}

static bool safe_relative_path(const char *path)
{
    return path && path[0] != '\0' && path[0] != '/' && !strstr(path, "\\") && !strstr(path, "..");
}

static bool supported_font_path(const char *path)
{
    const char *dot = strrchr(path, '.');
    if (!dot) return false;
    char ext[8];
    size_t len = strlen(dot);
    if (len >= sizeof(ext)) return false;
    for (size_t i = 0; i <= len; i++)
        ext[i] = (char)tolower((unsigned char)dot[i]);
    return strcmp(ext, ".ttf") == 0 || strcmp(ext, ".otf") == 0 || strcmp(ext, ".fnt") == 0 || strcmp(ext, ".bdf") == 0;
}

static const char *skip_ws(const char *p)
{
    while (*p && isspace((unsigned char)*p))
        p++;
    return p;
}

static bool json_parse_string(const char **cursor, char *out, size_t out_size)
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

static bool validate_manifest_fonts(const char *manifest, const char *package_path, char *error, size_t error_size)
{
    const char *p = strstr(manifest, "\"fonts\"");
    if (!p) return true;
    p = strchr(p + strlen("\"fonts\""), ':');
    if (!p) return false;
    p = skip_ws(p + 1);
    if (*p != '{') return false;
    p++;
    p = skip_ws(p);
    if (*p == '}') return true;

    int count = 0;
    while (*p) {
        if (count >= VD_PACKAGE_FONT_MAX) {
            set_error(error, error_size, "Deck App Package declares too many fonts.");
            return false;
        }

        char name[VD_FONT_NAME_MAX];
        char rel_path[VD_PATH_MAX];
        if (!json_parse_string(&p, name, sizeof(name)) || !safe_font_name(name)) {
            set_error(error, error_size, "Deck App Package declares an invalid font name.");
            return false;
        }
        p = skip_ws(p);
        if (*p != ':') return false;
        p++;
        if (!json_parse_string(&p, rel_path, sizeof(rel_path)) || !safe_relative_path(rel_path) ||
            !supported_font_path(rel_path)) {
            set_error(error, error_size, "Deck App Package declares an invalid font path.");
            return false;
        }

        char font_path[VD_PATH_MAX];
        join_path(font_path, sizeof(font_path), package_path, rel_path);
        if (!path_exists(font_path)) {
            set_error(error, error_size, "Deck App Package Font is missing.");
            return false;
        }

        count++;
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

static bool safe_package_name(const char *package_name)
{
    return package_name && package_name[0] != '\0' && has_suffix(package_name, ".vdapp") &&
           !strstr(package_name, "/") && !strstr(package_name, "\\") && !strstr(package_name, "..");
}

bool package_library_validate_package(const char *package_path, const char *package_name, VdPackageInfo *out_info,
                                      char *error, size_t error_size)
{
    if (!safe_package_name(package_name)) {
        set_error(error, error_size, "Invalid Deck App Package Name.");
        return false;
    }
    if (!is_dir(package_path)) {
        set_error(error, error_size, "Deck App Package Directory is missing.");
        return false;
    }

    char manifest_path[VD_PATH_MAX];
    join_path(manifest_path, sizeof(manifest_path), package_path, "manifest.json");
    char *manifest = read_text_file(manifest_path);
    if (!manifest) {
        set_error(error, error_size, "Deck App Package Manifest is missing.");
        return false;
    }

    int schema_version = 0;
    char display_name[VD_DISPLAY_NAME_MAX];
    char version[VD_VERSION_MAX];
    char entry[64];
    bool ok = json_int_value(manifest, "schemaVersion", &schema_version) &&
              json_string_value(manifest, "name", display_name, sizeof(display_name)) &&
              json_string_value(manifest, "version", version, sizeof(version)) &&
              json_string_value(manifest, "entry", entry, sizeof(entry));

    if (!ok || schema_version != 1 || strcmp(entry, "app.js") != 0 || !valid_semverish(version)) {
        free(manifest);
        set_error(error, error_size, "Deck App Package Manifest is invalid.");
        return false;
    }

    char entry_path[VD_PATH_MAX];
    join_path(entry_path, sizeof(entry_path), package_path, entry);
    if (!path_exists(entry_path)) {
        free(manifest);
        set_error(error, error_size, "Deck App Package Entry is missing.");
        return false;
    }

    if (!validate_manifest_fonts(manifest, package_path, error, error_size)) {
        free(manifest);
        if (!error || error_size == 0 || error[0] == '\0')
            set_error(error, error_size, "Deck App Package Manifest fonts are invalid.");
        return false;
    }
    free(manifest);

    if (out_info) {
        memset(out_info, 0, sizeof(*out_info));
        snprintf(out_info->package_name, sizeof(out_info->package_name), "%s", package_name);
        snprintf(out_info->display_name, sizeof(out_info->display_name), "%s", display_name);
        snprintf(out_info->version, sizeof(out_info->version), "%s", version);
        snprintf(out_info->path, sizeof(out_info->path), "%s", package_path);
        out_info->is_active = strcmp(package_name, g_active_name) == 0;
    }
    return true;
}

static bool read_active_state(void)
{
    char state_path[VD_PATH_MAX];
    join_path(state_path, sizeof(state_path), g_root, "active-package.txt");
    char *contents = read_text_file(state_path);
    if (!contents) return false;
    contents[strcspn(contents, "\r\n")] = '\0';
    bool ok = safe_package_name(contents);
    if (ok) snprintf(g_active_name, sizeof(g_active_name), "%s", contents);
    free(contents);
    return ok;
}

static bool write_active_state(const char *package_name)
{
    char state_path[VD_PATH_MAX];
    join_path(state_path, sizeof(state_path), g_root, "active-package.txt");
    return write_text_file(state_path, package_name);
}

static void refresh_active_path(void)
{
    if (g_active_name[0] == '\0') {
        g_active_path[0] = '\0';
        return;
    }
    join_path(g_active_path, sizeof(g_active_path), g_installed_root, g_active_name);
}

static void clear_active_state_file(void)
{
    char state_path[VD_PATH_MAX];
    join_path(state_path, sizeof(state_path), g_root, "active-package.txt");
    (void)remove(state_path);
}

bool package_library_has_active_deck_app(void)
{
    return g_active_name[0] != '\0' && is_dir(g_active_path);
}

static int package_compare(const void *a, const void *b)
{
    const VdPackageInfo *pa = a;
    const VdPackageInfo *pb = b;
    int by_name = strcasecmp(pa->display_name, pb->display_name);
    if (by_name != 0) return by_name;
    return strcmp(pa->package_name, pb->package_name);
}

int package_library_list(VdPackageInfo *items, int max_items)
{
    if (!items || max_items <= 0) return 0;
    DIR *dir = opendir(g_installed_root);
    if (!dir) return 0;

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) && count < max_items) {
        if (entry->d_name[0] == '.' || !has_suffix(entry->d_name, ".vdapp")) continue;
        char path[VD_PATH_MAX];
        join_path(path, sizeof(path), g_installed_root, entry->d_name);
        if (package_library_validate_package(path, entry->d_name, &items[count], NULL, 0)) count++;
    }
    closedir(dir);
    qsort(items, (size_t)count, sizeof(VdPackageInfo), package_compare);
    return count;
}

bool package_library_init(char *error, size_t error_size)
{
    join_path(g_installed_root, sizeof(g_installed_root), g_root, "installed-deck-apps");
    join_path(g_staging_root, sizeof(g_staging_root), g_root, "staging/runtime-upload");

    if (!mkdir_p(g_installed_root) || !mkdir_p(g_staging_root)) {
        set_error(error, error_size, "Could not initialize Installed Deck App Library paths.");
        return false;
    }

    g_active_name[0] = '\0';
    g_active_path[0] = '\0';
    read_active_state();
    refresh_active_path();
    if (g_active_name[0] != '\0' && !is_dir(g_active_path)) {
        clear_active_state_file();
        g_active_name[0] = '\0';
        g_active_path[0] = '\0';
    }

    if (g_active_name[0] == '\0') {
        VdPackageInfo items[VD_PACKAGE_LIST_MAX];
        int count = package_library_list(items, VD_PACKAGE_LIST_MAX);
        if (count > 0 && !package_library_set_active(items[0].package_name, error, error_size)) return false;
    }

    refresh_active_path();
    return true;
}

const char *package_library_root(void)
{
    return g_root;
}
const char *package_library_staging_root(void)
{
    return g_staging_root;
}
const char *package_library_active_package_name(void)
{
    return g_active_name;
}
const char *package_library_active_package_path(void)
{
    return g_active_path;
}

bool package_library_set_active(const char *package_name, char *error, size_t error_size)
{
    if (!safe_package_name(package_name)) {
        set_error(error, error_size, "Invalid Active Deck App package name.");
        return false;
    }
    char path[VD_PATH_MAX];
    join_path(path, sizeof(path), g_installed_root, package_name);
    if (!package_library_validate_package(path, package_name, NULL, error, error_size)) return false;
    if (!write_active_state(package_name)) {
        set_error(error, error_size, "Could not persist Active Deck App.");
        return false;
    }
    snprintf(g_active_name, sizeof(g_active_name), "%s", package_name);
    refresh_active_path();
    return true;
}

bool package_library_remove(const char *package_name, char *error, size_t error_size)
{
    if (strcmp(package_name, g_active_name) == 0) {
        set_error(error, error_size, "Cannot remove the Active Deck App.");
        return false;
    }
    if (!safe_package_name(package_name)) {
        set_error(error, error_size, "Invalid Deck App Package Name.");
        return false;
    }
    char path[VD_PATH_MAX];
    join_path(path, sizeof(path), g_installed_root, package_name);
    if (!remove_tree(path)) {
        set_error(error, error_size, "Could not remove Deck App Package.");
        return false;
    }
    return true;
}

bool package_library_publish_package(const char *source_path, const char *package_name, bool *replaced_active,
                                     VdPackageInfo *out_info, char *error, size_t error_size)
{
    bool had_no_active = (g_active_name[0] == '\0');
    VdPackageInfo info;
    if (!package_library_validate_package(source_path, package_name, &info, error, error_size)) return false;

    char destination[VD_PATH_MAX];
    char backup[VD_PATH_MAX];
    join_path(destination, sizeof(destination), g_installed_root, package_name);
    snprintf(backup, sizeof(backup), "%s.replacing", destination);
    remove_tree(backup);

    if (is_dir(destination) && rename(destination, backup) != 0) {
        set_error(error, error_size, "Could not prepare existing package for replacement.");
        return false;
    }

    if (rename(source_path, destination) != 0) {
        if (is_dir(backup)) rename(backup, destination);
        set_error(error, error_size, "Could not publish Deck App Package.");
        return false;
    }

    remove_tree(backup);
    if (had_no_active) {
        if (!package_library_set_active(package_name, error, error_size)) return false;
        if (replaced_active) *replaced_active = true;
    } else if (replaced_active) {
        *replaced_active = strcmp(package_name, g_active_name) == 0;
    }
    if (out_info) package_library_validate_package(destination, package_name, out_info, NULL, 0);
    refresh_active_path();
    return true;
}

void package_library_clear_staging(void)
{
    remove_tree(g_staging_root);
    mkdir_p(g_staging_root);
}
