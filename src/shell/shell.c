#include "shell/shell.h"

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
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

#define SHELL_FOOTER_HINT_Y (SCREEN_HEIGHT - SHELL_PADDING - SHELL_FONT_CAPTION)
#define SHELL_LAN_STRIP_Y (SHELL_FOOTER_HINT_Y - SHELL_SPACE_MD - SHELL_FONT_CAPTION - SHELL_SPACE_XS - SHELL_FONT_CAPTION)

#define SHELL_TITLE_Y SHELL_PADDING
#define SHELL_SECTION_Y (SHELL_TITLE_Y + SHELL_FONT_TITLE + SHELL_SPACE_SM)
#define SHELL_LIST_Y (SHELL_SECTION_Y + SHELL_FONT_BODY + SHELL_SPACE_MD)
#define SHELL_LIST_BOTTOM (SHELL_LAN_STRIP_Y - SHELL_SPACE_MD)

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

static void shell_draw_lan_strip(const VdShell *shell)
{
    int y = SHELL_LAN_STRIP_Y;
    DrawText("LAN", SHELL_PADDING, y, SHELL_FONT_CAPTION, (Color){ 140, 160, 190, 255 });
    y += SHELL_FONT_CAPTION + SHELL_SPACE_XS;
    if (upload_server_is_running(&shell->upload_server)) {
        DrawText(upload_server_url(&shell->upload_server), SHELL_PADDING, y, SHELL_FONT_CAPTION, GREEN);
    } else if (shell->bind_error[0] != '\0') {
        DrawText(shell->bind_error, SHELL_PADDING, y, SHELL_FONT_CAPTION, (Color){ 220, 120, 120, 255 });
    } else {
        DrawText("Starting network listener...", SHELL_PADDING, y, SHELL_FONT_CAPTION, (Color){ 130, 140, 160, 255 });
    }
}

static void shell_draw_footer_hint(void)
{
    DrawText("Start / F1 · Confirm · Back", SHELL_PADDING, SHELL_FOOTER_HINT_Y, SHELL_FONT_CAPTION, GRAY);
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
    return package_library_list(packages, VD_PACKAGE_LIST_MAX);
}

static int row_band_height(int row, int package_count)
{
    (void)row;
    (void)package_count;
    return SHELL_ROW_H;
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
    char error[256];
    if (!upload_server_start(&shell->upload_server, error, sizeof(error))) {
        snprintf(shell->bind_error, sizeof(shell->bind_error), "%s", error);
    }
}

bool shell_try_auto_activate(VdShell *shell, bool *request_runtime_restart)
{
    const char *package_name = getenv("VITADECK_AUTO_ACTIVATE");
    if (!package_name || package_name[0] == '\0') return false;
    activate_package(shell, package_name, request_runtime_restart);
    return true;
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

    if (left_pressed() && shell->focus_col > 0) shell->focus_col--;
    if (right_pressed() && shell->focus_col < 1) shell->focus_col++;

    ensure_focus_row_visible(shell, package_count, row_count);

    if (back_pressed()) {
        if (package_library_has_active_deck_app()) hide_shell_to_deck(shell);
        return;
    }

    if (!confirm_pressed()) return;

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

void shell_render(VdShell *shell)
{
    if (shell->state == VD_SHELL_HIDDEN) return;
    draw_panel();

    DrawText("VitaDeck", SHELL_PADDING, SHELL_TITLE_Y, SHELL_FONT_TITLE, RAYWHITE);
    DrawText("Deck Apps", SHELL_PADDING, SHELL_SECTION_Y, SHELL_FONT_BODY, GRAY);

    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    int row_count = package_count + 1;

    int list_top = SHELL_LIST_Y;
    if (package_count == 0) {
        int hint_y = SHELL_SECTION_Y + SHELL_FONT_BODY + SHELL_SPACE_XS;
        DrawText("No Deck Apps installed yet.", SHELL_PADDING, hint_y, SHELL_FONT_CAPTION, (Color){ 150, 160, 180, 255 });
        hint_y += SHELL_FONT_CAPTION + SHELL_SPACE_XS;
        DrawText("Use the LAN URL below to upload a .vdapp zip or pair Host Control.", SHELL_PADDING, hint_y, SHELL_FONT_CAPTION, (Color){ 130, 140, 160, 255 });
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
        int row_w = SCREEN_WIDTH - SHELL_PADDING - SHELL_PADDING;
        int band_h = row_band_height(row, package_count);
        int bottom_reserve = row + 1 < row_count ? SHELL_OVERFLOW_ROW_STEP : 0;
        if (row_top + band_h + bottom_reserve > SHELL_LIST_BOTTOM) break;
        Color band_color = row_focus ? (Color){ 42, 52, 72, 255 } : (Color){ 28, 34, 44, 255 };
        DrawRectangle(SHELL_PADDING, row_top, row_w, band_h, band_color);

        int text_y = shell_text_baseline_y(row_top, SHELL_FONT_BODY);
        const VdPackageInfo *pkg = &packages[row];
        char name_line[VD_DISPLAY_NAME_MAX + VD_VERSION_MAX + 12];
        snprintf(name_line, sizeof(name_line), "%s   %s", pkg->display_name, pkg->version);
        Color name_color = pkg->is_active ? GREEN : RAYWHITE;
        DrawText(name_line, SHELL_NAME_X + SHELL_SPACE_MD, text_y, SHELL_FONT_BODY, name_color);
        draw_tick_cell(SHELL_TICK_CX, row_top, pkg->is_active, row_focus, shell->focus_col == 0);
        draw_x_cell(SHELL_X_CX, row_top, row_focus, shell->focus_col == 1);
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
        int msg_y = SHELL_LAN_STRIP_Y - SHELL_SPACE_MD - SHELL_FONT_BODY;
        DrawText(shell->message, SHELL_PADDING, msg_y, SHELL_FONT_BODY, YELLOW);
    }
    shell_draw_lan_strip(shell);
    shell_draw_footer_hint();
}
