#include "shell/shell.h"

#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include "core/package_library.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

typedef enum {
    SHELL_ACTION_UPLOAD,
    SHELL_ACTION_ACTIVATE,
    SHELL_ACTION_REMOVE
} ShellActionType;

typedef struct {
    ShellActionType type;
    char package_name[VD_PACKAGE_NAME_MAX];
    char label[192];
} ShellAction;

static bool start_pressed(void)
{
    bool gamepad = IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    return gamepad || IsKeyPressed(KEY_F1);
}

static bool confirm_pressed(void)
{
    bool gamepad = IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    return gamepad || IsKeyPressed(KEY_ENTER);
}

static bool back_pressed(void)
{
    bool gamepad = IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
    return gamepad || IsKeyPressed(KEY_BACKSPACE);
}

static bool up_pressed(void)
{
    return (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) || IsKeyPressed(KEY_UP);
}

static bool down_pressed(void)
{
    return (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) || IsKeyPressed(KEY_DOWN);
}

static int build_actions(ShellAction *actions, int max_actions)
{
    int count = 0;
    if (count < max_actions) {
        actions[count].type = SHELL_ACTION_UPLOAD;
        actions[count].package_name[0] = '\0';
        snprintf(actions[count].label, sizeof(actions[count].label), "Runtime Upload");
        count++;
    }

    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    for (int i = 0; i < package_count && count < max_actions; i++) {
        if (!packages[i].is_active) {
            actions[count].type = SHELL_ACTION_ACTIVATE;
            snprintf(actions[count].package_name, sizeof(actions[count].package_name), "%s", packages[i].package_name);
            snprintf(actions[count].label, sizeof(actions[count].label), "Activate %s %s", packages[i].display_name, packages[i].version);
            count++;
        }
        if (!packages[i].is_active && count < max_actions) {
            actions[count].type = SHELL_ACTION_REMOVE;
            snprintf(actions[count].package_name, sizeof(actions[count].package_name), "%s", packages[i].package_name);
            snprintf(actions[count].label, sizeof(actions[count].label), "Remove %s %s", packages[i].display_name, packages[i].version);
            count++;
        }
    }
    return count;
}

static void open_upload(VdShell *shell)
{
    shell->state = VD_SHELL_UPLOAD;
    shell->focus_index = 0;
    shell->bind_error[0] = '\0';
    shell->message[0] = '\0';
    char error[256];
    if (!upload_server_start(&shell->upload_server, error, sizeof(error))) {
        snprintf(shell->bind_error, sizeof(shell->bind_error), "%s", error);
    }
}

static void close_upload_to_home(VdShell *shell)
{
    upload_server_stop(&shell->upload_server);
    shell->state = VD_SHELL_HOME;
    shell->focus_index = 0;
}

static void activate_package(VdShell *shell, const char *package_name, bool *request_runtime_restart)
{
    char error[256];
    if (package_library_set_active(package_name, error, sizeof(error))) {
        snprintf(shell->message, sizeof(shell->message), "Activated %s.", package_name);
        shell->state = VD_SHELL_HIDDEN;
        *request_runtime_restart = true;
    } else {
        snprintf(shell->message, sizeof(shell->message), "%s", error);
    }
}

static void remove_package(VdShell *shell, const char *package_name)
{
    char error[256];
    if (package_library_remove(package_name, error, sizeof(error))) {
        snprintf(shell->message, sizeof(shell->message), "Removed %s.", package_name);
        shell->focus_index = 0;
    } else {
        snprintf(shell->message, sizeof(shell->message), "%s", error);
    }
}

void shell_init(VdShell *shell)
{
    memset(shell, 0, sizeof(*shell));
    shell->state = VD_SHELL_HIDDEN;
    upload_server_init(&shell->upload_server);
}

void shell_shutdown(VdShell *shell)
{
    upload_server_stop(&shell->upload_server);
}

bool shell_is_visible(const VdShell *shell)
{
    return shell->state != VD_SHELL_HIDDEN;
}

void shell_update(VdShell *shell, bool *request_runtime_restart)
{
    if (shell->state != VD_SHELL_UPLOAD) return;

    char message[256];
    upload_server_last_message(&shell->upload_server, message, sizeof(message));
    if (message[0] != '\0') snprintf(shell->message, sizeof(shell->message), "%s", message);

    bool restart_active = false;
    VdPackageInfo info;
    if (upload_server_take_success(&shell->upload_server, &restart_active, &info)) {
        upload_server_stop(&shell->upload_server);
        shell->state = VD_SHELL_HIDDEN;
        snprintf(shell->message, sizeof(shell->message), "Installed %s %s.", info.display_name, info.version);
        if (restart_active) *request_runtime_restart = true;
    }
}

void shell_poll_system_input(VdShell *shell, bool *request_runtime_restart)
{
    if (start_pressed()) {
        if (shell->state == VD_SHELL_HIDDEN) {
            shell->state = VD_SHELL_HOME;
            shell->focus_index = 0;
        } else if (shell->state == VD_SHELL_HOME) {
            shell->state = VD_SHELL_HIDDEN;
        }
    }

    if (shell->state == VD_SHELL_HIDDEN) return;

    if (shell->state == VD_SHELL_UPLOAD) {
        if (confirm_pressed() || back_pressed()) close_upload_to_home(shell);
        return;
    }

    ShellAction actions[VD_PACKAGE_LIST_MAX * 2 + 1];
    int action_count = build_actions(actions, (int)(sizeof(actions) / sizeof(actions[0])));
    if (action_count <= 0) return;
    if (shell->focus_index >= action_count) shell->focus_index = action_count - 1;

    if (up_pressed() && shell->focus_index > 0) shell->focus_index--;
    if (down_pressed() && shell->focus_index + 1 < action_count) shell->focus_index++;
    if (back_pressed()) {
        shell->state = VD_SHELL_HIDDEN;
        return;
    }
    if (confirm_pressed()) {
        ShellAction *action = &actions[shell->focus_index];
        if (action->type == SHELL_ACTION_UPLOAD) open_upload(shell);
        if (action->type == SHELL_ACTION_ACTIVATE) activate_package(shell, action->package_name, request_runtime_restart);
        if (action->type == SHELL_ACTION_REMOVE) remove_package(shell, action->package_name);
    }
}

static void draw_panel(void)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 18, 22, 30, 245 });
    DrawRectangleLines(24, 24, SCREEN_WIDTH - 48, SCREEN_HEIGHT - 48, (Color){ 120, 150, 190, 255 });
}

void shell_render(VdShell *shell)
{
    if (shell->state == VD_SHELL_HIDDEN) return;
    draw_panel();

    if (shell->state == VD_SHELL_UPLOAD) {
        DrawText("VitaDeck Runtime Upload", 48, 48, 30, RAYWHITE);
        if (shell->bind_error[0] != '\0') {
            DrawText("Upload listener could not start.", 48, 100, 24, RED);
            DrawText(shell->bind_error, 48, 136, 20, RAYWHITE);
        } else {
            DrawText("Open this URL from a browser on your LAN:", 48, 100, 22, RAYWHITE);
            DrawText(upload_server_url(&shell->upload_server), 48, 136, 28, GREEN);
            DrawText("Upload a zip containing exactly one .vdapp directory.", 48, 178, 20, RAYWHITE);
        }
        if (shell->message[0] != '\0') DrawText(shell->message, 48, 228, 20, YELLOW);
        DrawRectangle(48, 444, 220, 48, (Color){ 80, 95, 120, 255 });
        DrawText("Cancel Upload", 64, 457, 22, RAYWHITE);
        DrawText("Start is ignored here. Cross/Enter or Circle/Backspace cancels.", 48, 504, 16, GRAY);
        return;
    }

    DrawText("VitaDeck Shell", 48, 42, 32, RAYWHITE);
    DrawText("Installed Deck Apps", 48, 86, 20, GRAY);

    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    int y = 118;
    for (int i = 0; i < package_count; i++) {
        Color color = packages[i].is_active ? GREEN : RAYWHITE;
        char line[256];
        snprintf(line, sizeof(line), "%s %s%s", packages[i].display_name, packages[i].version, packages[i].is_active ? " (Active)" : "");
        DrawText(line, 64, y, 20, color);
        y += 26;
    }

    ShellAction actions[VD_PACKAGE_LIST_MAX * 2 + 1];
    int action_count = build_actions(actions, (int)(sizeof(actions) / sizeof(actions[0])));
    y = 284;
    DrawText("Actions", 48, y, 20, GRAY);
    y += 32;
    for (int i = 0; i < action_count; i++) {
        bool focused = i == shell->focus_index;
        DrawRectangle(54, y - 8, 640, 34, focused ? (Color){ 70, 95, 135, 255 } : (Color){ 35, 42, 55, 255 });
        DrawText(actions[i].label, 66, y, 20, RAYWHITE);
        y += 40;
    }

    if (shell->message[0] != '\0') DrawText(shell->message, 48, 486, 18, YELLOW);
    DrawText("F1/Start toggles Shell. Enter/Cross confirms. Backspace/Circle backs.", 48, 512, 16, GRAY);
}
