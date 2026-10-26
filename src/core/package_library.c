#include "package_library.h"

#include "core/deck_app_storage.h"
#include "core/package_manifest.h"
#include "core/util.h"
#include "platform/thread.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#ifdef __vita__
#define VD_DATA_ROOT "ux0:data/vitadeck"
#else
#define VD_DATA_ROOT "vitadeck-data"
#endif

// The roots are written only by package_library_init, before other threads start.
static char g_root[VD_PATH_MAX] = VD_DATA_ROOT;
static char g_installed_root[VD_PATH_MAX];
static char g_staging_root[VD_PATH_MAX];

// Guards the active package state, the cached package list, and changes to installed package directories.
static vd_mutex *g_mutex;
static char g_active_name[VD_PACKAGE_NAME_MAX];
static char g_active_path[VD_PATH_MAX];
// Scanned on the first list after an invalidation; is_active is filled in when copied out.
static VdPackageInfo g_list[VD_PACKAGE_LIST_MAX];
static int g_list_count;
static bool g_list_valid;

static bool read_package_info(const char *package_path, const char *package_name, VdPackageInfo *out_info, char *error,
                              size_t error_size)
{
    VdPackageManifest manifest;
    if (!package_manifest_read(package_path, package_name, &manifest, error, error_size)) return false;

    if (out_info) {
        memset(out_info, 0, sizeof(*out_info));
        snprintf(out_info->package_name, sizeof(out_info->package_name), "%s", manifest.package_name);
        snprintf(out_info->display_name, sizeof(out_info->display_name), "%s", manifest.display_name);
        snprintf(out_info->version, sizeof(out_info->version), "%s", manifest.version);
        snprintf(out_info->path, sizeof(out_info->path), "%s", package_path);
    }
    return true;
}

bool package_library_validate_package(const char *package_path, const char *package_name, VdPackageInfo *out_info,
                                      char *error, size_t error_size)
{
    if (!read_package_info(package_path, package_name, out_info, error, error_size)) return false;
    if (out_info) {
        vd_mutex_lock(g_mutex);
        out_info->is_active = strcmp(package_name, g_active_name) == 0;
        vd_mutex_unlock(g_mutex);
    }
    return true;
}

static bool read_active_state(void)
{
    char state_path[VD_PATH_MAX];
    vd_join_path(state_path, sizeof(state_path), g_root, "active-package.txt");
    char *contents = vd_read_file(state_path, NULL);
    if (!contents) return false;
    contents[strcspn(contents, "\r\n")] = '\0';
    bool ok = vd_safe_package_name(contents);
    if (ok) snprintf(g_active_name, sizeof(g_active_name), "%s", contents);
    free(contents);
    return ok;
}

static bool write_active_state(const char *package_name)
{
    char state_path[VD_PATH_MAX];
    vd_join_path(state_path, sizeof(state_path), g_root, "active-package.txt");
    return vd_write_text_file(state_path, package_name);
}

static void refresh_active_path(void)
{
    if (g_active_name[0] == '\0') {
        g_active_path[0] = '\0';
        return;
    }
    vd_join_path(g_active_path, sizeof(g_active_path), g_installed_root, g_active_name);
}

static void clear_active_state_file(void)
{
    char state_path[VD_PATH_MAX];
    vd_join_path(state_path, sizeof(state_path), g_root, "active-package.txt");
    (void)remove(state_path);
}

static void copy_locked(char *out, size_t out_size, const char *value)
{
    vd_mutex_lock(g_mutex);
    snprintf(out, out_size, "%s", value);
    vd_mutex_unlock(g_mutex);
}

bool package_library_has_active_deck_app(void)
{
    char path[VD_PATH_MAX];
    copy_locked(path, sizeof(path), g_active_path);
    return path[0] != '\0' && vd_is_dir(path);
}

static int package_compare(const void *a, const void *b)
{
    const VdPackageInfo *pa = a;
    const VdPackageInfo *pb = b;
    int by_name = strcasecmp(pa->display_name, pb->display_name);
    if (by_name != 0) return by_name;
    return strcmp(pa->package_name, pb->package_name);
}

static void scan_list_locked(void)
{
    g_list_count = 0;
    g_list_valid = true;
    DIR *dir = opendir(g_installed_root);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) && g_list_count < VD_PACKAGE_LIST_MAX) {
        if (entry->d_name[0] == '.' || !vd_has_suffix(entry->d_name, ".vdapp")) continue;
        char path[VD_PATH_MAX];
        vd_join_path(path, sizeof(path), g_installed_root, entry->d_name);
        if (!read_package_info(path, entry->d_name, &g_list[g_list_count], NULL, 0)) continue;
        g_list_count++;
    }
    closedir(dir);
    qsort(g_list, (size_t)g_list_count, sizeof(VdPackageInfo), package_compare);
}

int package_library_list(VdPackageInfo *items, int max_items)
{
    if (!items || max_items <= 0) return 0;
    vd_mutex_lock(g_mutex);
    if (!g_list_valid) scan_list_locked();
    int count = g_list_count < max_items ? g_list_count : max_items;
    for (int i = 0; i < count; i++) {
        items[i] = g_list[i];
        items[i].is_active = strcmp(items[i].package_name, g_active_name) == 0;
    }
    vd_mutex_unlock(g_mutex);
    return count;
}

void package_library_refresh(void)
{
    vd_mutex_lock(g_mutex);
    g_list_valid = false;
    vd_mutex_unlock(g_mutex);
}

bool package_library_init(char *error, size_t error_size)
{
    if (!g_mutex) g_mutex = vd_mutex_create();
    if (!g_mutex) {
        vd_set_error(error, error_size, "Could not initialize Installed Deck App Library lock.");
        return false;
    }

    const char *env_root = getenv("VITADECK_DATA_ROOT");
    if (env_root && env_root[0] != '\0') {
        snprintf(g_root, sizeof(g_root), "%s", env_root);
    }

    vd_join_path(g_installed_root, sizeof(g_installed_root), g_root, "installed-deck-apps");
    vd_join_path(g_staging_root, sizeof(g_staging_root), g_root, "staging/runtime-upload");

    if (!vd_mkdir_p(g_installed_root) || !vd_mkdir_p(g_staging_root)) {
        vd_set_error(error, error_size, "Could not initialize Installed Deck App Library paths.");
        return false;
    }

    vd_mutex_lock(g_mutex);
    g_active_name[0] = '\0';
    g_active_path[0] = '\0';
    read_active_state();
    refresh_active_path();
    if (g_active_name[0] != '\0' && !vd_is_dir(g_active_path)) {
        clear_active_state_file();
        g_active_name[0] = '\0';
        g_active_path[0] = '\0';
    }
    bool has_active = g_active_name[0] != '\0';
    vd_mutex_unlock(g_mutex);

    if (!has_active) {
        VdPackageInfo items[VD_PACKAGE_LIST_MAX];
        int count = package_library_list(items, VD_PACKAGE_LIST_MAX);
        if (count > 0 && !package_library_set_active(items[0].package_name, error, error_size)) return false;
    }
    return true;
}

const char *package_library_data_root(void)
{
    return g_root;
}
const char *package_library_staging_root(void)
{
    return g_staging_root;
}
void package_library_active_package_name(char *out, size_t out_size)
{
    copy_locked(out, out_size, g_active_name);
}
void package_library_active_package_path(char *out, size_t out_size)
{
    copy_locked(out, out_size, g_active_path);
}

static bool set_active_locked(const char *package_name, bool persist, char *error, size_t error_size)
{
    if (persist && !write_active_state(package_name)) {
        vd_set_error(error, error_size, "Could not persist Active Deck App.");
        return false;
    }
    snprintf(g_active_name, sizeof(g_active_name), "%s", package_name);
    refresh_active_path();
    return true;
}

static bool set_active(const char *package_name, bool persist, char *error, size_t error_size)
{
    if (!vd_safe_package_name(package_name)) {
        vd_set_error(error, error_size, "Invalid Active Deck App package name.");
        return false;
    }
    char path[VD_PATH_MAX];
    vd_join_path(path, sizeof(path), g_installed_root, package_name);
    vd_mutex_lock(g_mutex);
    bool ok = read_package_info(path, package_name, NULL, error, error_size) &&
              set_active_locked(package_name, persist, error, error_size);
    vd_mutex_unlock(g_mutex);
    return ok;
}

bool package_library_set_active(const char *package_name, char *error, size_t error_size)
{
    return set_active(package_name, true, error, error_size);
}

bool package_library_set_active_transient(const char *package_name, char *error, size_t error_size)
{
    return set_active(package_name, false, error, error_size);
}

bool package_library_remove(const char *package_name, char *error, size_t error_size)
{
    if (!vd_safe_package_name(package_name)) {
        vd_set_error(error, error_size, "Invalid Deck App Package Name.");
        return false;
    }
    char path[VD_PATH_MAX];
    vd_join_path(path, sizeof(path), g_installed_root, package_name);

    const char *failure = NULL;
    vd_mutex_lock(g_mutex);
    if (strcmp(package_name, g_active_name) == 0) {
        failure = "Cannot remove the Active Deck App.";
    } else {
        g_list_valid = false;
        if (!vd_remove_tree(path)) {
            failure = "Could not remove Deck App Package.";
        } else if (!deck_app_storage_remove(g_root, package_name)) {
            failure = "Could not remove Deck App Storage.";
        }
    }
    vd_mutex_unlock(g_mutex);

    if (failure) {
        vd_set_error(error, error_size, failure);
        return false;
    }
    return true;
}

bool package_library_publish_package(const char *source_path, const char *package_name, bool *replaced_active,
                                     VdPackageInfo *out_info, char *error, size_t error_size)
{
    if (!read_package_info(source_path, package_name, NULL, error, error_size)) return false;

    char destination[VD_PATH_MAX];
    char backup[VD_PATH_MAX];
    vd_join_path(destination, sizeof(destination), g_installed_root, package_name);
    // backup is destination plus a suffix, so it fitting means destination fits too.
    int backup_len = snprintf(backup, sizeof(backup), "%s.replacing", destination);
    if (backup_len < 0 || (size_t)backup_len >= sizeof(backup)) {
        vd_set_error(error, error_size, "Deck App Package path is too long.");
        return false;
    }
    vd_remove_tree(backup);

    vd_mutex_lock(g_mutex);
    g_list_valid = false;
    if (vd_is_dir(destination) && rename(destination, backup) != 0) {
        vd_mutex_unlock(g_mutex);
        vd_set_error(error, error_size, "Could not prepare existing package for replacement.");
        return false;
    }

    if (rename(source_path, destination) != 0) {
        if (vd_is_dir(backup)) rename(backup, destination);
        vd_mutex_unlock(g_mutex);
        vd_set_error(error, error_size, "Could not publish Deck App Package.");
        return false;
    }

    bool ok = g_active_name[0] != '\0' || set_active_locked(package_name, true, error, error_size);
    if (replaced_active) *replaced_active = strcmp(package_name, g_active_name) == 0;
    vd_mutex_unlock(g_mutex);

    vd_remove_tree(backup);
    if (!ok) return false;
    if (out_info) package_library_validate_package(destination, package_name, out_info, NULL, 0);
    return true;
}

void package_library_clear_staging(void)
{
    vd_remove_tree(g_staging_root);
    vd_mkdir_p(g_staging_root);
}
