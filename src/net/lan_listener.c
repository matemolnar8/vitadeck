#include "net/lan_listener.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "arena.h"
#include "core/json.h"
#include "core/util.h"
#include "host_control/link.h"
#include "net/http.h"
#include "platform/lan_address.h"
#include "upload/archive.h"

#define VD_LAN_DEFAULT_PORT 8787
#define VD_LAN_PORT_TRIES 10
#define VD_LINK_MAX_BODY (64 * 1024)
// Per read, so a slow upload is fine; a client that goes quiet gives its slot back.
#define VD_LAN_RECV_TIMEOUT_SEC 15

typedef struct {
    VdLanListener *listener;
    int fd;
    int slot;
} HandlerArg;

static void remove_client_fd_locked(VdLanListener *listener, int fd)
{
    for (int i = 0; i < listener->open_client_count; i++) {
        if (listener->open_client_fds[i] == fd) {
            listener->open_client_fds[i] = listener->open_client_fds[--listener->open_client_count];
            break;
        }
    }
}

static void remove_client_fd(VdLanListener *listener, int fd)
{
    vd_mutex_lock(listener->mutex);
    remove_client_fd_locked(listener, fd);
    vd_mutex_unlock(listener->mutex);
}

static void reap_finished_handlers(VdLanListener *listener)
{
    vd_thread *finished[VD_LAN_MAX_OPEN_CLIENTS];
    int finished_count = 0;
    vd_mutex_lock(listener->mutex);
    for (int i = 0; i < VD_LAN_MAX_OPEN_CLIENTS; i++) {
        if (listener->handlers[i].thread && listener->handlers[i].done) {
            finished[finished_count++] = listener->handlers[i].thread;
            listener->handlers[i] = (VdLanHandler){0};
        }
    }
    vd_mutex_unlock(listener->mutex);
    for (int i = 0; i < finished_count; i++) {
        vd_thread_join(finished[i]);
        vd_thread_destroy(finished[i]);
    }
}

static void set_last_message(VdLanListener *listener, const char *message)
{
    vd_mutex_lock(listener->mutex);
    snprintf(listener->last_message, sizeof(listener->last_message), "%s", message);
    vd_mutex_unlock(listener->mutex);
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

static bool take_ingest_lock(VdLanListener *listener)
{
    bool ok = false;
    vd_mutex_lock(listener->mutex);
    if (!listener->ingesting) {
        listener->ingesting = true;
        ok = true;
    }
    vd_mutex_unlock(listener->mutex);
    return ok;
}

static void release_ingest_lock(VdLanListener *listener)
{
    vd_mutex_lock(listener->mutex);
    listener->ingesting = false;
    vd_mutex_unlock(listener->mutex);
}

static void handle_upload(HandlerArg *arg)
{
    VdLanListener *listener = arg->listener;
    Arena arena = {0};
    bool locked = false;

    if (!take_ingest_lock(listener)) {
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

    char archive_path[VD_PATH_MAX];
    snprintf(archive_path, sizeof(archive_path), "%s/upload.zip", package_library_staging_root());
    int status = 400;
    const char *code = "malformed_request";
    const char *message = "Could not parse upload.";
    if (!request_body_to_file(request_headers, body, body_len, archive_path, &status, &code, &message)) {
        http_send_json_error(arg->fd, status, code, message);
        set_last_message(listener, message);
        goto done;
    }

    char error[256];
    VdArchiveExtractResult extract;
    if (!upload_archive_extract(&arena, archive_path, &extract, error, sizeof(error))) {
        http_send_json_error(arg->fd, 422, "invalid_archive", error);
        set_last_message(listener, error);
        goto done;
    }

    bool replaced_active = false;
    VdPackageInfo info;
    if (!package_library_publish_package(extract.package_path, extract.package_name, &replaced_active, &info, error,
                                         sizeof(error))) {
        http_send_json_error(arg->fd, 422, "invalid_package", error);
        set_last_message(listener, error);
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

    vd_mutex_lock(listener->mutex);
    listener->success_pending = true;
    listener->restart_active = replaced_active;
    listener->last_package = info;
    snprintf(listener->last_message, sizeof(listener->last_message), "Installed %s %s.", info.display_name,
             info.version);
    vd_mutex_unlock(listener->mutex);

done:
    if (locked) release_ingest_lock(listener);
    arena_free(&arena);
}

static void reject_link(VdLanListener *listener, int fd, int status, const char *code, const char *message)
{
    http_send_json_error(fd, status, code, message);
    char line[sizeof(listener->last_message)];
    snprintf(line, sizeof(line), "Link rejected: %s", message);
    set_last_message(listener, line);
}

static void handle_link(HandlerArg *arg)
{
    VdLanListener *listener = arg->listener;
    Arena arena = {0};
    char *headers = NULL;
    unsigned char *body = NULL;
    size_t body_len = 0;
    int read_error = 400;
    if (!http_read_request(&arena, arg->fd, VD_LINK_MAX_BODY, &headers, &body, &body_len, &read_error)) {
        if (read_error == 413) {
            reject_link(listener, arg->fd, 413, "payload_too_large", "Link request exceeds 64 KiB.");
            http_drain_rejected_request(arg->fd, VD_LINK_MAX_BODY);
        } else {
            reject_link(listener, arg->fd, 400, "malformed_request", "Could not read link request.");
        }
        goto done;
    }
    if (!http_content_type_is(headers, "application/json")) {
        reject_link(listener, arg->fd, 400, "malformed_request", "Link request must be application/json.");
        goto done;
    }

    VdHostControlLink link;
    VdHostControlLinkReply reply = host_control_link_parse((const char *)body, &link);
    if (reply.status != 200) {
        reject_link(listener, arg->fd, reply.status, reply.code, reply.message);
        goto done;
    }
    if (!host_control_links_save(&link)) {
        reject_link(listener, arg->fd, 500, "command_failed", "VitaDeck could not save the link.");
        goto done;
    }

    http_send_response(arg->fd, 200, "OK", "application/json", "{\"ok\":true}");
    char line[sizeof(listener->last_message)];
    snprintf(line, sizeof(line), "Linked to %s at %s", link.name, link.url);
    set_last_message(listener, line);

done:
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
        handle_upload(arg);
    } else if (strcmp(path, "/link") == 0 && strcmp(method, "POST") != 0) {
        http_send_simple_response(arg->fd, 405, "Method Not Allowed", "text/plain", "Method Not Allowed");
    } else if (strcmp(path, "/link") == 0) {
        handle_link(arg);
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
    vd_mutex_lock(arg->listener->mutex);
    remove_client_fd_locked(arg->listener, arg->fd);
    arg->listener->handlers[arg->slot].done = true;
    vd_mutex_unlock(arg->listener->mutex);
    close(arg->fd);
    free(arg);
    return NULL;
}

static void *accept_thread(void *raw)
{
    VdLanListener *listener = raw;
    while (listener->running) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int fd = accept(listener->listen_fd, (struct sockaddr *)&client, &client_len);
        if (fd < 0) {
            if (listener->running) vd_thread_yield();
            continue;
        }
        struct timeval timeout = {.tv_sec = VD_LAN_RECV_TIMEOUT_SEC};
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        reap_finished_handlers(listener);

        HandlerArg *arg = calloc(1, sizeof(*arg));
        if (!arg) {
            close(fd);
            continue;
        }
        arg->listener = listener;
        arg->fd = fd;
        arg->slot = -1;

        vd_mutex_lock(listener->mutex);
        for (int i = 0; i < VD_LAN_MAX_OPEN_CLIENTS; i++) {
            if (!listener->handlers[i].thread) {
                arg->slot = i;
                break;
            }
        }
        if (arg->slot < 0 || listener->open_client_count >= VD_LAN_MAX_OPEN_CLIENTS) {
            vd_mutex_unlock(listener->mutex);
            close(fd);
            free(arg);
            continue;
        }
        listener->open_client_fds[listener->open_client_count++] = fd;
        vd_mutex_unlock(listener->mutex);

        int slot = arg->slot;
        vd_thread *handler = vd_thread_create(handler_thread, arg);
        if (!handler) {
            remove_client_fd(listener, fd);
            close(fd);
            free(arg);
            continue;
        }

        vd_mutex_lock(listener->mutex);
        listener->handlers[slot].thread = handler;
        vd_mutex_unlock(listener->mutex);
    }
    return NULL;
}

void lan_listener_init(VdLanListener *listener)
{
    memset(listener, 0, sizeof(*listener));
    listener->listen_fd = -1;
    listener->mutex = vd_mutex_create();
}

bool lan_listener_start(VdLanListener *listener, char *error, size_t error_size)
{
    if (listener->running) return true;
    if (!listener->mutex) listener->mutex = vd_mutex_create();
    listener->success_pending = false;
    listener->restart_active = false;
    listener->last_message[0] = '\0';

    for (int i = 0; i < VD_LAN_PORT_TRIES; i++) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) continue;
        int yes = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons((uint16_t)(VD_LAN_DEFAULT_PORT + i));
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0 && listen(fd, 8) == 0) {
            listener->listen_fd = fd;
            listener->port = VD_LAN_DEFAULT_PORT + i;
            char ip[48];
            lan_address_detect(ip, sizeof(ip));
            snprintf(listener->address, sizeof(listener->address), "%s:%d", ip, listener->port);
            snprintf(listener->url, sizeof(listener->url), "http://%s/", listener->address);
            listener->running = true;
            listener->thread = vd_thread_create(accept_thread, listener);
            if (!listener->thread) {
                close(fd);
                listener->listen_fd = -1;
                listener->running = false;
                vd_set_error(error, error_size, "Could not start the LAN Listener thread.");
                return false;
            }
            return true;
        }
        close(fd);
    }

    vd_set_error(error, error_size, "Could not bind the LAN Listener on ports 8787-8796.");
    return false;
}

void lan_listener_stop(VdLanListener *listener)
{
    if (!listener->running && !listener->thread) return;
    listener->running = false;
    if (listener->listen_fd >= 0) {
        shutdown(listener->listen_fd, SHUT_RDWR);
        close(listener->listen_fd);
        listener->listen_fd = -1;
    }
    if (listener->thread) {
        vd_thread_join(listener->thread);
        vd_thread_destroy(listener->thread);
        listener->thread = NULL;
    }

    int pending_fds[VD_LAN_MAX_OPEN_CLIENTS];
    int pending_n = 0;
    vd_mutex_lock(listener->mutex);
    pending_n = listener->open_client_count;
    for (int i = 0; i < pending_n; i++)
        pending_fds[i] = listener->open_client_fds[i];
    listener->open_client_count = 0;
    vd_mutex_unlock(listener->mutex);
    for (int i = 0; i < pending_n; i++) {
        if (pending_fds[i] >= 0) (void)shutdown(pending_fds[i], SHUT_RDWR);
    }

    for (int i = 0; i < VD_LAN_MAX_OPEN_CLIENTS; i++) {
        if (!listener->handlers[i].thread) continue;
        vd_thread_join(listener->handlers[i].thread);
        vd_thread_destroy(listener->handlers[i].thread);
        listener->handlers[i] = (VdLanHandler){0};
    }
}

void lan_listener_destroy(VdLanListener *listener)
{
    lan_listener_stop(listener);
    vd_mutex_destroy(listener->mutex);
    listener->mutex = NULL;
}

bool lan_listener_is_running(VdLanListener *listener)
{
    return listener->running;
}

const char *lan_listener_address(VdLanListener *listener)
{
    return listener->address;
}

const char *lan_listener_url(VdLanListener *listener)
{
    return listener->url;
}

bool lan_listener_take_success(VdLanListener *listener, bool *restart_active, VdPackageInfo *package_info)
{
    bool success = false;
    vd_mutex_lock(listener->mutex);
    if (listener->success_pending) {
        success = true;
        listener->success_pending = false;
        if (restart_active) *restart_active = listener->restart_active;
        if (package_info) *package_info = listener->last_package;
    }
    vd_mutex_unlock(listener->mutex);
    return success;
}

bool lan_listener_take_message(VdLanListener *listener, char *out, size_t out_size)
{
    vd_mutex_lock(listener->mutex);
    bool has_message = listener->last_message[0] != '\0';
    if (has_message) {
        snprintf(out, out_size, "%s", listener->last_message);
        listener->last_message[0] = '\0';
    }
    vd_mutex_unlock(listener->mutex);
    return has_message;
}
