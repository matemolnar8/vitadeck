#include "deck_app_storage.h"

#include "core/package_library.h"
#include "core/util.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    char dir[VD_PATH_MAX];
    char file[VD_PATH_MAX];
} StoragePaths;

static bool storage_paths(const char *data_root, const char *package_name, StoragePaths *paths)
{
    if (!vd_safe_package_name(package_name)) return false;
    char data_dir[VD_PATH_MAX];
    return vd_join_path(data_dir, sizeof(data_dir), data_root, "deck-app-data") &&
           vd_join_path(paths->dir, sizeof(paths->dir), data_dir, package_name) &&
           vd_join_path(paths->file, sizeof(paths->file), paths->dir, "storage.json");
}

char *deck_app_storage_read(const char *data_root, const char *package_name, size_t *out_len)
{
    StoragePaths paths;
    if (!storage_paths(data_root, package_name, &paths)) return NULL;
    bool found = false;
    char *json = vd_read_replaced_file(paths.file, out_len, &found);
    if (found) return json;

    char *empty = malloc(3);
    if (!empty) return NULL;
    memcpy(empty, "{}", 3);
    if (out_len) *out_len = 2;
    return empty;
}

static bool fail(char *error, size_t error_size, const char *message)
{
    vd_set_error(error, error_size, message);
    return false;
}

bool deck_app_storage_write(const char *data_root, const char *package_name, const char *json, size_t len, char *error,
                            size_t error_size)
{
    if (len > VD_DECK_APP_STORAGE_MAX) return fail(error, error_size, "Deck App Storage is limited to 1 MiB.");
    StoragePaths paths;
    if (!storage_paths(data_root, package_name, &paths)) {
        return fail(error, error_size, "Invalid Deck App Storage path.");
    }
    if (!vd_mkdir_p(paths.dir)) return fail(error, error_size, "Could not create Deck App Storage directory.");
    if (!vd_replace_file(paths.file, json, len)) return fail(error, error_size, "Could not write Deck App Storage.");
    return true;
}

bool deck_app_storage_remove(const char *data_root, const char *package_name)
{
    StoragePaths paths;
    return storage_paths(data_root, package_name, &paths) && vd_remove_tree(paths.dir);
}
