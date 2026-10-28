#include "host_control/listener.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "arena.h"
#include "core/util.h"
#include "net/http.h"
#include "platform/lan_address.h"

#define VD_HOST_CONTROL_DEFAULT_PORT 8798
#define VD_HOST_CONTROL_PORT_TRIES 10
#define VD_HOST_CONTROL_MAX_BODY (64 * 1024)
#define VD_HOST_CONTROL_RECV_TIMEOUT_SEC 2

static void set_message(VdHostControlListener *listener, const char *message)
{
    vd_mutex_lock(listener->mutex);
    snprintf(listener->last_message, sizeof(listener->last_message), "%s", message);
    vd_mutex_unlock(listener->mutex);
}

static void reject(VdHostControlListener *listener, int fd, int status, const char *code, const char *message)
{
    http_send_json_error(fd, status, code, message);
    char line[sizeof(listener->last_message)];
    snprintf(line, sizeof(line), "Link rejected: %s", message);
    set_message(listener, line);
}

// Answers one connection. True when it stored a link.
static bool handle_connection(VdHostControlListener *listener, int fd)
{
    char first[256];
    ssize_t n = recv(fd, first, sizeof(first) - 1, MSG_PEEK);
    if (n <= 0) return false;
    first[n] = '\0';

    char method[16] = {0};
    char path[128] = {0};
    sscanf(first, "%15s %127s", method, path);
    if (strcmp(path, "/link") != 0) {
        http_send_simple_response(fd, 404, "Not Found", "text/plain", "Not Found");
        return false;
    }
    if (strcmp(method, "POST") != 0) {
        http_send_simple_response(fd, 405, "Method Not Allowed", "text/plain", "Method Not Allowed");
        return false;
    }

    Arena arena = {0};
    bool linked = false;
    char *headers = NULL;
    unsigned char *body = NULL;
    size_t body_len = 0;
    int read_error = 400;
    if (!http_read_request(&arena, fd, VD_HOST_CONTROL_MAX_BODY, &headers, &body, &body_len, &read_error)) {
        if (read_error == 413) {
            reject(listener, fd, 413, "payload_too_large", "Link request exceeds 64 KiB.");
            http_drain_rejected_request(fd, VD_HOST_CONTROL_MAX_BODY);
        } else {
            reject(listener, fd, 400, "malformed_request", "Could not read link request.");
        }
        goto done;
    }
    if (!http_content_type_is(headers, "application/json")) {
        reject(listener, fd, 400, "malformed_request", "Link request must be application/json.");
        goto done;
    }

    VdHostControlLink link;
    VdHostControlLinkReply reply = host_control_link_parse((const char *)body, &link);
    if (reply.status != 200) {
        reject(listener, fd, reply.status, reply.code, reply.message);
        goto done;
    }
    if (!host_control_links_save(&link)) {
        reject(listener, fd, 500, "command_failed", "VitaDeck could not save the link.");
        goto done;
    }

    http_send_response(fd, 200, "OK", "application/json", "{\"ok\":true}");
    vd_mutex_lock(listener->mutex);
    listener->link_pending = true;
    listener->last_link = link;
    vd_mutex_unlock(listener->mutex);
    linked = true;

done:
    arena_free(&arena);
    return linked;
}

static void *listener_thread(void *raw)
{
    VdHostControlListener *listener = raw;
    while (listener->running) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int fd = accept(listener->listen_fd, (struct sockaddr *)&client, &client_len);
        if (fd < 0) {
            if (listener->running) vd_thread_yield();
            continue;
        }
        struct timeval timeout = {.tv_sec = VD_HOST_CONTROL_RECV_TIMEOUT_SEC};
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        bool linked = handle_connection(listener, fd);
        close(fd);
        if (linked) listener->running = false;
    }
    return NULL;
}

void host_control_listener_init(VdHostControlListener *listener)
{
    memset(listener, 0, sizeof(*listener));
    listener->listen_fd = -1;
    listener->mutex = vd_mutex_create();
}

bool host_control_listener_start(VdHostControlListener *listener, char *error, size_t error_size)
{
    if (listener->running) return true;
    host_control_listener_stop(listener);
    listener->link_pending = false;
    listener->last_message[0] = '\0';

    for (int i = 0; i < VD_HOST_CONTROL_PORT_TRIES; i++) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) continue;
        int yes = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons((uint16_t)(VD_HOST_CONTROL_DEFAULT_PORT + i));
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0 || listen(fd, 4) != 0) {
            close(fd);
            continue;
        }

        listener->listen_fd = fd;
        char ip[48];
        lan_address_detect(ip, sizeof(ip));
        snprintf(listener->address, sizeof(listener->address), "%s:%d", ip, VD_HOST_CONTROL_DEFAULT_PORT + i);
        listener->running = true;
        listener->thread = vd_thread_create(listener_thread, listener);
        if (!listener->thread) {
            listener->running = false;
            close(fd);
            listener->listen_fd = -1;
            vd_set_error(error, error_size, "Could not start the Host Control Link Listener thread.");
            return false;
        }
        return true;
    }

    vd_set_error(error, error_size, "Could not bind the Host Control Link Listener on ports 8798-8807.");
    return false;
}

void host_control_listener_stop(VdHostControlListener *listener)
{
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
}

void host_control_listener_destroy(VdHostControlListener *listener)
{
    host_control_listener_stop(listener);
    vd_mutex_destroy(listener->mutex);
    listener->mutex = NULL;
}

bool host_control_listener_is_running(const VdHostControlListener *listener)
{
    return listener->running;
}

const char *host_control_listener_address(const VdHostControlListener *listener)
{
    return listener->address;
}

bool host_control_listener_take_link(VdHostControlListener *listener, VdHostControlLink *out)
{
    vd_mutex_lock(listener->mutex);
    bool pending = listener->link_pending;
    if (pending) {
        *out = listener->last_link;
        listener->link_pending = false;
    }
    vd_mutex_unlock(listener->mutex);
    return pending;
}

bool host_control_listener_take_message(VdHostControlListener *listener, char *out, size_t out_size)
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
