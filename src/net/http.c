#include "net/http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "core/json.h"
#include "platform/socket.h"

#define VD_HTTP_DRAIN_TIMEOUT_SEC 2

static const char *next_line(const char *line)
{
    const char *newline = strchr(line, '\n');
    return newline ? newline + 1 : NULL;
}

bool http_header_value(const char *headers, const char *name, char *out, size_t out_size)
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

long http_content_length(const char *headers)
{
    char value[32];
    if (!http_header_value(headers, "Content-Length", value, sizeof(value))) return -1;
    char *end = NULL;
    long length = strtol(value, &end, 10);
    if (end == value || *end != '\0') return -1;
    return length;
}

bool http_content_type_is(const char *headers, const char *media_type)
{
    char value[256];
    if (!http_header_value(headers, "Content-Type", value, sizeof(value))) return false;
    size_t len = strcspn(value, "; \t");
    return len == strlen(media_type) && strncasecmp(value, media_type, len) == 0;
}

unsigned char *http_find_bytes(unsigned char *haystack, size_t haystack_len, const char *needle)
{
    size_t needle_len = strlen(needle);
    if (needle_len == 0 || haystack_len < needle_len) return NULL;
    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        if (memcmp(haystack + i, needle, needle_len) == 0) return haystack + i;
    }
    return NULL;
}

bool http_read_request(Arena *arena, int fd, size_t max_body, char **out_headers, unsigned char **out_body,
                       size_t *out_body_len, int *error_status)
{
    unsigned char *buffer = arena_alloc(arena, VD_HTTP_HEADER_MAX);
    if (!buffer) return false;

    size_t used = 0;
    unsigned char *header_end = NULL;
    while (used < VD_HTTP_HEADER_MAX) {
        ssize_t n = recv(fd, buffer + used, VD_HTTP_HEADER_MAX - used, 0);
        if (n <= 0) break;
        used += (size_t)n;
        header_end = http_find_bytes(buffer, used, "\r\n\r\n");
        if (header_end) break;
    }
    if (!header_end) {
        *error_status = 400;
        return false;
    }

    size_t header_len = (size_t)(header_end - buffer) + 4;
    char *headers = arena_alloc(arena, header_len + 1);
    if (!headers) return false;
    memcpy(headers, buffer, header_len);
    headers[header_len] = '\0';

    long content_length = http_content_length(headers);
    if (content_length < 0 || (size_t)content_length > max_body) {
        *error_status = content_length < 0 ? 400 : 413;
        return false;
    }

    unsigned char *body = arena_alloc(arena, (size_t)content_length + 1);
    if (!body) return false;
    size_t already = used - header_len;
    if (already > (size_t)content_length) already = (size_t)content_length;
    memcpy(body, buffer + header_len, already);

    size_t body_used = already;
    while (body_used < (size_t)content_length) {
        ssize_t n = recv(fd, body + body_used, (size_t)content_length - body_used, 0);
        if (n <= 0) break;
        body_used += (size_t)n;
    }
    if (body_used != (size_t)content_length) {
        *error_status = 400;
        return false;
    }
    body[body_used] = '\0';

    *out_headers = headers;
    *out_body = body;
    *out_body_len = body_used;
    return true;
}

static bool send_all(int fd, const char *data, size_t size)
{
    while (size > 0) {
        ssize_t n = send(fd, data, size, VD_MSG_NOSIGNAL);
        if (n <= 0) return false;
        data += n;
        size -= (size_t)n;
    }
    return true;
}

void http_send_response(int fd, int status, const char *status_text, const char *content_type, const char *body)
{
    char header[512];
    int body_len = (int)strlen(body);
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 %d %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %d\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              status, status_text, content_type, body_len);
    if (send_all(fd, header, (size_t)header_len)) send_all(fd, body, (size_t)body_len);
}

static void drain_request_headers(int fd)
{
    char buffer[1024];
    char window[4] = {0};
    int window_len = 0;

    while (true) {
        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
        if (n <= 0) return;
        for (ssize_t i = 0; i < n; i++) {
            if (window_len < 4) {
                window[window_len++] = buffer[i];
            } else {
                memmove(window, window + 1, 3);
                window[3] = buffer[i];
            }
            if (window_len == 4 && memcmp(window, "\r\n\r\n", 4) == 0) return;
        }
    }
}

void http_send_simple_response(int fd, int status, const char *status_text, const char *content_type, const char *body)
{
    drain_request_headers(fd);
    http_send_response(fd, status, status_text, content_type, body);
}

static const char *status_text(int status)
{
    switch (status) {
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 409:
        return "Conflict";
    case 413:
        return "Payload Too Large";
    case 415:
        return "Unsupported Media Type";
    case 422:
        return "Unprocessable Entity";
    case 500:
        return "Internal Server Error";
    default:
        return "Bad Request";
    }
}

void http_send_json_error(int fd, int status, const char *code, const char *message)
{
    char code_json[128];
    char message_json[1536];
    json_escape(code_json, sizeof(code_json), code);
    json_escape(message_json, sizeof(message_json), message);
    char body[2048];
    snprintf(body, sizeof(body), "{\"ok\":false,\"code\":\"%s\",\"message\":\"%s\"}", code_json, message_json);
    http_send_response(fd, status, status_text(status), "application/json", body);
}

void http_drain_rejected_request(int fd, size_t max_body)
{
    shutdown(fd, SHUT_WR);
    struct timeval timeout = {.tv_sec = VD_HTTP_DRAIN_TIMEOUT_SEC};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    char buffer[4096];
    for (size_t drained = 0; drained < max_body + VD_HTTP_HEADER_MAX;) {
        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
        if (n <= 0) return;
        drained += (size_t)n;
    }
}
