#include "shell/shell.h"

#include <raylib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "core/package_library.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define SHELL_INSET 24
#define SHELL_PADDING (SHELL_INSET + 8)

#define SHELL_FONT_TITLE 24
#define SHELL_FONT_BODY 18
#define SHELL_FONT_CAPTION 16

#define SHELL_SPACE_XS 6
#define SHELL_SPACE_SM 8
#define SHELL_SPACE_MD 16

#define SHELL_FOOTER_Y (SCREEN_HEIGHT - SHELL_PADDING - SHELL_FONT_CAPTION)

#define SHELL_TITLE_Y SHELL_PADDING
#define SHELL_SECTION_Y (SHELL_TITLE_Y + SHELL_FONT_TITLE + SHELL_SPACE_SM)
#define SHELL_LIST_Y (SHELL_SECTION_Y + SHELL_FONT_BODY + SHELL_SPACE_MD)
#define SHELL_LIST_BOTTOM (SHELL_FOOTER_Y - SHELL_SPACE_MD - SHELL_FONT_BODY - SHELL_SPACE_MD)

#define SHELL_ROW_H 50
#define SHELL_ROW_STEP (SHELL_ROW_H + SHELL_SPACE_SM)
#define SHELL_UPLOAD_ROW_H (SHELL_SPACE_MD + SHELL_FONT_BODY + SHELL_SPACE_XS + SHELL_FONT_CAPTION + SHELL_SPACE_MD)
#define SHELL_OVERFLOW_ROW_H 18
#define SHELL_OVERFLOW_ROW_STEP (SHELL_OVERFLOW_ROW_H + SHELL_SPACE_SM)
#define SHELL_NAME_X SHELL_PADDING
#define SHELL_X_CX (SCREEN_WIDTH - SHELL_PADDING - 28)
#define SHELL_TICK_CX (SHELL_X_CX - 80)

static int shell_text_baseline_y(int row_top, int font_h)
{
    return row_top + (SHELL_ROW_H - font_h) / 2;
}

static void shell_draw_footer_hint(void)
{
    DrawText("Start / F1 · Confirm · Back", SHELL_PADDING, SHELL_FOOTER_Y, SHELL_FONT_CAPTION, GRAY);
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
    return n + 2;
}

static bool row_is_upload(int row, int package_count)
{
    return row == package_count;
}

static bool row_is_host_control(int row, int package_count)
{
    return row == package_count + 1;
}

static int row_band_height(int row, int package_count)
{
    return (row_is_upload(row, package_count) || row_is_host_control(row, package_count)) ? SHELL_UPLOAD_ROW_H : SHELL_ROW_H;
}

static int shell_visible_rows_from(int start_row, int package_count, int row_count)
{
    if (row_count <= 0) return 0;
    if (start_row < 0) start_row = 0;
    if (start_row >= row_count) start_row = row_count - 1;

    int available = SHELL_LIST_BOTTOM - SHELL_LIST_Y;
    if (start_row > 0) available -= SHELL_OVERFLOW_ROW_STEP;
    if (available <= 0) return 1;

    int used = 0;
    int visible = 0;
    for (int row = start_row; row < row_count; row++) {
        int band_h = row_band_height(row, package_count);
        int cost = band_h + (visible > 0 ? SHELL_SPACE_SM : 0);
        int bottom_reserve = row + 1 < row_count ? SHELL_OVERFLOW_ROW_STEP : 0;
        if (used + cost + bottom_reserve > available) break;
        used += cost;
        visible++;
    }
    return visible > 0 ? visible : 1;
}

static int clamp_scroll_row(int scroll_row, int row_count)
{
    if (row_count <= 0) return 0;
    if (scroll_row < 0) return 0;
    if (scroll_row >= row_count) return row_count - 1;
    return scroll_row;
}

static void ensure_focus_row_visible(VdShell *shell, int package_count, int row_count)
{
    shell->scroll_row = clamp_scroll_row(shell->scroll_row, row_count);
    int visible_rows = shell_visible_rows_from(shell->scroll_row, package_count, row_count);
    if (shell->focus_row < shell->scroll_row) {
        shell->scroll_row = shell->focus_row;
    } else if (shell->focus_row >= shell->scroll_row + visible_rows) {
        shell->scroll_row = shell->focus_row - visible_rows + 1;
    }
    shell->scroll_row = clamp_scroll_row(shell->scroll_row, row_count);
}

static void clear_remove_confirm(VdShell *shell)
{
    shell->remove_confirm_package[0] = '\0';
    shell->remove_confirm_label[0] = '\0';
    shell->remove_confirm_row = -1;
}

static bool start_upload_server(VdShell *shell)
{
    shell->bind_error[0] = '\0';
    char error[256];
    if (!upload_server_start(&shell->upload_server, error, sizeof(error))) {
        snprintf(shell->bind_error, sizeof(shell->bind_error), "%s", error);
        snprintf(shell->message, sizeof(shell->message), "%s", error);
        return false;
    }
    snprintf(shell->message, sizeof(shell->message), "Upload server started.");
    return true;
}

static void toggle_upload_server(VdShell *shell)
{
    if (upload_server_is_running(&shell->upload_server)) {
        upload_server_stop(&shell->upload_server);
        shell->bind_error[0] = '\0';
        snprintf(shell->message, sizeof(shell->message), "Upload server stopped.");
        return;
    }
    start_upload_server(shell);
}

static void enter_host_control_settings(VdShell *shell)
{
    if (!settings_get_host_control_url(shell->host_control_url, sizeof(shell->host_control_url))) {
        snprintf(shell->host_control_url, sizeof(shell->host_control_url), "http://192.168.1.100:8797");
    }
    shell->host_control_cursor = (int)strlen(shell->host_control_url);
    shell->state = VD_SHELL_HOST_CONTROL;
    snprintf(shell->message, sizeof(shell->message), "Edit Host Control address.");
}

static bool host_control_cancel_pressed(void)
{
    return IsKeyPressed(KEY_ESCAPE) || (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT));
}

static int host_control_char_index(char c)
{
    const char *chars = "0123456789.:/abcdefghijklmnopqrstuvwxyz-";
    const char *p = strchr(chars, (int)c);
    return p ? (int)(p - chars) : 0;
}

static char host_control_cycle_char(char current, int delta)
{
    const char *chars = "0123456789.:/abcdefghijklmnopqrstuvwxyz-";
    int count = (int)strlen(chars);
    int index = host_control_char_index(current);
    index = (index + delta + count) % count;
    return chars[index];
}

static void host_control_insert_char(VdShell *shell, char c)
{
    size_t len = strlen(shell->host_control_url);
    if (len + 1 >= sizeof(shell->host_control_url)) return;
    if (shell->host_control_cursor < 0) shell->host_control_cursor = 0;
    if (shell->host_control_cursor > (int)len) shell->host_control_cursor = (int)len;
    memmove(shell->host_control_url + shell->host_control_cursor + 1,
        shell->host_control_url + shell->host_control_cursor,
        len - (size_t)shell->host_control_cursor + 1);
    shell->host_control_url[shell->host_control_cursor++] = c;
}

static void host_control_delete_char(VdShell *shell)
{
    size_t len = strlen(shell->host_control_url);
    if (len == 0 || shell->host_control_cursor <= 0) return;
    if (shell->host_control_cursor > (int)len) shell->host_control_cursor = (int)len;
    memmove(shell->host_control_url + shell->host_control_cursor - 1,
        shell->host_control_url + shell->host_control_cursor,
        len - (size_t)shell->host_control_cursor + 1);
    shell->host_control_cursor--;
}

static void save_host_control_settings(VdShell *shell)
{
    char error[256];
    if (!settings_set_host_control_url(shell->host_control_url, error, sizeof(error))) {
        snprintf(shell->message, sizeof(shell->message), "%s", error);
        return;
    }
    shell->state = VD_SHELL_HOME;
    snprintf(shell->message, sizeof(shell->message), "Saved Host Control address.");
}

static void hide_shell_to_deck(VdShell *shell)
{
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
    shell->state = VD_SHELL_HOME;
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
    char message[256];
    upload_server_last_message(&shell->upload_server, message, sizeof(message));
    if (message[0] != '\0') snprintf(shell->message, sizeof(shell->message), "%s", message);

    bool restart_active = false;
    VdPackageInfo info;
    if (upload_server_take_success(&shell->upload_server, &restart_active, &info)) {
        snprintf(shell->message, sizeof(shell->message), "Installed %s %s.", info.display_name, info.version);
        if (restart_active) *request_runtime_restart = true;
    }
}

static void poll_host_control_settings(VdShell *shell)
{
    size_t len = strlen(shell->host_control_url);
    if (shell->host_control_cursor < 0) shell->host_control_cursor = 0;
    if (shell->host_control_cursor > (int)len) shell->host_control_cursor = (int)len;

    int ch = GetCharPressed();
    while (ch > 0) {
        if (ch >= 32 && ch <= 126 && !isspace(ch)) host_control_insert_char(shell, (char)ch);
        ch = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) host_control_delete_char(shell);
    if (left_pressed() && shell->host_control_cursor > 0) shell->host_control_cursor--;
    if (right_pressed() && shell->host_control_cursor < (int)strlen(shell->host_control_url)) shell->host_control_cursor++;

    bool cycle_up = up_pressed();
    bool cycle_down = down_pressed();
    if (cycle_up || cycle_down) {
        len = strlen(shell->host_control_url);
        if (len == 0) host_control_insert_char(shell, '0');
        int index = shell->host_control_cursor;
        if (index >= (int)len) index = (int)len - 1;
        if (index >= 0) {
            shell->host_control_url[index] = host_control_cycle_char(shell->host_control_url[index], cycle_up ? 1 : -1);
        }
    }

    if (host_control_cancel_pressed()) {
        shell->state = VD_SHELL_HOME;
        snprintf(shell->message, sizeof(shell->message), "Host Control address unchanged.");
        return;
    }

    if (confirm_pressed()) save_host_control_settings(shell);
}

void shell_poll_system_input(VdShell *shell, bool *request_runtime_restart)
{
    if (start_pressed()) {
        if (shell->state == VD_SHELL_HIDDEN) {
            shell->state = VD_SHELL_HOME;
            shell->focus_row = 0;
            shell->scroll_row = 0;
            shell->focus_col = 0;
        } else if (package_library_has_active_deck_app()) {
            hide_shell_to_deck(shell);
        }
    }

    if (shell->state == VD_SHELL_HIDDEN) return;
    if (shell->state == VD_SHELL_HOST_CONTROL) {
        poll_host_control_settings(shell);
        return;
    }

    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    int row_count = package_count + 2;
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

    if (row_is_upload(shell->focus_row, package_count) || row_is_host_control(shell->focus_row, package_count)) {
        shell->focus_col = 0;
    } else {
        if (left_pressed() && shell->focus_col > 0) shell->focus_col--;
        if (right_pressed() && shell->focus_col < 1) shell->focus_col++;
    }

    ensure_focus_row_visible(shell, package_count, row_count);

    if (back_pressed()) {
        if (package_library_has_active_deck_app()) hide_shell_to_deck(shell);
        return;
    }

    if (!confirm_pressed()) return;

    if (row_is_upload(shell->focus_row, package_count)) {
        toggle_upload_server(shell);
        return;
    }
    if (row_is_host_control(shell->focus_row, package_count)) {
        enter_host_control_settings(shell);
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
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 18, 22, 30, 200 });
    DrawRectangleLines(24, 24, SCREEN_WIDTH - 48, SCREEN_HEIGHT - 48, (Color){ 120, 150, 190, 200 });
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

static void draw_overflow_row(int row_top, bool points_up)
{
    int row_w = SCREEN_WIDTH - SHELL_PADDING - SHELL_PADDING;
    DrawRectangle(SHELL_PADDING, row_top, row_w, SHELL_OVERFLOW_ROW_H, (Color){ 24, 30, 40, 255 });

    Color chevron = (Color){ 120, 140, 170, 255 };
    int cy = row_top + SHELL_OVERFLOW_ROW_H / 2;
    for (int i = -1; i <= 1; i++) {
        int cx = SCREEN_WIDTH / 2 + i * 20;
        if (points_up) {
            DrawLineEx((Vector2){ (float)(cx - 6), (float)(cy + 3) },
                (Vector2){ (float)cx, (float)(cy - 4) },
                2.0f,
                chevron);
            DrawLineEx((Vector2){ (float)cx, (float)(cy - 4) },
                (Vector2){ (float)(cx + 6), (float)(cy + 3) },
                2.0f,
                chevron);
        } else {
            DrawLineEx((Vector2){ (float)(cx - 6), (float)(cy - 3) },
                (Vector2){ (float)cx, (float)(cy + 4) },
                2.0f,
                chevron);
            DrawLineEx((Vector2){ (float)cx, (float)(cy + 4) },
                (Vector2){ (float)(cx + 6), (float)(cy - 3) },
                2.0f,
                chevron);
        }
    }
}

static void draw_host_control_settings(VdShell *shell)
{
    draw_panel();
    DrawText("Host Control", SHELL_PADDING, SHELL_TITLE_Y, SHELL_FONT_TITLE, RAYWHITE);
    DrawText("Console Address", SHELL_PADDING, SHELL_SECTION_Y, SHELL_FONT_BODY, GRAY);

    int box_y = SHELL_LIST_Y;
    int box_w = SCREEN_WIDTH - SHELL_PADDING - SHELL_PADDING;
    DrawRectangle(SHELL_PADDING, box_y, box_w, 60, (Color){ 28, 34, 44, 255 });
    DrawRectangleLines(SHELL_PADDING, box_y, box_w, 60, (Color){ 100, 120, 150, 255 });

    int text_y = box_y + 18;
    DrawText(shell->host_control_url, SHELL_PADDING + SHELL_SPACE_MD, text_y, SHELL_FONT_BODY, RAYWHITE);
    int cursor_x = SHELL_PADDING + SHELL_SPACE_MD + MeasureText(TextSubtext(shell->host_control_url, 0, shell->host_control_cursor), SHELL_FONT_BODY);
    DrawLine(cursor_x, text_y - 2, cursor_x, text_y + SHELL_FONT_BODY + 2, YELLOW);

    int hint_y = box_y + 80;
    DrawText("Keyboard: type address, Backspace delete, Enter save, Esc cancel.", SHELL_PADDING, hint_y, SHELL_FONT_CAPTION, (Color){ 150, 160, 180, 255 });
    hint_y += SHELL_FONT_CAPTION + SHELL_SPACE_XS;
    DrawText("Gamepad/Vita: Left/Right cursor, Up/Down cycle character, Cross save, Circle cancel.", SHELL_PADDING, hint_y, SHELL_FONT_CAPTION, (Color){ 150, 160, 180, 255 });

    if (shell->message[0] != '\0') {
        int msg_y = SHELL_FOOTER_Y - SHELL_SPACE_MD - SHELL_FONT_BODY;
        DrawText(shell->message, SHELL_PADDING, msg_y, SHELL_FONT_BODY, YELLOW);
    }
}

void shell_render(VdShell *shell)
{
    if (shell->state == VD_SHELL_HIDDEN) return;
    if (shell->state == VD_SHELL_HOST_CONTROL) {
        draw_host_control_settings(shell);
        return;
    }
    draw_panel();

    DrawText("VitaDeck", SHELL_PADDING, SHELL_TITLE_Y, SHELL_FONT_TITLE, RAYWHITE);
    DrawText("Deck Apps", SHELL_PADDING, SHELL_SECTION_Y, SHELL_FONT_BODY, GRAY);

    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    int row_count = package_count + 2;

    int list_top = SHELL_LIST_Y;
    if (package_count == 0) {
        int hint_y = SHELL_SECTION_Y + SHELL_FONT_BODY + SHELL_SPACE_XS;
        DrawText("No Deck Apps installed yet.", SHELL_PADDING, hint_y, SHELL_FONT_CAPTION, (Color){ 150, 160, 180, 255 });
        hint_y += SHELL_FONT_CAPTION + SHELL_SPACE_XS;
        DrawText("Select Upload, confirm to start the server, then open the URL shown below on", SHELL_PADDING, hint_y, SHELL_FONT_CAPTION, (Color){ 130, 140, 160, 255 });
        hint_y += SHELL_FONT_CAPTION + SHELL_SPACE_XS;
        DrawText("another device on this network and upload a zip containing one .vdapp folder.", SHELL_PADDING, hint_y, SHELL_FONT_CAPTION, (Color){ 130, 140, 160, 255 });
        list_top = hint_y + SHELL_FONT_CAPTION + SHELL_SPACE_MD;
    }

    shell->scroll_row = clamp_scroll_row(shell->scroll_row, row_count);

    int row_top = list_top;
    if (shell->scroll_row > 0) {
        draw_overflow_row(row_top, true);
        row_top += SHELL_OVERFLOW_ROW_STEP;
    }

    int end_row = shell->scroll_row;
    for (int row = shell->scroll_row; row < row_count; row++) {
        bool row_focus = row == shell->focus_row && shell->remove_confirm_package[0] == '\0';
        bool upload_row = row_is_upload(row, package_count);
        bool host_control_row = row_is_host_control(row, package_count);
        bool upload_running = upload_row && upload_server_is_running(&shell->upload_server);

        int row_w = SCREEN_WIDTH - SHELL_PADDING - SHELL_PADDING;
        int band_h = row_band_height(row, package_count);
        int bottom_reserve = row + 1 < row_count ? SHELL_OVERFLOW_ROW_STEP : 0;
        if (row_top + band_h + bottom_reserve > SHELL_LIST_BOTTOM) break;
        Color band_color = row_focus ? (Color){ 42, 52, 72, 255 } : (Color){ 28, 34, 44, 255 };
        if (upload_running) band_color = row_focus ? (Color){ 34, 64, 58, 255 } : (Color){ 24, 48, 44, 255 };
        DrawRectangle(SHELL_PADDING, row_top, row_w, band_h,
            band_color);

        int text_y = shell_text_baseline_y(row_top, SHELL_FONT_BODY);
        if (upload_row) text_y = row_top + SHELL_SPACE_MD;

        if (upload_row) {
            int pad = SHELL_SPACE_MD;
            DrawText("Upload", SHELL_NAME_X + pad, text_y, SHELL_FONT_BODY, upload_running ? GREEN : RAYWHITE);
            int sub_y = text_y + SHELL_FONT_BODY + SHELL_SPACE_XS;
            if (upload_running) {
                DrawText(upload_server_url(&shell->upload_server), SHELL_NAME_X + pad, sub_y, SHELL_FONT_CAPTION, GREEN);
            } else {
                DrawText("Confirm to start server", SHELL_NAME_X + pad, sub_y, SHELL_FONT_CAPTION, (Color){ 130, 140, 160, 255 });
            }
        } else if (host_control_row) {
            int pad = SHELL_SPACE_MD;
            char url[VD_HOST_CONTROL_URL_MAX];
            if (!settings_get_host_control_url(url, sizeof(url))) snprintf(url, sizeof(url), "Not configured");
            DrawText("Host Control", SHELL_NAME_X + pad, text_y, SHELL_FONT_BODY, RAYWHITE);
            int sub_y = text_y + SHELL_FONT_BODY + SHELL_SPACE_XS;
            DrawText(url, SHELL_NAME_X + pad, sub_y, SHELL_FONT_CAPTION, (Color){ 130, 140, 160, 255 });
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
        end_row = row + 1;
    }

    if (end_row < row_count) draw_overflow_row(row_top, false);

    if (shell->remove_confirm_package[0] != '\0') {
        int box_y = (int)(SCREEN_HEIGHT * 0.40f);
        DrawRectangle(0, box_y, SCREEN_WIDTH, 112, (Color){ 10, 12, 18, 255 });
        int ly = box_y + SHELL_SPACE_MD;
        DrawText(shell->remove_confirm_label, SHELL_PADDING, ly, SHELL_FONT_BODY, RAYWHITE);
        DrawText("Confirm: remove · Back: cancel", SHELL_PADDING, ly + SHELL_FONT_BODY + SHELL_SPACE_SM, SHELL_FONT_CAPTION, GRAY);
    }

    if (shell->message[0] != '\0') {
        int msg_y = SHELL_FOOTER_Y - SHELL_SPACE_MD - SHELL_FONT_BODY;
        DrawText(shell->message, SHELL_PADDING, msg_y, SHELL_FONT_BODY, YELLOW);
    }
    shell_draw_footer_hint();
}
