#ifndef VD_HTTP_PARSE_H
#define VD_HTTP_PARSE_H

#include <stdbool.h>
#include <stddef.h>

#define VD_HTTP_HEADER_BUF_MAX 32768
#define VD_HTTP_MAX_HEADERS 32

typedef struct {
    char method[16];
    char path[256];
    int minor_version;
    char *header_block;
    size_t header_block_len;
    unsigned char *body;
    size_t body_len;
} VdHttpRequest;

void vd_http_request_init(VdHttpRequest *req);
void vd_http_request_free(VdHttpRequest *req);

/* Reads a full HTTP/1.x request using picohttpparser. Sets *error_status on failure (400/413). */
bool vd_http_read_request(int fd, size_t max_body, VdHttpRequest *req, int *error_status);

const char *vd_http_header_value(const VdHttpRequest *req, const char *name);

#endif /* VD_HTTP_PARSE_H */
