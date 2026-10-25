#ifndef DECK_APP_STORAGE_H
#define DECK_APP_STORAGE_H

#include <stdbool.h>
#include <stddef.h>

#define VD_DECK_APP_STORAGE_MAX (1024 * 1024)

// Returns the stored JSON as a malloc'd string, "{}" when nothing is stored, or NULL when it cannot be read.
char *deck_app_storage_read(const char *data_root, const char *package_name, size_t *out_len);
// Replaces the stored JSON; a crash at any point leaves either the old or the new contents readable.
bool deck_app_storage_write(const char *data_root, const char *package_name, const char *json, size_t len, char *error,
                            size_t error_size);
bool deck_app_storage_remove(const char *data_root, const char *package_name);

#endif /* DECK_APP_STORAGE_H */
