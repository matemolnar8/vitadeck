#include "upload/http_server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "upload/archive.h"

#ifdef __vita__
#include <psp2/net/netctl.h>
#else
#include <ifaddrs.h>
#endif

#define VD_UPLOAD_DEFAULT_PORT 8787
#define VD_UPLOAD_PORT_TRIES 10
#define VD_UPLOAD_HEADER_MAX 32768

typedef struct {
    VdUploadServer *server;
    int fd;
} HandlerArg;

static void set_error(char *error, size_t error_size, const char *message)
{
    if (!error || error_size == 0) return;
    snprintf(error, error_size, "%s", message);
}

static void set_last_message(VdUploadServer *server, const char *message)
{
    vd_mutex_lock(server->mutex);
    snprintf(server->last_message, sizeof(server->last_message), "%s", message);
    vd_mutex_unlock(server->mutex);
}

static void send_response(int fd, int status, const char *status_text, const char *content_type, const char *body)
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
    send(fd, header, (size_t)header_len, 0);
    send(fd, body, (size_t)body_len, 0);
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

static void send_simple_response(int fd, int status, const char *status_text, const char *content_type, const char *body)
{
    drain_request_headers(fd);
    send_response(fd, status, status_text, content_type, body);
}

static void send_json_error(int fd, int status, const char *code, const char *message)
{
    char body[512];
    snprintf(body, sizeof(body), "{\"ok\":false,\"code\":\"%s\",\"message\":\"%s\"}", code, message);
    send_response(fd, status, status == 409 ? "Conflict" : status == 413 ? "Payload Too Large" : status == 415 ? "Unsupported Media Type" : status == 422 ? "Unprocessable Entity" : "Bad Request", "application/json", body);
}

static void detect_lan_ip(char *out, size_t out_size)
{
    snprintf(out, out_size, "127.0.0.1");
#ifdef __vita__
    SceNetCtlInfo info;
    memset(&info, 0, sizeof(info));
    if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info) == 0 && info.ip_address[0] != '\0') {
        snprintf(out, out_size, "%s", info.ip_address);
    }
#else
    struct ifaddrs *ifaddr = NULL;
    if (getifaddrs(&ifaddr) != 0) return;
    for (struct ifaddrs *ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
        struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
        const char *ip = inet_ntoa(addr->sin_addr);
        if (!ip || strncmp(ip, "127.", 4) == 0) continue;
        snprintf(out, out_size, "%s", ip);
        break;
    }
    freeifaddrs(ifaddr);
#endif
}

static const char *header_value(const char *headers, const char *name)
{
    size_t name_len = strlen(name);
    const char *p = headers;
    while ((p = strstr(p, name))) {
        if ((p == headers || p[-1] == '\n') && p[name_len] == ':') {
            p += name_len + 1;
            while (*p == ' ' || *p == '\t') p++;
            return p;
        }
        p += name_len;
    }
    return NULL;
}

static long parse_content_length(const char *headers)
{
    const char *value = header_value(headers, "Content-Length");
    if (!value) return -1;
    return strtol(value, NULL, 10);
}

static bool content_type_has(const char *headers, const char *needle)
{
    const char *value = header_value(headers, "Content-Type");
    return value && strstr(value, needle);
}

static bool write_bytes(const char *path, const unsigned char *data, size_t size)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = fwrite(data, 1, size, f) == size;
    fclose(f);
    return ok;
}

static bool extract_boundary(const char *headers, char *boundary, size_t boundary_size)
{
    const char *ct = header_value(headers, "Content-Type");
    if (!ct) return false;
    const char *p = strstr(ct, "boundary=");
    if (!p) return false;
    p += strlen("boundary=");
    if (*p == '"') p++;
    size_t len = 0;
    while (*p && *p != '"' && *p != '\r' && *p != '\n' && *p != ';' && len + 1 < boundary_size) {
        boundary[len++] = *p++;
    }
    boundary[len] = '\0';
    return len > 0;
}

static unsigned char *find_bytes(unsigned char *haystack, size_t haystack_len, const char *needle)
{
    size_t needle_len = strlen(needle);
    if (needle_len == 0 || haystack_len < needle_len) return NULL;
    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        if (memcmp(haystack + i, needle, needle_len) == 0) return haystack + i;
    }
    return NULL;
}

static bool multipart_archive_to_file(const char *headers, unsigned char *body, size_t body_len, const char *out_path)
{
    char boundary[128];
    if (!extract_boundary(headers, boundary, sizeof(boundary))) return false;

    unsigned char *part = find_bytes(body, body_len, "name=\"archive\"");
    if (!part) return false;
    unsigned char *data_start = find_bytes(part, body_len - (size_t)(part - body), "\r\n\r\n");
    if (!data_start) return false;
    data_start += 4;

    char end_marker[160];
    snprintf(end_marker, sizeof(end_marker), "\r\n--%s", boundary);
    unsigned char *data_end = find_bytes(data_start, body_len - (size_t)(data_start - body), end_marker);
    if (!data_end || data_end < data_start) return false;

    return write_bytes(out_path, data_start, (size_t)(data_end - data_start));
}

static bool request_body_to_file(const char *headers, unsigned char *body, size_t body_len, const char *out_path, int *status, const char **code, const char **message)
{
    if (content_type_has(headers, "application/zip")) {
        if (!write_bytes(out_path, body, body_len)) {
            *status = 400; *code = "write_failed"; *message = "Could not write uploaded archive.";
            return false;
        }
        return true;
    }

    if (content_type_has(headers, "multipart/form-data")) {
        if (!multipart_archive_to_file(headers, body, body_len, out_path)) {
            *status = 400; *code = "malformed_multipart"; *message = "Multipart upload must include file field archive.";
            return false;
        }
        return true;
    }

    *status = 415; *code = "unsupported_content_type"; *message = "Use application/zip or multipart/form-data.";
    return false;
}

static bool read_request(int fd, char **out_headers, unsigned char **out_body, size_t *out_body_len, int *error_status)
{
    unsigned char *buffer = malloc(VD_UPLOAD_HEADER_MAX);
    if (!buffer) return false;

    size_t used = 0;
    unsigned char *header_end = NULL;
    while (used < VD_UPLOAD_HEADER_MAX) {
        ssize_t n = recv(fd, buffer + used, VD_UPLOAD_HEADER_MAX - used, 0);
        if (n <= 0) break;
        used += (size_t)n;
        header_end = find_bytes(buffer, used, "\r\n\r\n");
        if (header_end) break;
    }
    if (!header_end) {
        free(buffer);
        *error_status = 400;
        return false;
    }

    size_t header_len = (size_t)(header_end - buffer) + 4;
    char *headers = malloc(header_len + 1);
    if (!headers) {
        free(buffer);
        return false;
    }
    memcpy(headers, buffer, header_len);
    headers[header_len] = '\0';

    long content_length = parse_content_length(headers);
    if (content_length < 0 || content_length > VD_UPLOAD_MAX_BYTES) {
        free(headers);
        free(buffer);
        *error_status = content_length > VD_UPLOAD_MAX_BYTES ? 413 : 400;
        return false;
    }

    unsigned char *body = malloc((size_t)content_length);
    if (!body) {
        free(headers);
        free(buffer);
        return false;
    }
    size_t already = used - header_len;
    if (already > (size_t)content_length) already = (size_t)content_length;
    memcpy(body, buffer + header_len, already);
    free(buffer);

    size_t body_used = already;
    while (body_used < (size_t)content_length) {
        ssize_t n = recv(fd, body + body_used, (size_t)content_length - body_used, 0);
        if (n <= 0) break;
        body_used += (size_t)n;
    }
    if (body_used != (size_t)content_length) {
        free(headers);
        free(body);
        *error_status = 400;
        return false;
    }

    *out_headers = headers;
    *out_body = body;
    *out_body_len = body_used;
    return true;
}

static bool take_ingest_lock(VdUploadServer *server)
{
    bool ok = false;
    vd_mutex_lock(server->mutex);
    if (!server->ingesting) {
        server->ingesting = true;
        ok = true;
    }
    vd_mutex_unlock(server->mutex);
    return ok;
}

static void release_ingest_lock(VdUploadServer *server)
{
    vd_mutex_lock(server->mutex);
    server->ingesting = false;
    vd_mutex_unlock(server->mutex);
}

static void handle_upload(HandlerArg *arg, const char *headers)
{
    VdUploadServer *server = arg->server;
    if (!take_ingest_lock(server)) {
        send_json_error(arg->fd, 409, "upload_in_progress", "Another Runtime Upload is already in progress.");
        return;
    }

    char *request_headers = NULL;
    unsigned char *body = NULL;
    size_t body_len = 0;
    int read_error = 400;
    if (!read_request(arg->fd, &request_headers, &body, &body_len, &read_error)) {
        send_json_error(arg->fd, read_error, read_error == 413 ? "upload_too_large" : "malformed_request", read_error == 413 ? "Uploaded archive exceeds 16MB." : "Could not read upload request.");
        release_ingest_lock(server);
        return;
    }

    (void)headers;
    char archive_path[VD_PATH_MAX];
    snprintf(archive_path, sizeof(archive_path), "%s/upload.zip", package_library_staging_root());
    int status = 400;
    const char *code = "malformed_request";
    const char *message = "Could not parse upload.";
    if (!request_body_to_file(request_headers, body, body_len, archive_path, &status, &code, &message)) {
        send_json_error(arg->fd, status, code, message);
        set_last_message(server, message);
        free(request_headers);
        free(body);
        release_ingest_lock(server);
        return;
    }
    free(request_headers);
    free(body);

    char error[256];
    VdArchiveExtractResult extract;
    if (!upload_archive_extract(archive_path, &extract, error, sizeof(error))) {
        send_json_error(arg->fd, 422, "invalid_archive", error);
        set_last_message(server, error);
        release_ingest_lock(server);
        return;
    }

    bool replaced_active = false;
    VdPackageInfo info;
    if (!package_library_publish_package(extract.package_path, extract.package_name, &replaced_active, &info, error, sizeof(error))) {
        send_json_error(arg->fd, 422, "invalid_package", error);
        set_last_message(server, error);
        release_ingest_lock(server);
        return;
    }

    char body_json[512];
    snprintf(body_json, sizeof(body_json), "{\"ok\":true,\"packageName\":\"%s\",\"version\":\"%s\"}", info.package_name, info.version);
    send_response(arg->fd, 200, "OK", "application/json", body_json);

    vd_mutex_lock(server->mutex);
    server->success_pending = true;
    server->restart_active = replaced_active;
    server->last_package = info;
    snprintf(server->last_message, sizeof(server->last_message), "Installed %s %s.", info.display_name, info.version);
    vd_mutex_unlock(server->mutex);
    release_ingest_lock(server);
}

static void *handler_thread(void *raw)
{
    HandlerArg *arg = raw;
    char first[1024];
    ssize_t n = recv(arg->fd, first, sizeof(first) - 1, MSG_PEEK);
    if (n <= 0) goto done;
    first[n] = '\0';

    char method[16] = {0};
    char path[128] = {0};
    sscanf(first, "%15s %127s", method, path);

    if (strcmp(path, "/upload") == 0 && strcmp(method, "POST") != 0) {
        send_simple_response(arg->fd, 405, "Method Not Allowed", "text/plain", "Method Not Allowed");
    } else if (strcmp(path, "/upload") == 0) {
        handle_upload(arg, first);
    } else if (strcmp(path, "/") == 0 && strcmp(method, "GET") == 0) {
        const char *html =
            "<!doctype html><html><head><meta charset=\"utf-8\"><title>VitaDeck Upload</title>"
            "<style>body{font-family:system-ui;margin:2rem;max-width:42rem}button{font-size:1rem;padding:.6rem 1rem}</style></head>"
            "<body><h1>VitaDeck Runtime Upload</h1><p>Select a zip containing exactly one .vdapp directory.</p>"
            "<form id=\"form\"><input id=\"archive\" name=\"archive\" type=\"file\" accept=\".zip\" required><button>Upload</button></form>"
            "<pre id=\"result\"></pre><script>form.onsubmit=async e=>{e.preventDefault();const fd=new FormData();fd.append('archive',archive.files[0]);"
            "const r=await fetch('/upload',{method:'POST',body:fd});result.textContent=JSON.stringify(await r.json(),null,2)}</script></body></html>";
        send_simple_response(arg->fd, 200, "OK", "text/html; charset=utf-8", html);
    } else {
        send_simple_response(arg->fd, 404, "Not Found", "text/plain", "Not Found");
    }

done:
    close(arg->fd);
    free(arg);
    return NULL;
}

static void *server_thread(void *raw)
{
    VdUploadServer *server = raw;
    while (server->running) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int fd = accept(server->listen_fd, (struct sockaddr *)&client, &client_len);
        if (fd < 0) {
            if (server->running) vd_thread_yield();
            continue;
        }

        HandlerArg *arg = calloc(1, sizeof(*arg));
        if (!arg) {
            close(fd);
            continue;
        }
        arg->server = server;
        arg->fd = fd;
        vd_thread *handler = vd_thread_create(handler_thread, arg);
        if (!handler) {
            close(fd);
            free(arg);
            continue;
        }

        vd_mutex_lock(server->mutex);
        if (server->handler_count < 16) {
            server->handlers[server->handler_count++] = handler;
        } else {
            vd_thread_destroy(handler);
        }
        vd_mutex_unlock(server->mutex);
    }
    return NULL;
}

void upload_server_init(VdUploadServer *server)
{
    memset(server, 0, sizeof(*server));
    server->listen_fd = -1;
    server->mutex = vd_mutex_create();
}

bool upload_server_start(VdUploadServer *server, char *error, size_t error_size)
{
    if (server->running) return true;
    if (!server->mutex) server->mutex = vd_mutex_create();
    server->success_pending = false;
    server->restart_active = false;
    server->last_message[0] = '\0';

    for (int i = 0; i < VD_UPLOAD_PORT_TRIES; i++) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) continue;
        int yes = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons((uint16_t)(VD_UPLOAD_DEFAULT_PORT + i));
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0 && listen(fd, 8) == 0) {
            server->listen_fd = fd;
            server->port = VD_UPLOAD_DEFAULT_PORT + i;
            char ip[64];
            detect_lan_ip(ip, sizeof(ip));
            snprintf(server->url, sizeof(server->url), "http://%s:%d/", ip, server->port);
            server->running = true;
            server->thread = vd_thread_create(server_thread, server);
            if (!server->thread) {
                close(fd);
                server->listen_fd = -1;
                server->running = false;
                set_error(error, error_size, "Could not start Runtime Upload Listener thread.");
                return false;
            }
            return true;
        }
        close(fd);
    }

    set_error(error, error_size, "Could not bind Runtime Upload Listener on ports 8787-8796.");
    return false;
}

void upload_server_stop(VdUploadServer *server)
{
    if (!server->running && !server->thread) return;
    server->running = false;
    if (server->listen_fd >= 0) {
        shutdown(server->listen_fd, SHUT_RDWR);
        close(server->listen_fd);
        server->listen_fd = -1;
    }
    if (server->thread) {
        vd_thread_join(server->thread);
        vd_thread_destroy(server->thread);
        server->thread = NULL;
    }
    for (int i = 0; i < server->handler_count; i++) {
        vd_thread_join(server->handlers[i]);
        vd_thread_destroy(server->handlers[i]);
        server->handlers[i] = NULL;
    }
    server->handler_count = 0;
}

bool upload_server_is_running(VdUploadServer *server)
{
    return server->running;
}

const char *upload_server_url(VdUploadServer *server)
{
    return server->url;
}

bool upload_server_take_success(VdUploadServer *server, bool *restart_active, VdPackageInfo *package_info)
{
    bool success = false;
    vd_mutex_lock(server->mutex);
    if (server->success_pending) {
        success = true;
        server->success_pending = false;
        if (restart_active) *restart_active = server->restart_active;
        if (package_info) *package_info = server->last_package;
    }
    vd_mutex_unlock(server->mutex);
    return success;
}

void upload_server_last_message(VdUploadServer *server, char *out, size_t out_size)
{
    vd_mutex_lock(server->mutex);
    snprintf(out, out_size, "%s", server->last_message);
    vd_mutex_unlock(server->mutex);
}
