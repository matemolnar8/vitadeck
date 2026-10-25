#include "core/util.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/package_library.h"

void vd_set_error(char *error, size_t error_size, const char *message)
{
    if (!error || error_size == 0) return;
    snprintf(error, error_size, "%s", message);
}

bool vd_has_suffix(const char *value, const char *suffix)
{
    size_t value_len = strlen(value);
    size_t suffix_len = strlen(suffix);
    return value_len >= suffix_len && strcmp(value + value_len - suffix_len, suffix) == 0;
}

bool vd_join_path(char *out, size_t out_size, const char *a, const char *b)
{
    int len = snprintf(out, out_size, "%s/%s", a, b);
    return len >= 0 && (size_t)len < out_size;
}

bool vd_is_dir(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

bool vd_mkdir_p(const char *path)
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

bool vd_mkdir_parent(const char *path)
{
    char parent[VD_PATH_MAX];
    snprintf(parent, sizeof(parent), "%s", path);
    char *slash = strrchr(parent, '/');
    if (!slash) return true;
    *slash = '\0';
    return vd_mkdir_p(parent);
}

bool vd_remove_tree(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir) return unlink(path) == 0 || errno == ENOENT;

    struct dirent *entry;
    bool ok = true;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char child[VD_PATH_MAX];
        vd_join_path(child, sizeof(child), path, entry->d_name);
        if (!vd_remove_tree(child)) {
            ok = false;
            break;
        }
    }
    closedir(dir);
    return ok && (rmdir(path) == 0 || errno == ENOENT);
}

char *vd_read_file(const char *path, size_t *out_len)
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

bool vd_write_text_file(const char *path, const char *contents)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = fwrite(contents, 1, strlen(contents), f) == strlen(contents);
    fclose(f);
    return ok;
}

bool vd_safe_package_name(const char *package_name)
{
    return package_name && package_name[0] != '\0' && strlen(package_name) < VD_PACKAGE_NAME_MAX &&
           vd_has_suffix(package_name, ".vdapp") && !strstr(package_name, "/") && !strstr(package_name, "\\") &&
           !strstr(package_name, "..");
}
