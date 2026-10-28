#include "upload/http_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "arena.h"
#include "core/json.h"
#include "core/util.h"
#include "net/http.h"
#include "platform/lan_address.h"
#include "upload/archive.h"

#define VD_UPLOAD_DEFAULT_PORT 8787
#define VD_UPLOAD_PORT_TRIES 10

typedef struct {
    VdUploadServer *server;
    int fd;
    int slot;
} HandlerArg;

static void remove_client_fd_locked(VdUploadServer *server, int fd)
{
    for (int i = 0; i < server->open_client_count; i++) {
        if (server->open_client_fds[i] == fd) {
            server->open_client_fds[i] = server->open_client_fds[--server->open_client_count];
            break;
        }
    }
}

static void remove_client_fd(VdUploadServer *server, int fd)
{
    vd_mutex_lock(server->mutex);
    remove_client_fd_locked(server, fd);
    vd_mutex_unlock(server->mutex);
}

static void reap_finished_handlers(VdUploadServer *server)
{
    vd_thread *finished[VD_UPLOAD_MAX_OPEN_CLIENTS];
    int finished_count = 0;
    vd_mutex_lock(server->mutex);
    for (int i = 0; i < VD_UPLOAD_MAX_OPEN_CLIENTS; i++) {
        if (server->handlers[i].thread && server->handlers[i].done) {
            finished[finished_count++] = server->handlers[i].thread;
            server->handlers[i] = (VdUploadHandler){0};
        }
    }
    vd_mutex_unlock(server->mutex);
    for (int i = 0; i < finished_count; i++) {
        vd_thread_join(finished[i]);
        vd_thread_destroy(finished[i]);
    }
}

static void set_last_message(VdUploadServer *server, const char *message)
{
    vd_mutex_lock(server->mutex);
    snprintf(server->last_message, sizeof(server->last_message), "%s", message);
    vd_mutex_unlock(server->mutex);
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
    char ct[256];
    if (!http_header_value(headers, "Content-Type", ct, sizeof(ct))) return false;
    const char *p = strstr(ct, "boundary=");
    if (!p) return false;
    p += strlen("boundary=");
    if (*p == '"') p++;
    size_t len = 0;
    while (*p && *p != '"' && *p != ';' && len + 1 < boundary_size) {
        boundary[len++] = *p++;
    }
    boundary[len] = '\0';
    return len > 0;
}

static bool multipart_archive_to_file(const char *headers, unsigned char *body, size_t body_len, const char *out_path)
{
    char boundary[128];
    if (!extract_boundary(headers, boundary, sizeof(boundary))) return false;

    unsigned char *part = http_find_bytes(body, body_len, "name=\"archive\"");
    if (!part) return false;
    unsigned char *data_start = http_find_bytes(part, body_len - (size_t)(part - body), "\r\n\r\n");
    if (!data_start) return false;
    data_start += 4;

    char end_marker[160];
    snprintf(end_marker, sizeof(end_marker), "\r\n--%s", boundary);
    unsigned char *data_end = http_find_bytes(data_start, body_len - (size_t)(data_start - body), end_marker);
    if (!data_end || data_end < data_start) return false;

    return write_bytes(out_path, data_start, (size_t)(data_end - data_start));
}

static bool request_body_to_file(const char *headers, unsigned char *body, size_t body_len, const char *out_path,
                                 int *status, const char **code, const char **message)
{
    if (http_content_type_is(headers, "application/zip")) {
        if (!write_bytes(out_path, body, body_len)) {
            *status = 400;
            *code = "write_failed";
            *message = "Could not write uploaded archive.";
            return false;
        }
        return true;
    }

    if (http_content_type_is(headers, "multipart/form-data")) {
        if (!multipart_archive_to_file(headers, body, body_len, out_path)) {
            *status = 400;
            *code = "malformed_multipart";
            *message = "Multipart upload must include file field archive.";
            return false;
        }
        return true;
    }

    *status = 415;
    *code = "unsupported_content_type";
    *message = "Use application/zip or multipart/form-data.";
    return false;
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
    Arena arena = {0};
    bool locked = false;

    if (!take_ingest_lock(server)) {
        http_send_json_error(arg->fd, 409, "upload_in_progress", "Another Runtime Upload is already in progress.");
        http_drain_rejected_request(arg->fd, VD_UPLOAD_MAX_BYTES);
        goto done;
    }
    locked = true;

    char *request_headers = NULL;
    unsigned char *body = NULL;
    size_t body_len = 0;
    int read_error = 400;
    if (!http_read_request(&arena, arg->fd, VD_UPLOAD_MAX_BYTES, &request_headers, &body, &body_len, &read_error)) {
        http_send_json_error(arg->fd, read_error, read_error == 413 ? "upload_too_large" : "malformed_request",
                             read_error == 413 ? "Uploaded archive exceeds 16MB." : "Could not read upload request.");
        if (read_error == 413) http_drain_rejected_request(arg->fd, VD_UPLOAD_MAX_BYTES);
        goto done;
    }

    (void)headers;
    char archive_path[VD_PATH_MAX];
    snprintf(archive_path, sizeof(archive_path), "%s/upload.zip", package_library_staging_root());
    int status = 400;
    const char *code = "malformed_request";
    const char *message = "Could not parse upload.";
    if (!request_body_to_file(request_headers, body, body_len, archive_path, &status, &code, &message)) {
        http_send_json_error(arg->fd, status, code, message);
        set_last_message(server, message);
        goto done;
    }

    char error[256];
    VdArchiveExtractResult extract;
    if (!upload_archive_extract(&arena, archive_path, &extract, error, sizeof(error))) {
        http_send_json_error(arg->fd, 422, "invalid_archive", error);
        set_last_message(server, error);
        goto done;
    }

    bool replaced_active = false;
    VdPackageInfo info;
    if (!package_library_publish_package(extract.package_path, extract.package_name, &replaced_active, &info, error,
                                         sizeof(error))) {
        http_send_json_error(arg->fd, 422, "invalid_package", error);
        set_last_message(server, error);
        goto done;
    }

    char name_json[VD_PACKAGE_NAME_MAX * 6];
    char version_json[VD_VERSION_MAX * 6];
    json_escape(name_json, sizeof(name_json), info.package_name);
    json_escape(version_json, sizeof(version_json), info.version);
    char body_json[2048];
    snprintf(body_json, sizeof(body_json), "{\"ok\":true,\"packageName\":\"%s\",\"version\":\"%s\"}", name_json,
             version_json);
    http_send_response(arg->fd, 200, "OK", "application/json", body_json);

    vd_mutex_lock(server->mutex);
    server->success_pending = true;
    server->restart_active = replaced_active;
    server->last_package = info;
    snprintf(server->last_message, sizeof(server->last_message), "Installed %s %s.", info.display_name, info.version);
    vd_mutex_unlock(server->mutex);

done:
    if (locked) release_ingest_lock(server);
    arena_free(&arena);
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
        http_send_simple_response(arg->fd, 405, "Method Not Allowed", "text/plain", "Method Not Allowed");
    } else if (strcmp(path, "/upload") == 0) {
        handle_upload(arg, first);
    } else if (strcmp(path, "/") == 0 && strcmp(method, "GET") == 0) {
        const char *html =
            "<!doctype html><html><head><meta charset=\"utf-8\"><title>VitaDeck Upload</title>"
            "<style>body{font-family:system-ui;margin:2rem;max-width:42rem}button{font-size:1rem;padding:.6rem "
            "1rem}</style></head>"
            "<body><h1>VitaDeck</h1><p>Select a zip containing exactly one .vdapp directory.</p>"
            "<form id=\"form\"><input id=\"archive\" name=\"archive\" type=\"file\" accept=\".zip\" "
            "required><button>Upload</button></form>"
            "<pre id=\"result\"></pre><script>form.onsubmit=async e=>{e.preventDefault();const fd=new "
            "FormData();fd.append('archive',archive.files[0]);"
            "const r=await fetch('/upload',{method:'POST',body:fd});result.textContent=JSON.stringify(await "
            "r.json(),null,2)}</script></body></html>";
        http_send_simple_response(arg->fd, 200, "OK", "text/html; charset=utf-8", html);
    } else {
        http_send_simple_response(arg->fd, 404, "Not Found", "text/plain", "Not Found");
    }

done:
    vd_mutex_lock(arg->server->mutex);
    remove_client_fd_locked(arg->server, arg->fd);
    arg->server->handlers[arg->slot].done = true;
    vd_mutex_unlock(arg->server->mutex);
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

        reap_finished_handlers(server);

        HandlerArg *arg = calloc(1, sizeof(*arg));
        if (!arg) {
            close(fd);
            continue;
        }
        arg->server = server;
        arg->fd = fd;
        arg->slot = -1;

        vd_mutex_lock(server->mutex);
        for (int i = 0; i < VD_UPLOAD_MAX_OPEN_CLIENTS; i++) {
            if (!server->handlers[i].thread) {
                arg->slot = i;
                break;
            }
        }
        if (arg->slot < 0 || server->open_client_count >= VD_UPLOAD_MAX_OPEN_CLIENTS) {
            vd_mutex_unlock(server->mutex);
            close(fd);
            free(arg);
            continue;
        }
        server->open_client_fds[server->open_client_count++] = fd;
        vd_mutex_unlock(server->mutex);

        int slot = arg->slot;
        vd_thread *handler = vd_thread_create(handler_thread, arg);
        if (!handler) {
            remove_client_fd(server, fd);
            close(fd);
            free(arg);
            continue;
        }

        vd_mutex_lock(server->mutex);
        server->handlers[slot].thread = handler;
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
            lan_address_detect(ip, sizeof(ip));
            snprintf(server->url, sizeof(server->url), "http://%s:%d/", ip, server->port);
            server->running = true;
            server->thread = vd_thread_create(server_thread, server);
            if (!server->thread) {
                close(fd);
                server->listen_fd = -1;
                server->running = false;
                vd_set_error(error, error_size, "Could not start Runtime Upload Listener thread.");
                return false;
            }
            return true;
        }
        close(fd);
    }

    vd_set_error(error, error_size, "Could not bind Runtime Upload Listener on ports 8787-8796.");
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

    int pending_fds[VD_UPLOAD_MAX_OPEN_CLIENTS];
    int pending_n = 0;
    vd_mutex_lock(server->mutex);
    pending_n = server->open_client_count;
    for (int i = 0; i < pending_n; i++)
        pending_fds[i] = server->open_client_fds[i];
    server->open_client_count = 0;
    vd_mutex_unlock(server->mutex);
    for (int i = 0; i < pending_n; i++) {
        if (pending_fds[i] >= 0) (void)shutdown(pending_fds[i], SHUT_RDWR);
    }

    for (int i = 0; i < VD_UPLOAD_MAX_OPEN_CLIENTS; i++) {
        if (!server->handlers[i].thread) continue;
        vd_thread_join(server->handlers[i].thread);
        vd_thread_destroy(server->handlers[i].thread);
        server->handlers[i] = (VdUploadHandler){0};
    }
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

bool upload_server_take_message(VdUploadServer *server, char *out, size_t out_size)
{
    vd_mutex_lock(server->mutex);
    bool has_message = server->last_message[0] != '\0';
    if (has_message) {
        snprintf(out, out_size, "%s", server->last_message);
        server->last_message[0] = '\0';
    }
    vd_mutex_unlock(server->mutex);
    return has_message;
}
