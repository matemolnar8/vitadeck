#include "upload/archive.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

static void set_error(char *error, size_t error_size, const char *message)
{
    if (!error || error_size == 0) return;
    snprintf(error, error_size, "%s", message);
}

static uint16_t le16(const unsigned char *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t le32(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool has_suffix(const char *value, const char *suffix)
{
    size_t value_len = strlen(value);
    size_t suffix_len = strlen(suffix);
    return value_len >= suffix_len && strcmp(value + value_len - suffix_len, suffix) == 0;
}

static void join_path(char *out, size_t out_size, const char *a, const char *b)
{
    snprintf(out, out_size, "%s/%s", a, b);
}

static bool mkdir_p(const char *path)
{
    char tmp[VD_PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len == 0) return false;
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';
    for (char *p = tmp + 1; *p; p++) {
        if (*p != '/') continue;
        *p = '\0';
        if (mkdir(tmp, 0777) != 0 && errno != EEXIST) return false;
        *p = '/';
    }
    return mkdir(tmp, 0777) == 0 || errno == EEXIST;
}

static bool mkdir_parent(const char *path)
{
    char parent[VD_PATH_MAX];
    snprintf(parent, sizeof(parent), "%s", path);
    char *slash = strrchr(parent, '/');
    if (!slash) return true;
    *slash = '\0';
    return mkdir_p(parent);
}

static bool read_file(const char *path, unsigned char **out_data, size_t *out_size)
{
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    if (len < 0 || len > VD_UPLOAD_MAX_BYTES) {
        fclose(f);
        return false;
    }
    unsigned char *data = malloc((size_t)len);
    if (!data) {
        fclose(f);
        return false;
    }
    size_t read_len = fread(data, 1, (size_t)len, f);
    fclose(f);
    if (read_len != (size_t)len) {
        free(data);
        return false;
    }
    *out_data = data;
    *out_size = read_len;
    return true;
}

static bool safe_zip_path(const char *name)
{
    if (!name || name[0] == '\0' || name[0] == '/') return false;
    if (strstr(name, "..")) return false;
    if (strchr(name, '\\')) return false;
    return true;
}

static bool is_macos_metadata_path(const char *name)
{
    if (strncmp(name, "__MACOSX/", 9) == 0) return true;
    const char *base = strrchr(name, '/');
    base = base ? base + 1 : name;
    return strcmp(base, ".DS_Store") == 0 || strncmp(base, "._", 2) == 0;
}

static bool top_level_name(const char *entry_name, char *out, size_t out_size)
{
    const char *slash = strchr(entry_name, '/');
    size_t len = slash ? (size_t)(slash - entry_name) : strlen(entry_name);
    if (len == 0 || len + 1 > out_size) return false;
    memcpy(out, entry_name, len);
    out[len] = '\0';
    return true;
}

static bool write_stored(const unsigned char *data, size_t size, const char *out_path)
{
    if (!mkdir_parent(out_path)) return false;
    FILE *f = fopen(out_path, "wb");
    if (!f) return false;
    bool ok = fwrite(data, 1, size, f) == size;
    fclose(f);
    return ok;
}

static bool write_deflated(const unsigned char *data, size_t compressed_size, size_t uncompressed_size, const char *out_path)
{
    unsigned char *out = malloc(uncompressed_size);
    if (!out) return false;

    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.next_in = (Bytef *)data;
    stream.avail_in = (uInt)compressed_size;
    stream.next_out = out;
    stream.avail_out = (uInt)uncompressed_size;

    bool ok = inflateInit2(&stream, -MAX_WBITS) == Z_OK;
    if (ok) {
        int ret = inflate(&stream, Z_FINISH);
        ok = ret == Z_STREAM_END && stream.total_out == uncompressed_size;
    }
    inflateEnd(&stream);

    if (ok) ok = write_stored(out, uncompressed_size, out_path);
    free(out);
    return ok;
}

bool upload_archive_extract(const char *zip_path, VdArchiveExtractResult *result, char *error, size_t error_size)
{
    memset(result, 0, sizeof(*result));

    unsigned char *zip = NULL;
    size_t zip_size = 0;
    if (!read_file(zip_path, &zip, &zip_size)) {
        set_error(error, error_size, "Archive is missing or exceeds upload size limit.");
        return false;
    }

    size_t eocd_pos = (size_t)-1;
    size_t search_start = zip_size > 66000 ? zip_size - 66000 : 0;
    for (size_t i = zip_size >= 22 ? zip_size - 22 : 0; i >= search_start && i < zip_size; i--) {
        if (le32(zip + i) == 0x06054b50) {
            eocd_pos = i;
            break;
        }
        if (i == 0) break;
    }
    if (eocd_pos == (size_t)-1) {
        free(zip);
        set_error(error, error_size, "Archive is not a supported zip file.");
        return false;
    }

    unsigned int entry_count = le16(zip + eocd_pos + 10);
    uint32_t central_size = le32(zip + eocd_pos + 12);
    uint32_t central_offset = le32(zip + eocd_pos + 16);
    if (entry_count == 0 || entry_count > VD_UPLOAD_MAX_ENTRIES || central_offset + central_size > zip_size) {
        free(zip);
        set_error(error, error_size, "Archive entry count or directory is invalid.");
        return false;
    }

    package_library_clear_staging();

    char top_name[VD_PACKAGE_NAME_MAX] = "";
    size_t total_unpacked = 0;
    size_t pos = central_offset;
    for (unsigned int i = 0; i < entry_count; i++) {
        if (pos + 46 > zip_size || le32(zip + pos) != 0x02014b50) {
            free(zip);
            set_error(error, error_size, "Archive central directory is invalid.");
            return false;
        }

        uint16_t method = le16(zip + pos + 10);
        uint32_t compressed_size = le32(zip + pos + 20);
        uint32_t uncompressed_size = le32(zip + pos + 24);
        uint16_t name_len = le16(zip + pos + 28);
        uint16_t extra_len = le16(zip + pos + 30);
        uint16_t comment_len = le16(zip + pos + 32);
        uint32_t local_offset = le32(zip + pos + 42);
        if (pos + 46 + name_len + extra_len + comment_len > zip_size || name_len >= VD_PATH_MAX) {
            free(zip);
            set_error(error, error_size, "Archive entry metadata is invalid.");
            return false;
        }

        char entry_name[VD_PATH_MAX];
        memcpy(entry_name, zip + pos + 46, name_len);
        entry_name[name_len] = '\0';
        pos += 46 + name_len + extra_len + comment_len;

        if (is_macos_metadata_path(entry_name)) continue;

        if (!safe_zip_path(entry_name)) {
            free(zip);
            set_error(error, error_size, "Archive contains an unsafe path.");
            return false;
        }

        char entry_top[VD_PACKAGE_NAME_MAX];
        if (!top_level_name(entry_name, entry_top, sizeof(entry_top)) || !has_suffix(entry_top, ".vdapp")) {
            free(zip);
            set_error(error, error_size, "Archive must contain exactly one top-level .vdapp directory.");
            return false;
        }
        if (top_name[0] == '\0') {
            snprintf(top_name, sizeof(top_name), "%s", entry_top);
        } else if (strcmp(top_name, entry_top) != 0) {
            free(zip);
            set_error(error, error_size, "Archive contains more than one top-level entry.");
            return false;
        }

        bool is_directory = entry_name[strlen(entry_name) - 1] == '/';
        if (is_directory) continue;

        total_unpacked += uncompressed_size;
        if (total_unpacked > VD_UPLOAD_MAX_UNPACKED_BYTES || local_offset + 30 > zip_size || le32(zip + local_offset) != 0x04034b50) {
            free(zip);
            set_error(error, error_size, "Archive exceeds unpacked size limit or has invalid local data.");
            return false;
        }

        uint16_t local_name_len = le16(zip + local_offset + 26);
        uint16_t local_extra_len = le16(zip + local_offset + 28);
        size_t data_offset = local_offset + 30 + local_name_len + local_extra_len;
        if (data_offset + compressed_size > zip_size) {
            free(zip);
            set_error(error, error_size, "Archive entry data is invalid.");
            return false;
        }

        char out_path[VD_PATH_MAX];
        join_path(out_path, sizeof(out_path), package_library_staging_root(), entry_name);
        bool ok = false;
        if (method == 0) {
            ok = compressed_size == uncompressed_size && write_stored(zip + data_offset, uncompressed_size, out_path);
        } else if (method == 8) {
            ok = write_deflated(zip + data_offset, compressed_size, uncompressed_size, out_path);
        }
        if (!ok) {
            free(zip);
            set_error(error, error_size, "Archive contains an unsupported or invalid compressed entry.");
            return false;
        }
    }

    free(zip);
    if (top_name[0] == '\0') {
        set_error(error, error_size, "Archive did not contain a Deck App Package Directory.");
        return false;
    }

    snprintf(result->package_name, sizeof(result->package_name), "%s", top_name);
    join_path(result->package_path, sizeof(result->package_path), package_library_staging_root(), top_name);
    return true;
}
