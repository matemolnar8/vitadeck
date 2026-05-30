#include <stdio.h>
#include <string.h>

#include "picohttpparser.h"

int main(void)
{
    const char req[] =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost:8787\r\n"
        "Content-Length: 42\r\n"
        "\r\n";

    const char *method = NULL;
    size_t method_len = 0;
    const char *path = NULL;
    size_t path_len = 0;
    int minor_version = 0;
    struct phr_header headers[8];
    size_t num_headers = sizeof(headers) / sizeof(headers[0]);

    int consumed = phr_parse_request(req, strlen(req), &method, &method_len, &path, &path_len,
                                     &minor_version, headers, &num_headers, 0);
    if (consumed < 0) {
        fprintf(stderr, "parse failed: %d\n", consumed);
        return 1;
    }

    printf("method=%.*s path=%.*s version=1.%d headers=%zu consumed=%d\n",
           (int)method_len, method, (int)path_len, path, minor_version, num_headers, consumed);
    return 0;
}
