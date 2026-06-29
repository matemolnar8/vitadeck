#ifndef IMAGES_H
#define IMAGES_H

#include <stdbool.h>
#include <stddef.h>
#include <raylib.h>

bool image_registry_load_package(const char *package_path, char *error, size_t error_size);
void image_registry_shutdown(void);
Texture2D image_registry_get(const char *name);
bool image_resolve_layout(const char *name, int req_width, int req_height, int *out_width, int *out_height);

#endif /* IMAGES_H */
