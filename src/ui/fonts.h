#ifndef FONTS_H
#define FONTS_H

#include <stdbool.h>
#include <stddef.h>
#include <raylib.h>
#include "core/package_manifest.h"
#include "glyph_cache.h"

#define VD_FONT_DEFAULT_NAME "default"

#define VD_TEXT_FONT_SIZE_DEFAULT 30
#define VD_BUTTON_FONT_SIZE_DEFAULT 20
#define VD_TEXT_INPUT_FONT_SIZE_DEFAULT 20

// TrueType and OpenType fonts draw through the glyph cache; .fnt and .bdf fonts, whose outline is NULL, through
// raylib's own atlas, without fallback.
typedef struct {
    VdGlyphFont *outline;
    Font bitmap;
} VdFont;

void font_registry_init(void);
// Replaces the loaded package assets with the manifest's; a NULL manifest only unloads them.
bool font_registry_load_package(const char *package_path, const VdPackageManifest *manifest, char *error,
                                size_t error_size);
void font_registry_shutdown(void);
// Unknown names and "default" give the Runtime Default Font.
const VdFont *font_registry_get(const char *name);

#endif /* FONTS_H */
