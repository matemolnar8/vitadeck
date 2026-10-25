#ifndef VD_SOCKET_H
#define VD_SOCKET_H

#include <sys/socket.h>

// send() flag so a write to a disconnected client fails with EPIPE instead of killing the process with SIGPIPE.
// sceNet never raises SIGPIPE and the Vita headers have no MSG_NOSIGNAL.
#ifdef __vita__
#define VD_MSG_NOSIGNAL 0
#else
#define VD_MSG_NOSIGNAL MSG_NOSIGNAL
#endif

#endif /* VD_SOCKET_H */
