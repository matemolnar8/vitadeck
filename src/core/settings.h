#ifndef VD_SETTINGS_H
#define VD_SETTINGS_H

#include <stdbool.h>
#include <stddef.h>

#define VD_HOST_CONTROL_URL_MAX 128

bool settings_get_host_control_url(char *out, size_t out_size);
bool settings_set_host_control_url(const char *url, char *error, size_t error_size);

#endif /* VD_SETTINGS_H */
