#ifndef VD_UPLOAD_HTTP_PARSE_H
#define VD_UPLOAD_HTTP_PARSE_H

#include <stdbool.h>
#include <stddef.h>

/* Copies the trimmed value of the header line starting with `name:` (case-insensitive). False if absent or too long. */
bool upload_header_value(const char *headers, const char *name, char *out, size_t out_size);
/* Returns -1 when Content-Length is missing or not a decimal number. */
long upload_content_length(const char *headers);
/* Escapes `in` for use inside a JSON string. Never splits an escape; 6x the input length always fits. */
void upload_json_escape(char *out, size_t out_size, const char *in);

#endif /* VD_UPLOAD_HTTP_PARSE_H */
