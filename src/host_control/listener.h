#ifndef VD_HOST_CONTROL_LISTENER_H
#define VD_HOST_CONTROL_LISTENER_H

#include <stdbool.h>
#include <stddef.h>
#include "host_control/link.h"
#include "platform/thread.h"

// Accepts one Host Control Link on `POST /link`, then stops itself.
typedef struct {
    vd_thread *thread;
    vd_mutex *mutex;
    volatile bool running;
    int listen_fd;
    // The connection being answered, so stop can cut off a client that sends slowly. -1 when none.
    int client_fd;
    char address[64];
    bool link_pending;
    VdHostControlLink last_link;
    char last_message[256];
} VdHostControlListener;

void host_control_listener_init(VdHostControlListener *listener);
bool host_control_listener_start(VdHostControlListener *listener, char *error, size_t error_size);
// Joins the thread and closes the socket; also needed after the listener stopped itself.
void host_control_listener_stop(VdHostControlListener *listener);
void host_control_listener_destroy(VdHostControlListener *listener);
bool host_control_listener_is_running(const VdHostControlListener *listener);
// `<ip>:<port>` while bound.
const char *host_control_listener_address(const VdHostControlListener *listener);
// True once per successful link, with the link the Host Companion sent.
bool host_control_listener_take_link(VdHostControlListener *listener, VdHostControlLink *out);
// The latest rejected link request, once.
bool host_control_listener_take_message(VdHostControlListener *listener, char *out, size_t out_size);

#endif /* VD_HOST_CONTROL_LISTENER_H */
