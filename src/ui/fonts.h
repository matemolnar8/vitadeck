#ifndef FONTS_H
#define FONTS_H

#include <stdbool.h>
#include <stddef.h>
#include <raylib.h>
#include "core/package_manifest.h"

#define VD_FONT_DEFAULT_NAME "default"

bool font_registry_init(char *error, size_t error_size);
// Replaces the loaded package assets with the manifest's; a NULL manifest only unloads them.
bool font_registry_load_package(const char *package_path, const VdPackageManifest *manifest, char *error,
                                size_t error_size);
void font_registry_shutdown(void);
Font font_registry_get(const char *name);
Font font_registry_default(void);

#endif /* FONTS_H */
