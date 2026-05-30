#ifndef VD_UPLOAD_ARCHIVE_H
#define VD_UPLOAD_ARCHIVE_H

#include <stdbool.h>
#include <stddef.h>
#include "arena.h"
#include "core/package_library.h"

#define VD_UPLOAD_MAX_BYTES (16 * 1024 * 1024)
#define VD_UPLOAD_MAX_UNPACKED_BYTES (64 * 1024 * 1024)
#define VD_UPLOAD_MAX_ENTRIES 256

typedef struct {
    char package_name[VD_PACKAGE_NAME_MAX];
    char package_path[VD_PATH_MAX];
} VdArchiveExtractResult;

bool upload_archive_extract(Arena *arena, const char *zip_path, VdArchiveExtractResult *result, char *error, size_t error_size);

#endif /* VD_UPLOAD_ARCHIVE_H */
