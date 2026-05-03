#include "shell/shell.h"

#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include "core/package_library.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define SHELL_INSET 24
#define SHELL_PAD_L 48
#define SHELL_PAD_R 56

#define SHELL_FONT_TITLE 26
#define SHELL_FONT_BODY 18
#define SHELL_FONT_CAPTION 16

#define SHELL_SPACE_XS 6
#define SHELL_SPACE_SM 8
#define SHELL_SPACE_MD 16
#define SHELL_SPACE_LG 24

#define SHELL_FOOTER_Y (SCREEN_HEIGHT - SHELL_INSET - SHELL_SPACE_MD - SHELL_FONT_CAPTION)

#define SHELL_TITLE_Y SHELL_SPACE_LG
#define SHELL_SECTION_Y (SHELL_TITLE_Y + SHELL_FONT_TITLE + SHELL_SPACE_SM)
#define SHELL_LIST_Y (SHELL_SECTION_Y + SHELL_FONT_BODY + SHELL_SPACE_MD)

#define SHELL_ROW_H 50
#define SHELL_ROW_STEP (SHELL_ROW_H + SHELL_SPACE_SM)
#define SHELL_UPLOAD_ROW_H (SHELL_SPACE_MD + SHELL_FONT_BODY + SHELL_SPACE_XS + SHELL_FONT_CAPTION + SHELL_SPACE_MD)
#define SHELL_NAME_X SHELL_PAD_L
#define SHELL_X_CX (SCREEN_WIDTH - SHELL_PAD_R - 28)
#define SHELL_TICK_CX (SHELL_X_CX - 80)

static int shell_text_baseline_y(int row_top, int font_h)
{
    return row_top + (SHELL_ROW_H - font_h) / 2;
}

static void shell_draw_footer_hint(void)
{
    DrawText("F1 · Enter · Back", SHELL_PAD_L, SHELL_FOOTER_Y, SHELL_FONT_CAPTION, GRAY);
}

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

static bool left_pressed(void)
{
    return (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) || IsKeyPressed(KEY_LEFT);
}

static bool right_pressed(void)
{
    return (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) || IsKeyPressed(KEY_RIGHT);
}

static void draw_checkmark(int cx, int cy, Color c)
{
    DrawLine(cx - 6, cy, cx - 1, cy + 6, c);
    DrawLine(cx - 1, cy + 6, cx + 10, cy - 8, c);
}

static int shell_row_count(void)
{
    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int n = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    return n + 1;
}

static bool row_is_upload(int row, int package_count)
{
    return row == package_count;
}

static void clear_remove_confirm(VdShell *shell)
{
    shell->remove_confirm_package[0] = '\0';
    shell->remove_confirm_label[0] = '\0';
    shell->remove_confirm_row = -1;
}

static void open_upload(VdShell *shell)
{
    shell->state = VD_SHELL_UPLOAD;
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
    int n = shell_row_count();
    shell->focus_row = n > 0 ? n - 1 : 0;
    shell->focus_col = 0;
}

static void hide_shell_to_deck(VdShell *shell)
{
    upload_server_stop(&shell->upload_server);
    shell->state = VD_SHELL_HIDDEN;
    clear_remove_confirm(shell);
}

static void activate_package(VdShell *shell, const char *package_name, bool *request_runtime_restart)
{
    char error[256];
    if (package_library_set_active(package_name, error, sizeof(error))) {
        snprintf(shell->message, sizeof(shell->message), "Activated %s.", package_name);
        hide_shell_to_deck(shell);
        *request_runtime_restart = true;
    } else {
        snprintf(shell->message, sizeof(shell->message), "%s", error);
    }
}

static void remove_package_confirmed(VdShell *shell, const char *package_name, int removed_row)
{
    char error[256];
    if (!package_library_remove(package_name, error, sizeof(error))) {
        snprintf(shell->message, sizeof(shell->message), "%s", error);
        clear_remove_confirm(shell);
        return;
    }
    snprintf(shell->message, sizeof(shell->message), "Removed %s.", package_name);
    clear_remove_confirm(shell);
    if (shell->focus_row > removed_row) shell->focus_row--;
    int rows = shell_row_count();
    if (shell->focus_row >= rows) shell->focus_row = rows > 0 ? rows - 1 : 0;
    shell->focus_col = 0;
}

void shell_init(VdShell *shell)
{
    memset(shell, 0, sizeof(*shell));
    shell->state = VD_SHELL_HIDDEN;
    shell->remove_confirm_row = -1;
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
        shell->state = VD_SHELL_HOME;
        shell->focus_row = 0;
        shell->focus_col = 0;
        snprintf(shell->message, sizeof(shell->message), "Installed %s %s.", info.display_name, info.version);
        if (restart_active) *request_runtime_restart = true;
    }
}

void shell_poll_system_input(VdShell *shell, bool *request_runtime_restart)
{
    if (start_pressed()) {
        if (shell->state == VD_SHELL_HIDDEN) {
            shell->state = VD_SHELL_HOME;
            shell->focus_row = 0;
            shell->focus_col = 0;
        } else if (shell->state == VD_SHELL_HOME) {
            hide_shell_to_deck(shell);
        }
    }

    if (shell->state == VD_SHELL_HIDDEN) return;

    if (shell->state == VD_SHELL_UPLOAD) {
        if (confirm_pressed() || back_pressed()) close_upload_to_home(shell);
        return;
    }

    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    int row_count = package_count + 1;
    if (row_count <= 0) return;

    if (shell->remove_confirm_package[0] != '\0') {
        if (back_pressed()) {
            clear_remove_confirm(shell);
            return;
        }
        if (confirm_pressed()) {
            remove_package_confirmed(shell, shell->remove_confirm_package, shell->remove_confirm_row);
        }
        return;
    }

    if (shell->focus_row >= row_count) shell->focus_row = row_count - 1;
    if (shell->focus_row < 0) shell->focus_row = 0;

    if (up_pressed() && shell->focus_row > 0) {
        shell->focus_row--;
        shell->focus_col = 0;
    }
    if (down_pressed() && shell->focus_row + 1 < row_count) {
        shell->focus_row++;
        shell->focus_col = 0;
    }

    if (row_is_upload(shell->focus_row, package_count)) {
        shell->focus_col = 0;
    } else {
        if (left_pressed() && shell->focus_col > 0) shell->focus_col--;
        if (right_pressed() && shell->focus_col < 1) shell->focus_col++;
    }

    if (back_pressed()) {
        hide_shell_to_deck(shell);
        return;
    }

    if (!confirm_pressed()) return;

    if (row_is_upload(shell->focus_row, package_count)) {
        open_upload(shell);
        return;
    }

    const VdPackageInfo *pkg = &packages[shell->focus_row];
    if (shell->focus_col == 0) {
        if (!pkg->is_active) activate_package(shell, pkg->package_name, request_runtime_restart);
        return;
    }

    snprintf(shell->remove_confirm_package, sizeof(shell->remove_confirm_package), "%s", pkg->package_name);
    snprintf(shell->remove_confirm_label, sizeof(shell->remove_confirm_label), "Remove %s %s?", pkg->display_name, pkg->version);
    shell->remove_confirm_row = shell->focus_row;
}

static void draw_panel(void)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 18, 22, 30, 255 });
    DrawRectangleLines(24, 24, SCREEN_WIDTH - 48, SCREEN_HEIGHT - 48, (Color){ 120, 150, 190, 255 });
}

static void draw_tick_cell(int cx, int row_top, bool is_active, bool row_focused, bool col_focused)
{
    const int cell_h = 34;
    const int half = 22;
    int cell_y = row_top + (SHELL_ROW_H - cell_h) / 2;
    Rectangle r = { (float)(cx - half), (float)cell_y, (float)(half * 2), (float)cell_h };
    bool slot_focus = row_focused && col_focused;
    Color fill = slot_focus ? (Color){ 55, 75, 110, 255 } : (Color){ 38, 46, 60, 255 };
    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 1.5f, (Color){ 100, 120, 150, 255 });
    int mid_y = row_top + SHELL_ROW_H / 2;
    if (is_active) {
        draw_checkmark(cx, mid_y + 2, GREEN);
    } else {
        Color hint = slot_focus ? RAYWHITE : (Color){ 160, 170, 190, 255 };
        const char *go = "Run";
        int ty = shell_text_baseline_y(row_top, SHELL_FONT_BODY);
        DrawText(go, cx - MeasureText(go, SHELL_FONT_BODY) / 2, ty, SHELL_FONT_BODY, hint);
    }
}

static void draw_x_cell(int cx, int row_top, bool row_focused, bool col_focused)
{
    const int cell_h = 34;
    const int half = 22;
    int cell_y = row_top + (SHELL_ROW_H - cell_h) / 2;
    Rectangle r = { (float)(cx - half), (float)cell_y, (float)(half * 2), (float)cell_h };
    bool slot_focus = row_focused && col_focused;
    Color fill = slot_focus ? (Color){ 90, 55, 55, 255 } : (Color){ 38, 46, 60, 255 };
    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 1.5f, (Color){ 140, 100, 100, 255 });
    Color t = slot_focus ? (Color){ 255, 200, 200, 255 } : (Color){ 200, 140, 140, 255 };
    const char *label = " X ";
    int ty = shell_text_baseline_y(row_top, SHELL_FONT_BODY);
    DrawText(label, cx - MeasureText(label, SHELL_FONT_BODY) / 2, ty, SHELL_FONT_BODY, t);
}

void shell_render(VdShell *shell)
{
    if (shell->state == VD_SHELL_HIDDEN) return;
    draw_panel();

    if (shell->state == VD_SHELL_UPLOAD) {
        int uy = SHELL_TITLE_Y;
        DrawText("VitaDeck Runtime Upload", SHELL_PAD_L, uy, SHELL_FONT_TITLE, RAYWHITE);
        uy += SHELL_FONT_TITLE + SHELL_SPACE_MD;
        if (shell->bind_error[0] != '\0') {
            DrawText("Upload listener could not start.", SHELL_PAD_L, uy, SHELL_FONT_BODY, RED);
            uy += SHELL_FONT_BODY + SHELL_SPACE_SM;
            DrawText(shell->bind_error, SHELL_PAD_L, uy, SHELL_FONT_BODY, RAYWHITE);
            uy += SHELL_FONT_BODY + SHELL_SPACE_MD;
        } else {
            DrawText("Open this URL from a browser on your LAN:", SHELL_PAD_L, uy, SHELL_FONT_BODY, RAYWHITE);
            uy += SHELL_FONT_BODY + SHELL_SPACE_SM;
            DrawText(upload_server_url(&shell->upload_server), SHELL_PAD_L, uy, SHELL_FONT_BODY, GREEN);
            uy += SHELL_FONT_BODY + SHELL_SPACE_MD;
            DrawText("Upload a zip containing exactly one .vdapp directory.", SHELL_PAD_L, uy, SHELL_FONT_BODY, RAYWHITE);
            uy += SHELL_FONT_BODY + SHELL_SPACE_MD;
        }
        if (shell->message[0] != '\0') {
            DrawText(shell->message, SHELL_PAD_L, uy, SHELL_FONT_BODY, YELLOW);
            uy += SHELL_FONT_BODY + SHELL_SPACE_MD;
        }
        const int btn_h = 40;
        const int btn_w = 200;
        int btn_y = SHELL_FOOTER_Y - SHELL_SPACE_MD - btn_h;
        DrawRectangle(SHELL_PAD_L, btn_y, btn_w, btn_h, (Color){ 80, 95, 120, 255 });
        DrawRectangleLinesEx((Rectangle){ (float)SHELL_PAD_L, (float)btn_y, (float)btn_w, (float)btn_h }, 1.0f, (Color){ 120, 150, 190, 255 });
        int btn_text_y = btn_y + (btn_h - SHELL_FONT_BODY) / 2;
        DrawText("Cancel upload", SHELL_PAD_L + SHELL_SPACE_MD, btn_text_y, SHELL_FONT_BODY, RAYWHITE);
        shell_draw_footer_hint();
        return;
    }

    DrawText("VitaDeck Shell", SHELL_PAD_L, SHELL_TITLE_Y, SHELL_FONT_TITLE, RAYWHITE);
    DrawText("Deck Apps", SHELL_PAD_L, SHELL_SECTION_Y, SHELL_FONT_BODY, GRAY);

    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    int row_count = package_count + 1;

    int row_top = SHELL_LIST_Y;
    for (int row = 0; row < row_count; row++) {
        bool row_focus = row == shell->focus_row && shell->remove_confirm_package[0] == '\0';
        bool upload_row = row_is_upload(row, package_count);

        int row_w = SCREEN_WIDTH - SHELL_PAD_L - SHELL_PAD_R;
        int band_h = upload_row ? SHELL_UPLOAD_ROW_H : SHELL_ROW_H;
        DrawRectangle(SHELL_PAD_L, row_top, row_w, band_h,
            row_focus ? (Color){ 42, 52, 72, 255 } : (Color){ 28, 34, 44, 255 });

        int text_y = shell_text_baseline_y(row_top, SHELL_FONT_BODY);
        if (upload_row) text_y = row_top + SHELL_SPACE_MD;

        if (upload_row) {
            int pad = SHELL_SPACE_MD;
            DrawText("Runtime Upload", SHELL_NAME_X + pad, text_y, SHELL_FONT_BODY, RAYWHITE);
            int sub_y = text_y + SHELL_FONT_BODY + SHELL_SPACE_XS;
            if (upload_server_is_running(&shell->upload_server)) {
                DrawText(upload_server_url(&shell->upload_server), SHELL_NAME_X + pad, sub_y, SHELL_FONT_CAPTION, GREEN);
            } else {
                DrawText("Enter to start listener", SHELL_NAME_X + pad, sub_y, SHELL_FONT_CAPTION, (Color){ 130, 140, 160, 255 });
            }
        } else {
            const VdPackageInfo *pkg = &packages[row];
            char name_line[VD_DISPLAY_NAME_MAX + VD_VERSION_MAX + 12];
            snprintf(name_line, sizeof(name_line), "%s   %s", pkg->display_name, pkg->version);
            Color name_color = pkg->is_active ? GREEN : RAYWHITE;
            DrawText(name_line, SHELL_NAME_X + SHELL_SPACE_MD, text_y, SHELL_FONT_BODY, name_color);
            draw_tick_cell(SHELL_TICK_CX, row_top, pkg->is_active, row_focus, shell->focus_col == 0);
            draw_x_cell(SHELL_X_CX, row_top, row_focus, shell->focus_col == 1);
        }
        row_top += band_h + SHELL_SPACE_SM;
    }

    if (shell->remove_confirm_package[0] != '\0') {
        int box_y = (int)(SCREEN_HEIGHT * 0.40f);
        DrawRectangle(0, box_y, SCREEN_WIDTH, 112, (Color){ 10, 12, 18, 255 });
        int ly = box_y + SHELL_SPACE_MD;
        DrawText(shell->remove_confirm_label, SHELL_PAD_L, ly, SHELL_FONT_BODY, RAYWHITE);
        DrawText("Enter: remove · Back: cancel", SHELL_PAD_L, ly + SHELL_FONT_BODY + SHELL_SPACE_SM, SHELL_FONT_CAPTION, GRAY);
    }

    if (shell->message[0] != '\0') {
        int msg_y = SHELL_FOOTER_Y - SHELL_SPACE_MD - SHELL_FONT_BODY;
        DrawText(shell->message, SHELL_PAD_L, msg_y, SHELL_FONT_BODY, YELLOW);
    }
    shell_draw_footer_hint();
}
