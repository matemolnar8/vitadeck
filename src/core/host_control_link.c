#include "host_control_link.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "package_library.h"

#define HC_LINK_FILE "host-control-link.json"
#define HC_HOST_NAME_MAX 128
#define HC_CALLBACK_URL_MAX 512
#define HC_JSON_MAX 1024

typedef struct {
    bool loaded;
    bool linked;
    char host_name[HC_HOST_NAME_MAX];
    char callback_url[HC_CALLBACK_URL_MAX];
} HostControlLinkState;

static HostControlLinkState g_link;

static void hc_send_simple(int fd, int status, const char *status_text, const char *body)
{
    int body_len = body ? (int)strlen(body) : 0;
    char header[512];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, status_text, body_len);
    send(fd, header, (size_t)header_len, 0);
    if (body_len > 0) send(fd, body, (size_t)body_len, 0);
}

static bool hc_extract_json_string(const char *body, const char *key, char *out, size_t out_size)
{
    if (!body || !key || !out || out_size == 0) return false;
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *found = strstr(body, pattern);
    if (!found) return false;
    const char *colon = strchr(found, ':');
    if (!colon) return false;
    const char *start = colon + 1;
    while (*start == ' ' || *start == '\t') start++;
    if (*start != '"') return false;
    start++;
    const char *end = start;
    while (*end && *end != '"') {
        if (*end == '\\' && end[1]) {
            end += 2;
            continue;
        }
        end++;
    }
    if (*end != '"') return false;
    size_t len = (size_t)(end - start);
    if (len + 1 > out_size) return false;
    memcpy(out, start, len);
    out[len] = '\0';
    return true;
}

static bool hc_valid_callback_url(const char *url)
{
    if (!url || url[0] == '\0') return false;
    if (strncmp(url, "http://", 7) != 0) return false;
    for (const char *p = url; *p; p++) {
        if (isspace((unsigned char)*p)) return false;
    }
    return strlen(url) < HC_CALLBACK_URL_MAX;
}

static void hc_link_path(char *out, size_t out_size)
{
    snprintf(out, out_size, "%s/%s", package_library_root(), HC_LINK_FILE);
}

static bool hc_save_link_file(void)
{
    char path[VD_PATH_MAX];
    hc_link_path(path, sizeof(path));
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    int n = fprintf(f,
        "{\"hostName\":\"%s\",\"callbackUrl\":\"%s\"}",
        g_link.host_name,
        g_link.callback_url);
    fclose(f);
    return n > 0;
}

void host_control_link_load(void)
{
    g_link.loaded = true;
    g_link.linked = false;
    g_link.host_name[0] = '\0';
    g_link.callback_url[0] = '\0';

    char path[VD_PATH_MAX];
    hc_link_path(path, sizeof(path));
    FILE *f = fopen(path, "rb");
    if (!f) return;

    char buffer[HC_JSON_MAX];
    size_t read_len = fread(buffer, 1, sizeof(buffer) - 1, f);
    fclose(f);
    if (read_len == 0) return;
    buffer[read_len] = '\0';

    char host_name[HC_HOST_NAME_MAX];
    char callback_url[HC_CALLBACK_URL_MAX];
    if (!hc_extract_json_string(buffer, "hostName", host_name, sizeof(host_name))) return;
    if (!hc_extract_json_string(buffer, "callbackUrl", callback_url, sizeof(callback_url))) return;
    if (!hc_valid_callback_url(callback_url)) return;

    snprintf(g_link.host_name, sizeof(g_link.host_name), "%s", host_name);
    snprintf(g_link.callback_url, sizeof(g_link.callback_url), "%s", callback_url);
    g_link.linked = true;
}

bool host_control_link_is_linked(void)
{
    host_control_link_load();
    return g_link.linked;
}

bool host_control_link_get_callback_url(char *out, size_t out_size)
{
    host_control_link_load();
    if (!g_link.linked || !out || out_size == 0) return false;
    snprintf(out, out_size, "%s", g_link.callback_url);
    return true;
}

bool host_control_link_get_host_name(char *out, size_t out_size)
{
    host_control_link_load();
    if (!g_link.linked || !out || out_size == 0) return false;
    snprintf(out, out_size, "%s", g_link.host_name);
    return true;
}

void host_control_link_format_status_line(char *out, size_t out_size)
{
    if (!out || out_size == 0) return;
    host_control_link_load();
    if (!g_link.linked) {
        snprintf(out, out_size, "Host: not linked");
        return;
    }
    snprintf(out, out_size, "Host: %s (%s)", g_link.host_name, g_link.callback_url);
}

static bool hc_apply_link(const char *host_name, const char *callback_url)
{
    if (!hc_valid_callback_url(callback_url)) return false;
    snprintf(g_link.host_name, sizeof(g_link.host_name), "%s", host_name ? host_name : "");
    snprintf(g_link.callback_url, sizeof(g_link.callback_url), "%s", callback_url);
    g_link.linked = true;
    return hc_save_link_file();
}

void host_control_link_handle_post(int client_fd, const char *body, size_t body_len)
{
    (void)body_len;
    if (!body) {
        hc_send_simple(client_fd, 400, "Bad Request",
            "{\"ok\":false,\"code\":\"malformed_request\",\"message\":\"Missing body.\"}");
        return;
    }

    char host_name[HC_HOST_NAME_MAX] = "";
    char callback_url[HC_CALLBACK_URL_MAX] = "";
    if (!hc_extract_json_string(body, "callbackUrl", callback_url, sizeof(callback_url))) {
        hc_send_simple(client_fd, 400, "Bad Request",
            "{\"ok\":false,\"code\":\"malformed_request\",\"message\":\"Missing callbackUrl.\"}");
        return;
    }
    (void)hc_extract_json_string(body, "hostName", host_name, sizeof(host_name));

    if (!hc_apply_link(host_name, callback_url)) {
        hc_send_simple(client_fd, 400, "Bad Request",
            "{\"ok\":false,\"code\":\"malformed_request\",\"message\":\"Invalid callbackUrl.\"}");
        return;
    }

    hc_send_simple(client_fd, 200, "OK", "{\"ok\":true,\"linked\":true}");
}

void host_control_link_handle_status_get(int client_fd)
{
    host_control_link_load();
    char body[HC_JSON_MAX];
    if (!g_link.linked) {
        snprintf(body, sizeof(body), "{\"ok\":true,\"linked\":false}");
    } else {
        char escaped_host[HC_HOST_NAME_MAX * 2];
        char escaped_url[HC_CALLBACK_URL_MAX * 2];
        const char *h = g_link.host_name;
        char *p = escaped_host;
        for (; *h && (size_t)(p - escaped_host) < sizeof(escaped_host) - 2; h++) {
            if (*h == '\\' || *h == '"') *p++ = '\\';
            *p++ = *h;
        }
        *p = '\0';
        h = g_link.callback_url;
        p = escaped_url;
        for (; *h && (size_t)(p - escaped_url) < sizeof(escaped_url) - 2; h++) {
            if (*h == '\\' || *h == '"') *p++ = '\\';
            *p++ = *h;
        }
        *p = '\0';
        snprintf(body, sizeof(body),
            "{\"ok\":true,\"linked\":true,\"hostName\":\"%s\",\"callbackUrl\":\"%s\"}",
            escaped_host,
            escaped_url);
    }
    hc_send_simple(client_fd, 200, "OK", body);
}
