#include "images.h"

#include <stdio.h>
#include <string.h>

#include "core/util.h"

#define VD_IMAGE_NAME_MAX 64
#define VD_IMAGE_MAX 32

_Static_assert(VD_IMAGE_MAX >= VD_PACKAGE_IMAGE_MAX, "image registry must hold every manifest image");

typedef struct {
    char name[VD_IMAGE_NAME_MAX];
    Texture2D texture;
} VdLoadedImage;

static VdLoadedImage g_package_images[VD_IMAGE_MAX];
static int g_package_image_count = 0;

static void unload_package_images(void)
{
    for (int i = 0; i < g_package_image_count; i++) {
        if (IsTextureValid(g_package_images[i].texture)) {
            UnloadTexture(g_package_images[i].texture);
        }
    }
    g_package_image_count = 0;
}

static bool load_texture_file(const char *path, Texture2D *out, char *error, size_t error_size)
{
    Texture2D texture = LoadTexture(path);
    if (!IsTextureValid(texture)) {
        vd_set_error(error, error_size, "Could not load image file.");
        return false;
    }
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    *out = texture;
    return true;
}

bool image_registry_load_package(const char *package_path, const VdPackageManifest *manifest, char *error,
                                 size_t error_size)
{
    unload_package_images();
    if (!manifest) return true;

    for (int i = 0; i < manifest->image_count; i++) {
        char image_path[VD_PATH_MAX];
        vd_join_path(image_path, sizeof(image_path), package_path, manifest->images[i].path);
        Texture2D texture = {0};
        if (!load_texture_file(image_path, &texture, error, error_size)) {
            unload_package_images();
            return false;
        }
        snprintf(g_package_images[g_package_image_count].name, sizeof(g_package_images[g_package_image_count].name),
                 "%s", manifest->images[i].name);
        g_package_images[g_package_image_count].texture = texture;
        g_package_image_count++;
    }

    return true;
}

void image_registry_shutdown(void)
{
    unload_package_images();
}

Texture2D image_registry_get(const char *name)
{
    if (!name || name[0] == '\0') return (Texture2D){0};
    for (int i = 0; i < g_package_image_count; i++) {
        if (strcmp(g_package_images[i].name, name) == 0) return g_package_images[i].texture;
    }
    return (Texture2D){0};
}

bool image_resolve_layout(const char *name, int req_width, int req_height, int *out_width, int *out_height)
{
    if (!out_width || !out_height) return false;

    Texture2D tex = image_registry_get(name);
    if (!IsTextureValid(tex) || tex.width <= 0 || tex.height <= 0) return false;

    if (req_width > 0 && req_height > 0) {
        *out_width = req_width;
        *out_height = req_height;
        return true;
    }
    if (req_width > 0) {
        *out_width = req_width;
        *out_height = (int)(req_width * (float)tex.height / (float)tex.width + 0.5f);
        if (*out_height < 1) *out_height = 1;
        return true;
    }
    if (req_height > 0) {
        *out_height = req_height;
        *out_width = (int)(req_height * (float)tex.width / (float)tex.height + 0.5f);
        if (*out_width < 1) *out_width = 1;
        return true;
    }

    return false;
}
