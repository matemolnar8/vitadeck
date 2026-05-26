#include "http_parse.h"

#include "picohttpparser.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <strings.h>

void vd_http_request_init(VdHttpRequest *req)
{
    memset(req, 0, sizeof(*req));
}

void vd_http_request_free(VdHttpRequest *req)
{
    free(req->header_block);
    free(req->body);
    vd_http_request_init(req);
}

const char *vd_http_header_value(const VdHttpRequest *req, const char *name)
{
    if (!req->header_block) return NULL;
    const char *p = req->header_block;
    size_t name_len = strlen(name);
    while (*p) {
        const char *line_end = strstr(p, "\r\n");
        if (!line_end) break;
        if (line_end == p) break;
        if ((size_t)(line_end - p) > name_len && strncasecmp(p, name, name_len) == 0 && p[name_len] == ':') {
            const char *value = p + name_len + 1;
            while (*value == ' ' || *value == '\t') value++;
            return value;
        }
        p = line_end + 2;
    }
    return NULL;
}

static long parse_content_length_header(const VdHttpRequest *req)
{
    const char *value = vd_http_header_value(req, "Content-Length");
    if (!value) return 0;
    return strtol(value, NULL, 10);
}

static bool grow_buffer(unsigned char **buf, size_t *cap, size_t need)
{
    if (need <= *cap) return true;
    size_t new_cap = *cap ? *cap : 4096;
    while (new_cap < need) new_cap *= 2;
    if (new_cap > VD_HTTP_HEADER_BUF_MAX + (1024 * 1024)) return false;
    unsigned char *grown = realloc(*buf, new_cap);
    if (!grown) return false;
    *buf = grown;
    *cap = new_cap;
    return true;
}

bool vd_http_read_request(int fd, size_t max_body, VdHttpRequest *req, int *error_status)
{
    unsigned char *buf = NULL;
    size_t cap = 0;
    size_t len = 0;
    size_t last_len = 0;

    const char *method = NULL;
    size_t method_len = 0;
    const char *path = NULL;
    size_t path_len = 0;
    int minor_version = 0;
    struct phr_header headers[VD_HTTP_MAX_HEADERS];
    size_t num_headers = VD_HTTP_MAX_HEADERS;

    while (true) {
        if (!grow_buffer(&buf, &cap, len + 1024)) {
            free(buf);
            *error_status = 400;
            return false;
        }
        if (len >= VD_HTTP_HEADER_BUF_MAX) {
            free(buf);
            *error_status = 413;
            return false;
        }

        ssize_t n = recv(fd, buf + len, cap - len, 0);
        if (n <= 0) {
            free(buf);
            *error_status = 400;
            return false;
        }
        len += (size_t)n;

        num_headers = VD_HTTP_MAX_HEADERS;
        int parsed = phr_parse_request((const char *)buf, len, &method, &method_len, &path, &path_len, &minor_version,
            headers, &num_headers, last_len);
        if (parsed > 0) {
            if ((size_t)parsed > len) {
                free(buf);
                *error_status = 400;
                return false;
            }

            size_t mlen = method_len < sizeof(req->method) - 1 ? method_len : sizeof(req->method) - 1;
            memcpy(req->method, method, mlen);
            req->method[mlen] = '\0';

            size_t plen = path_len < sizeof(req->path) - 1 ? path_len : sizeof(req->path) - 1;
            memcpy(req->path, path, plen);
            req->path[plen] = '\0';
            req->minor_version = minor_version;

            req->header_block = malloc((size_t)parsed + 1);
            if (!req->header_block) {
                free(buf);
                *error_status = 400;
                return false;
            }
            memcpy(req->header_block, buf, (size_t)parsed);
            req->header_block[(size_t)parsed] = '\0';
            req->header_block_len = (size_t)parsed;

            long content_length = parse_content_length_header(req);
            if (content_length < 0) {
                free(buf);
                vd_http_request_free(req);
                *error_status = 400;
                return false;
            }
            if ((size_t)content_length > max_body) {
                free(buf);
                vd_http_request_free(req);
                *error_status = 413;
                return false;
            }

            size_t body_in_buf = len - (size_t)parsed;
            if (content_length == 0) {
                req->body = NULL;
                req->body_len = 0;
                free(buf);
                return true;
            }

            req->body = malloc((size_t)content_length);
            if (!req->body) {
                free(buf);
                vd_http_request_free(req);
                *error_status = 400;
                return false;
            }

            size_t copied = body_in_buf < (size_t)content_length ? body_in_buf : (size_t)content_length;
            memcpy(req->body, buf + parsed, copied);
            size_t body_got = copied;
            while (body_got < (size_t)content_length) {
                ssize_t bn = recv(fd, req->body + body_got, (size_t)content_length - body_got, 0);
                if (bn <= 0) {
                    free(buf);
                    vd_http_request_free(req);
                    *error_status = 400;
                    return false;
                }
                body_got += (size_t)bn;
            }
            req->body_len = body_got;
            free(buf);
            return true;
        }

        if (parsed == -1) {
            free(buf);
            *error_status = 400;
            return false;
        }

        last_len = len;
    }
}
