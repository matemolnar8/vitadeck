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
// Replaces the file so that a crash at any point leaves either the old or the new contents for
// vd_read_replaced_file. Works on the Vita, whose rename does not replace an existing file.
bool vd_replace_file(const char *path, const char *data, size_t len);
// Reads a file vd_replace_file wrote, or the backup a crashed replace left. *found is false when neither exists.
char *vd_read_replaced_file(const char *path, size_t *out_len, bool *found);
// Deletes a file vd_replace_file wrote, with any leftover temp or backup file.
bool vd_remove_replaced_file(const char *path);
bool vd_safe_package_name(const char *package_name);

#endif /* VD_UTIL_H */
