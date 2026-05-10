#ifndef PACKAGE_LIBRARY_H
#define PACKAGE_LIBRARY_H

#include <stdbool.h>
#include <stddef.h>

#define VD_PACKAGE_NAME_MAX 128
#define VD_DISPLAY_NAME_MAX 128
#define VD_VERSION_MAX 64
#define VD_PATH_MAX 512
#define VD_PACKAGE_LIST_MAX 64

typedef struct {
    char package_name[VD_PACKAGE_NAME_MAX];
    char display_name[VD_DISPLAY_NAME_MAX];
    char version[VD_VERSION_MAX];
    char path[VD_PATH_MAX];
    bool is_active;
} VdPackageInfo;

bool package_library_init(char *error, size_t error_size);
bool package_library_has_active_deck_app(void);
const char *package_library_root(void);
const char *package_library_staging_root(void);
const char *package_library_active_package_name(void);
const char *package_library_active_package_path(void);

int package_library_list(VdPackageInfo *items, int max_items);
bool package_library_set_active(const char *package_name, char *error, size_t error_size);
bool package_library_remove(const char *package_name, char *error, size_t error_size);
bool package_library_validate_package(const char *package_path, const char *package_name, VdPackageInfo *out_info, char *error, size_t error_size);
bool package_library_publish_package(const char *source_path, const char *package_name, bool *replaced_active, VdPackageInfo *out_info, char *error, size_t error_size);
void package_library_clear_staging(void);

#endif /* PACKAGE_LIBRARY_H */
