#ifndef VD_UTIL_H
#define VD_UTIL_H

#include <stdbool.h>
#include <stddef.h>

void vd_set_error(char *error, size_t error_size, const char *message);
bool vd_has_suffix(const char *value, const char *suffix);
// Writes "a/b" into out; returns false when it does not fit.
bool vd_join_path(char *out, size_t out_size, const char *a, const char *b);
bool vd_is_dir(const char *path);
bool vd_mkdir_p(const char *path);
bool vd_mkdir_parent(const char *path);
bool vd_remove_tree(const char *path);
// Returns a malloc'd, NUL-terminated copy of the file, or NULL. out_len may be NULL.
char *vd_read_file(const char *path, size_t *out_len);
bool vd_write_text_file(const char *path, const char *contents);
bool vd_safe_package_name(const char *package_name);

#endif /* VD_UTIL_H */
