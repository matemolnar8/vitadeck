#include "settings.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/package_library.h"

static void host_control_path(char *out, size_t out_size)
{
    snprintf(out, out_size, "%s/host-control-url.txt", package_library_root());
}

static char *read_text_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long len = ftell(f);
    if (len < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    char *buffer = malloc((size_t)len + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    size_t read_len = fread(buffer, 1, (size_t)len, f);
    buffer[read_len] = '\0';
    fclose(f);
    return buffer;
}

static bool write_text_file(const char *path, const char *contents)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = fwrite(contents, 1, strlen(contents), f) == strlen(contents);
    fclose(f);
    return ok;
}

static bool valid_host_control_url(const char *url)
{
    if (!url || url[0] == '\0') return true;
    if (strlen(url) >= VD_HOST_CONTROL_URL_MAX) return false;
    if (strncmp(url, "http://", 7) != 0) return false;
    for (const char *p = url; *p; p++) {
        if (isspace((unsigned char)*p)) return false;
    }
    return true;
}

bool settings_get_host_control_url(char *out, size_t out_size)
{
    if (!out || out_size == 0) return false;
    out[0] = '\0';

    char path[VD_PATH_MAX];
    host_control_path(path, sizeof(path));
    char *contents = read_text_file(path);
    if (!contents) return false;
    contents[strcspn(contents, "\r\n")] = '\0';
    if (valid_host_control_url(contents)) snprintf(out, out_size, "%s", contents);
    free(contents);
    return out[0] != '\0';
}

bool settings_set_host_control_url(const char *url, char *error, size_t error_size)
{
    if (!valid_host_control_url(url)) {
        if (error && error_size > 0) snprintf(error, error_size, "Host Control address must start with http:// and contain no spaces.");
        return false;
    }

    char path[VD_PATH_MAX];
    host_control_path(path, sizeof(path));
    if (!write_text_file(path, url ? url : "")) {
        if (error && error_size > 0) snprintf(error, error_size, "Could not save Host Control address.");
        return false;
    }
    return true;
}
