#ifndef PACKAGE_MANIFEST_H
#define PACKAGE_MANIFEST_H

#include <stdbool.h>
#include <stddef.h>

#include "core/package_library.h"

#define VD_PACKAGE_ENTRY_MAX 64
#define VD_PACKAGE_ASSET_NAME_MAX 64
#define VD_PACKAGE_FONT_MAX 32
#define VD_PACKAGE_IMAGE_MAX 32

typedef struct {
    char name[VD_PACKAGE_ASSET_NAME_MAX];
    char path[VD_PATH_MAX];
} VdPackageAsset;

typedef struct {
    char package_name[VD_PACKAGE_NAME_MAX];
    char display_name[VD_DISPLAY_NAME_MAX];
    char version[VD_VERSION_MAX];
    char entry[VD_PACKAGE_ENTRY_MAX];
    VdPackageAsset fonts[VD_PACKAGE_FONT_MAX];
    int font_count;
    VdPackageAsset images[VD_PACKAGE_IMAGE_MAX];
    int image_count;
} VdPackageManifest;

bool package_manifest_read(const char *package_path, const char *package_name, VdPackageManifest *out_manifest,
                           char *error, size_t error_size);

#endif /* PACKAGE_MANIFEST_H */
