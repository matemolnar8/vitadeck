#define JIMP_IMPLEMENTATION
#include "core/json.h"

#include <stdio.h>
#include <string.h>

bool json_skip_value(Jimp *jimp)
{
    if (jimp_is_string_ahead(jimp)) return jimp_string(jimp);
    if (jimp_is_number_ahead(jimp)) return jimp_number(jimp);
    if (jimp_is_bool_ahead(jimp)) return jimp_bool(jimp);
    if (jimp_is_null_ahead(jimp)) return jimp__get_and_expect_token(jimp, JIMP_NULL);
    if (jimp_is_array_ahead(jimp)) {
        if (!jimp_array_begin(jimp)) return false;
        while (jimp_array_item(jimp)) {
            if (!json_skip_value(jimp)) return false;
        }
        return jimp_array_end(jimp);
    }
    if (jimp_is_object_ahead(jimp)) {
        if (!jimp_object_begin(jimp)) return false;
        while (jimp_object_member(jimp)) {
            if (!json_skip_value(jimp)) return false;
        }
        return jimp_object_end(jimp);
    }
    return false;
}

void json_escape(char *out, size_t out_size, const char *in)
{
    if (out_size == 0) return;
    size_t used = 0;
    for (const unsigned char *p = (const unsigned char *)in; *p; p++) {
        char escaped[7];
        int len = 1;
        if (*p == '"' || *p == '\\') {
            len = snprintf(escaped, sizeof(escaped), "\\%c", *p);
        } else if (*p < 0x20) {
            len = snprintf(escaped, sizeof(escaped), "\\u%04x", *p);
        } else {
            escaped[0] = (char)*p;
        }
        if (used + (size_t)len >= out_size) break;
        memcpy(out + used, escaped, (size_t)len);
        used += (size_t)len;
    }
    out[used] = '\0';
}
