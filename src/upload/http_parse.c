#include "upload/http_parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static const char *next_line(const char *line)
{
    const char *newline = strchr(line, '\n');
    return newline ? newline + 1 : NULL;
}

bool upload_header_value(const char *headers, const char *name, char *out, size_t out_size)
{
    size_t name_len = strlen(name);
    for (const char *line = headers; line; line = next_line(line)) {
        if (strncasecmp(line, name, name_len) != 0 || line[name_len] != ':') continue;
        const char *value = line + name_len + 1;
        while (*value == ' ' || *value == '\t')
            value++;
        size_t len = strcspn(value, "\r\n");
        while (len > 0 && (value[len - 1] == ' ' || value[len - 1] == '\t'))
            len--;
        if (len >= out_size) return false;
        memcpy(out, value, len);
        out[len] = '\0';
        return true;
    }
    return false;
}

long upload_content_length(const char *headers)
{
    char value[32];
    if (!upload_header_value(headers, "Content-Length", value, sizeof(value))) return -1;
    char *end = NULL;
    long length = strtol(value, &end, 10);
    if (end == value || *end != '\0') return -1;
    return length;
}

void upload_json_escape(char *out, size_t out_size, const char *in)
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
