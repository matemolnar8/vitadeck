#ifndef VD_NET_HTTP_H
#define VD_NET_HTTP_H

#include <stdbool.h>
#include <stddef.h>
#include "arena.h"

#define VD_HTTP_HEADER_MAX 32768

// Copies the trimmed value of the header line starting with `name:` (case-insensitive). False if absent or too long.
bool http_header_value(const char *headers, const char *name, char *out, size_t out_size);
// Returns -1 when Content-Length is missing or not a decimal number.
long http_content_length(const char *headers);
// Compares the Content-Type media type, ignoring parameters and case.
bool http_content_type_is(const char *headers, const char *media_type);
unsigned char *http_find_bytes(unsigned char *haystack, size_t haystack_len, const char *needle);

// Reads the headers and a Content-Length body of at most max_body bytes into the arena. The body is NUL-terminated
// one byte past its length. On failure *error_status is 413 when the body is too large, else 400.
bool http_read_request(Arena *arena, int fd, size_t max_body, char **out_headers, unsigned char **out_body,
                       size_t *out_body_len, int *error_status);

void http_send_response(int fd, int status, const char *status_text, const char *content_type, const char *body);
// Reads past the request headers first, for a reply that does not need the request.
void http_send_simple_response(int fd, int status, const char *status_text, const char *content_type, const char *body);
// Sends the `{"ok":false,"code","message"}` body Runtime Upload and Host Control share.
void http_send_json_error(int fd, int status, const char *code, const char *message);
// Lingering close after an early rejection: a client still sending its body would otherwise get a connection reset
// instead of the reply. Reads at most max_body plus the headers, giving up after a quiet timeout.
void http_drain_rejected_request(int fd, size_t max_body);

#endif /* VD_NET_HTTP_H */
