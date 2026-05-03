#ifndef VD_SHELL_H
#define VD_SHELL_H

#include <stdbool.h>
#include "upload/http_server.h"

typedef enum {
    VD_SHELL_HIDDEN,
    VD_SHELL_HOME,
    VD_SHELL_UPLOAD
} VdShellState;

typedef struct {
    VdShellState state;
    int focus_index;
    char message[256];
    char bind_error[256];
    VdUploadServer upload_server;
} VdShell;

void shell_init(VdShell *shell);
void shell_shutdown(VdShell *shell);
bool shell_is_visible(const VdShell *shell);
void shell_update(VdShell *shell, bool *request_runtime_restart);
void shell_poll_system_input(VdShell *shell, bool *request_runtime_restart);
void shell_render(VdShell *shell);

#endif /* VD_SHELL_H */
