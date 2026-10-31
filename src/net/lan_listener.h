#ifndef VD_NET_LAN_LISTENER_H
#define VD_NET_LAN_LISTENER_H

#include <stdbool.h>
#include <stddef.h>
#include "core/package_library.h"
#include "platform/thread.h"

#define VD_LAN_MAX_OPEN_CLIENTS 16

// The LAN Listener: Runtime Upload (`GET /`, `POST /upload`) and Host Control Link (`POST /link`) on one port.

typedef struct {
    vd_thread *thread;
    bool done;
} VdLanHandler;

typedef struct {
    vd_thread *thread;
    VdLanHandler handlers[VD_LAN_MAX_OPEN_CLIENTS];
    int open_client_fds[VD_LAN_MAX_OPEN_CLIENTS];
    int open_client_count;
    vd_mutex *mutex;
    volatile bool running;
    bool ingesting;
    bool success_pending;
    bool restart_active;
    int listen_fd;
    int port;
    char address[64];
    char url[128];
    char last_message[256];
    VdPackageInfo last_package;
} VdLanListener;

void lan_listener_init(VdLanListener *listener);
bool lan_listener_start(VdLanListener *listener, char *error, size_t error_size);
void lan_listener_stop(VdLanListener *listener);
void lan_listener_destroy(VdLanListener *listener);
bool lan_listener_is_running(VdLanListener *listener);
// `<ip>:<port>` and `http://<ip>:<port>/` while bound.
const char *lan_listener_address(VdLanListener *listener);
const char *lan_listener_url(VdLanListener *listener);
bool lan_listener_take_success(VdLanListener *listener, bool *restart_active, VdPackageInfo *package_info);
// The latest upload or link outcome, once.
bool lan_listener_take_message(VdLanListener *listener, char *out, size_t out_size);

#endif /* VD_NET_LAN_LISTENER_H */
