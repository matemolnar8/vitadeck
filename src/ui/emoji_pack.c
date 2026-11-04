#include "emoji_pack.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "stb_ds.h"

#define EOCD_SIZE 22
#define EOCD_SEARCH (EOCD_SIZE + 0xffff)
#define CENTRAL_HEADER_SIZE 46
#define LOCAL_HEADER_SIZE 30
// A Twemoji image is a few KB; a bigger entry means a broken pack.
#define ENTRY_MAX_BYTES (1024 * 1024)

typedef struct {
    uint32_t local_offset;
    uint32_t compressed_size;
    uint32_t size;
    uint16_t method;
} PackEntry;

struct VdEmojiPack {
    FILE *file;
    struct {
        char *key;
        PackEntry value;
    } *entries;
};

static uint16_t le16(const unsigned char *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t le32(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static unsigned char *read_at(FILE *file, long offset, size_t size)
{
    unsigned char *data = malloc(size > 0 ? size : 1);
    if (!data) return NULL;
    if (fseek(file, offset, SEEK_SET) != 0 || fread(data, 1, size, file) != size) {
        free(data);
        return NULL;
    }
    return data;
}

static bool read_directory(VdEmojiPack *pack)
{
    if (fseek(pack->file, 0, SEEK_END) != 0) return false;
    long file_size = ftell(pack->file);
    if (file_size < EOCD_SIZE) return false;

    size_t tail_size = file_size < EOCD_SEARCH ? (size_t)file_size : EOCD_SEARCH;
    unsigned char *tail = read_at(pack->file, file_size - (long)tail_size, tail_size);
    if (!tail) return false;
    const unsigned char *eocd = NULL;
    for (size_t i = tail_size - EOCD_SIZE + 1; i-- > 0;) {
        if (le32(tail + i) == 0x06054b50) {
            eocd = tail + i;
            break;
        }
    }
    uint16_t count = eocd ? le16(eocd + 10) : 0;
    uint32_t directory_size = eocd ? le32(eocd + 12) : 0;
    uint32_t directory_offset = eocd ? le32(eocd + 16) : 0;
    free(tail);
    if (!eocd || directory_offset > (uint32_t)file_size || directory_size > (uint32_t)file_size - directory_offset) {
        return false;
    }

    unsigned char *directory = read_at(pack->file, (long)directory_offset, directory_size);
    if (!directory) return false;
    sh_new_strdup(pack->entries);
    bool ok = true;
    size_t pos = 0;
    for (uint16_t i = 0; ok && i < count; i++) {
        const unsigned char *header = directory + pos;
        ok = directory_size - pos >= CENTRAL_HEADER_SIZE && le32(header) == 0x02014b50;
        if (!ok) break;
        size_t name_length = le16(header + 28);
        size_t record = CENTRAL_HEADER_SIZE + name_length + le16(header + 30) + le16(header + 32);
        ok = directory_size - pos >= record;
        if (!ok) break;

        char name[256];
        if (name_length < sizeof(name)) {
            memcpy(name, header + CENTRAL_HEADER_SIZE, name_length);
            name[name_length] = '\0';
            PackEntry entry = {
                .local_offset = le32(header + 42),
                .compressed_size = le32(header + 20),
                .size = le32(header + 24),
                .method = le16(header + 10),
            };
            shput(pack->entries, name, entry);
        }
        pos += record;
    }
    free(directory);
    return ok;
}

VdEmojiPack *emoji_pack_open(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;
    VdEmojiPack *pack = calloc(1, sizeof(*pack));
    if (!pack) {
        fclose(file);
        return NULL;
    }
    pack->file = file;
    if (!read_directory(pack)) {
        emoji_pack_close(pack);
        return NULL;
    }
    return pack;
}

void emoji_pack_close(VdEmojiPack *pack)
{
    if (!pack) return;
    shfree(pack->entries);
    fclose(pack->file);
    free(pack);
}

int emoji_pack_count(const VdEmojiPack *pack)
{
    return (int)shlen(pack->entries);
}

const char *emoji_pack_name(const VdEmojiPack *pack, int index)
{
    return pack->entries[index].key;
}

static unsigned char *inflate_entry(const unsigned char *data, uint32_t compressed_size, uint32_t size)
{
    unsigned char *out = malloc(size > 0 ? size : 1);
    if (!out) return NULL;
    z_stream stream = {0};
    stream.next_in = (Bytef *)data;
    stream.avail_in = compressed_size;
    stream.next_out = out;
    stream.avail_out = size;
    bool ok = inflateInit2(&stream, -MAX_WBITS) == Z_OK;
    ok = ok && inflate(&stream, Z_FINISH) == Z_STREAM_END && stream.total_out == size;
    inflateEnd(&stream);
    if (ok) return out;
    free(out);
    return NULL;
}

unsigned char *emoji_pack_read(VdEmojiPack *pack, const char *name, size_t *size)
{
    ptrdiff_t index = shgeti(pack->entries, name);
    if (index < 0) return NULL;
    PackEntry entry = pack->entries[index].value;
    if (entry.method != 8 || entry.size > ENTRY_MAX_BYTES || entry.compressed_size > ENTRY_MAX_BYTES) return NULL;

    unsigned char header[LOCAL_HEADER_SIZE];
    if (fseek(pack->file, (long)entry.local_offset, SEEK_SET) != 0 ||
        fread(header, 1, sizeof(header), pack->file) != sizeof(header) || le32(header) != 0x04034b50) {
        return NULL;
    }
    long data_offset = (long)entry.local_offset + LOCAL_HEADER_SIZE + le16(header + 26) + le16(header + 28);
    unsigned char *data = read_at(pack->file, data_offset, entry.compressed_size);
    if (!data) return NULL;
    unsigned char *inflated = inflate_entry(data, entry.compressed_size, entry.size);
    free(data);
    if (inflated) *size = entry.size;
    return inflated;
}
