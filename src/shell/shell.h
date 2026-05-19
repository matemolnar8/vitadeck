#ifndef VD_SHELL_H
#define VD_SHELL_H

#include <stdbool.h>
#include "core/package_library.h"
#include "core/settings.h"
#include "upload/http_server.h"

#define VD_SHELL_REMOVE_LABEL_MAX 192

typedef enum {
    VD_SHELL_HIDDEN,
    VD_SHELL_HOME,
    VD_SHELL_HOST_CONTROL
} VdShellState;

typedef struct {
    VdShellState state;
    int focus_row;
    int scroll_row;
    int focus_col;
    int remove_confirm_row;
    char remove_confirm_package[VD_PACKAGE_NAME_MAX];
    char remove_confirm_label[VD_SHELL_REMOVE_LABEL_MAX];
    char message[256];
    char bind_error[256];
    char host_control_url[VD_HOST_CONTROL_URL_MAX];
    int host_control_cursor;
    VdUploadServer upload_server;
} VdShell;

void shell_init(VdShell *shell);
void shell_shutdown(VdShell *shell);
bool shell_is_visible(const VdShell *shell);
void shell_update(VdShell *shell, bool *request_runtime_restart);
void shell_poll_system_input(VdShell *shell, bool *request_runtime_restart);
void shell_render(VdShell *shell);

#endif /* VD_SHELL_H */
