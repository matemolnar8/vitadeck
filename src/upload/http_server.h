#ifndef VD_UPLOAD_HTTP_SERVER_H
#define VD_UPLOAD_HTTP_SERVER_H

#include <stdbool.h>
#include <stddef.h>
#include "core/package_library.h"
#include "platform/thread.h"

#define VD_UPLOAD_MAX_OPEN_CLIENTS 16

typedef struct {
    vd_thread *thread;
    vd_thread *handlers[16];
    int handler_count;
    int open_client_fds[VD_UPLOAD_MAX_OPEN_CLIENTS];
    int open_client_count;
    vd_mutex *mutex;
    volatile bool running;
    bool ingesting;
    bool success_pending;
    bool restart_active;
    int listen_fd;
    int port;
    char url[128];
    char last_message[256];
    VdPackageInfo last_package;
} VdUploadServer;

void upload_server_init(VdUploadServer *server);
bool upload_server_start(VdUploadServer *server, char *error, size_t error_size);
void upload_server_stop(VdUploadServer *server);
bool upload_server_is_running(VdUploadServer *server);
const char *upload_server_url(VdUploadServer *server);
bool upload_server_take_success(VdUploadServer *server, bool *restart_active, VdPackageInfo *package_info);
void upload_server_last_message(VdUploadServer *server, char *out, size_t out_size);

#endif /* VD_UPLOAD_HTTP_SERVER_H */
