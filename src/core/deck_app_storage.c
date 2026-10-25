#include "deck_app_storage.h"

#include "core/package_library.h"
#include "core/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    char dir[VD_PATH_MAX];
    char file[VD_PATH_MAX];
    char temp[VD_PATH_MAX];
    char backup[VD_PATH_MAX];
} StoragePaths;

static bool storage_paths(const char *data_root, const char *package_name, StoragePaths *paths)
{
    if (!vd_safe_package_name(package_name)) return false;
    char data_dir[VD_PATH_MAX];
    if (!vd_join_path(data_dir, sizeof(data_dir), data_root, "deck-app-data") ||
        !vd_join_path(paths->dir, sizeof(paths->dir), data_dir, package_name) ||
        !vd_join_path(paths->file, sizeof(paths->file), paths->dir, "storage.json")) {
        return false;
    }
    // temp and backup are file plus a suffix, so they fit whenever it does.
    int temp_len = snprintf(paths->temp, sizeof(paths->temp), "%s.tmp", paths->file);
    int backup_len = snprintf(paths->backup, sizeof(paths->backup), "%s.bak", paths->file);
    return temp_len > 0 && (size_t)temp_len < sizeof(paths->temp) && backup_len > 0 &&
           (size_t)backup_len < sizeof(paths->backup);
}

static bool file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

char *deck_app_storage_read(const char *data_root, const char *package_name, size_t *out_len)
{
    StoragePaths paths;
    if (!storage_paths(data_root, package_name, &paths)) return NULL;
    // The backup only outlives a write that crashed before the new file was in place.
    const char *source = file_exists(paths.file) ? paths.file : file_exists(paths.backup) ? paths.backup : NULL;
    if (source) return vd_read_file(source, out_len);

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

    FILE *f = fopen(paths.temp, "wb");
    if (!f) return fail(error, error_size, "Could not write Deck App Storage.");
    bool written = fwrite(json, 1, len, f) == len;
    if (fclose(f) != 0 || !written) {
        remove(paths.temp);
        return fail(error, error_size, "Could not write Deck App Storage.");
    }

    // The Vita's rename does not replace an existing file, so the old store moves aside first and
    // stays readable as the backup until the new one is in place.
    if (file_exists(paths.file)) {
        remove(paths.backup);
        if (rename(paths.file, paths.backup) != 0) {
            remove(paths.temp);
            return fail(error, error_size, "Could not replace Deck App Storage.");
        }
    }
    if (rename(paths.temp, paths.file) != 0) {
        rename(paths.backup, paths.file);
        remove(paths.temp);
        return fail(error, error_size, "Could not replace Deck App Storage.");
    }
    remove(paths.backup);
    return true;
}

bool deck_app_storage_remove(const char *data_root, const char *package_name)
{
    StoragePaths paths;
    return storage_paths(data_root, package_name, &paths) && vd_remove_tree(paths.dir);
}
