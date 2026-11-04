#ifndef VD_EMOJI_PACK_H
#define VD_EMOJI_PACK_H

#include <stddef.h>

// The emoji images in one zip, read an entry at a time: the Vita's filesystem and its FTP server take minutes over
// thousands of small files. Reads deflated entries, as `cmake -E tar --format=zip` writes them.
typedef struct VdEmojiPack VdEmojiPack;

// NULL when the file is missing or not a zip.
VdEmojiPack *emoji_pack_open(const char *path);
void emoji_pack_close(VdEmojiPack *pack);
int emoji_pack_count(const VdEmojiPack *pack);
const char *emoji_pack_name(const VdEmojiPack *pack, int index);
// The entry's bytes from malloc; NULL when the pack has no such entry or it does not read.
unsigned char *emoji_pack_read(VdEmojiPack *pack, const char *name, size_t *size);

#endif /* VD_EMOJI_PACK_H */
