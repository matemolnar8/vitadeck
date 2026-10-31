#include "shell/shell.h"

#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include "core/bootstrap.h"
#include "core/package_library.h"
#include "host_control/link.h"
#include "ui/input.h"

#define SHELL_INSET 24
#define SHELL_PADDING (SHELL_INSET + 8)

#define SHELL_FONT_TITLE 24
#define SHELL_FONT_BODY 18
#define SHELL_FONT_CAPTION 16

#define SHELL_SPACE_XS 6
#define SHELL_SPACE_SM 8
#define SHELL_SPACE_MD 16

#define SHELL_FOOTER_Y (VD_SCREEN_HEIGHT - SHELL_PADDING - SHELL_FONT_CAPTION)

#define SHELL_TITLE_Y SHELL_PADDING
#define SHELL_SECTION_Y (SHELL_TITLE_Y + SHELL_FONT_TITLE + SHELL_SPACE_SM)
#define SHELL_LIST_Y (SHELL_SECTION_Y + SHELL_FONT_BODY + SHELL_SPACE_MD)
#define SHELL_LIST_BOTTOM (SHELL_FOOTER_Y - SHELL_SPACE_MD - SHELL_FONT_BODY - SHELL_SPACE_MD)

#define SHELL_ROW_H 50
#define SHELL_ROW_STEP (SHELL_ROW_H + SHELL_SPACE_SM)
#define SHELL_ENTRY_ROW_H (SHELL_SPACE_MD + SHELL_FONT_BODY + SHELL_SPACE_XS + SHELL_FONT_CAPTION + SHELL_SPACE_MD)
#define SHELL_OVERFLOW_ROW_H 18
#define SHELL_OVERFLOW_ROW_STEP (SHELL_OVERFLOW_ROW_H + SHELL_SPACE_SM)
#define SHELL_NAME_X SHELL_PADDING
#define SHELL_X_CX (VD_SCREEN_WIDTH - SHELL_PADDING - 28)
#define SHELL_TICK_CX (SHELL_X_CX - 80)

static int shell_text_baseline_y(int row_top, int font_h)
{
    return row_top + (SHELL_ROW_H - font_h) / 2;
}

static void shell_draw_footer_hint(const char *hint)
{
    DrawText(hint, SHELL_PADDING, SHELL_FOOTER_Y, SHELL_FONT_CAPTION, GRAY);
}

static bool gamepad_pressed(bool available, int button)
{
    return available && IsGamepadButtonPressed(0, button);
}

static void draw_checkmark(int cx, int cy, Color c)
{
    DrawLine(cx - 6, cy, cx - 1, cy + 6, c);
    DrawLine(cx - 1, cy + 6, cx + 10, cy - 8, c);
}

// The Home rows: the packages, then the Shell Listener Entry, then the Shell Host Control Entry.
#define SHELL_ENTRY_ROWS 2

static int shell_row_count(void)
{
    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int n = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    return n + SHELL_ENTRY_ROWS;
}

static bool row_is_listener(int row, int package_count)
{
    return row == package_count;
}

static bool row_is_host_control(int row, int package_count)
{
    return row == package_count + 1;
}

static int row_band_height(int row, int package_count)
{
    return row < package_count ? SHELL_ROW_H : SHELL_ENTRY_ROW_H;
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

// The packages' first row, or with none the Host Control row, so a first Cross does not stop the listener.
static int home_first_row(void)
{
    int rows = shell_row_count();
    return rows == SHELL_ENTRY_ROWS ? rows - 1 : 0;
}

// Uploads add package rows above the entries; keep focus on the entry it was on.
static void keep_entry_focus(VdShell *shell)
{
    int count = shell_row_count() - SHELL_ENTRY_ROWS;
    if (shell->focus_row >= shell->home_package_count) shell->focus_row += count - shell->home_package_count;
    shell->home_package_count = count;
}

static void clear_remove_confirm(VdShell *shell)
{
    shell->remove_confirm_package[0] = '\0';
    shell->remove_confirm_label[0] = '\0';
    shell->remove_confirm_row = -1;
}

static bool start_listener(VdShell *shell)
{
    char error[256];
    if (!lan_listener_start(&shell->listener, error, sizeof(error))) {
        snprintf(shell->message, sizeof(shell->message), "%s", error);
        return false;
    }
    return true;
}

static void toggle_listener(VdShell *shell)
{
    if (lan_listener_is_running(&shell->listener)) {
        lan_listener_stop(&shell->listener);
        snprintf(shell->message, sizeof(shell->message), "Stopped listening for uploads and links.");
        return;
    }
    if (start_listener(shell)) {
        snprintf(shell->message, sizeof(shell->message), "Listening for uploads and links.");
    }
}

static void open_host_control(VdShell *shell)
{
    shell->state = VD_SHELL_HOST_CONTROL;
    shell->host_control_focus = 0;
    shell->message[0] = '\0';
}

static void close_host_control(VdShell *shell)
{
    shell->state = VD_SHELL_HOME;
    shell->message[0] = '\0';
    int rows = shell_row_count();
    shell->focus_row = rows - 1;
    shell->home_package_count = rows - SHELL_ENTRY_ROWS;
    shell->focus_col = 0;
}

static void forget_focused_link(VdShell *shell)
{
    VdHostControlLink links[VD_HOST_CONTROL_MAX_LINKS];
    int count = host_control_links_list(links, VD_HOST_CONTROL_MAX_LINKS);
    if (shell->host_control_focus >= count) return;
    const VdHostControlLink *link = &links[shell->host_control_focus];
    bool forgot = host_control_links_forget(link->id);
    snprintf(shell->message, sizeof(shell->message), forgot ? "Forgot %s." : "Could not forget %s.", link->name);
    if (forgot && shell->host_control_focus >= count - 1) {
        shell->host_control_focus = count > 1 ? count - 2 : 0;
    }
}

static void apply_host_control_input(VdShell *shell, const VdShellInput *input)
{
    if (input->back) {
        close_host_control(shell);
        return;
    }
    VdHostControlLink links[VD_HOST_CONTROL_MAX_LINKS];
    int count = host_control_links_list(links, VD_HOST_CONTROL_MAX_LINKS);
    if (input->up && shell->host_control_focus > 0) shell->host_control_focus--;
    if (input->down && shell->host_control_focus + 1 < count) shell->host_control_focus++;
    if (input->confirm) forget_focused_link(shell);
}

static void hide_shell_to_deck(VdShell *shell)
{
    shell->state = VD_SHELL_HIDDEN;
    clear_remove_confirm(shell);
    shell->message[0] = '\0';
    input_ignore_held_pad_buttons();
}

static void activate_package(VdShell *shell, const VdPackageInfo *pkg, bool *request_runtime_restart)
{
    char error[256];
    if (package_library_set_active(pkg->package_name, error, sizeof(error))) {
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
    lan_listener_init(&shell->listener);
}

void shell_start_listener(VdShell *shell)
{
    start_listener(shell);
}

void shell_show_home(VdShell *shell)
{
    shell->state = VD_SHELL_HOME;
    shell->focus_row = home_first_row();
    shell->home_package_count = shell_row_count() - SHELL_ENTRY_ROWS;
    shell->scroll_row = 0;
    shell->focus_col = 0;
}

void shell_shutdown(VdShell *shell)
{
    lan_listener_destroy(&shell->listener);
}

bool shell_is_visible(const VdShell *shell)
{
    return shell->state != VD_SHELL_HIDDEN;
}

void shell_update(VdShell *shell, bool *request_runtime_restart)
{
    char message[256];
    if (lan_listener_take_message(&shell->listener, message, sizeof(message))) {
        snprintf(shell->message, sizeof(shell->message), "%s", message);
    }

    bool restart_active = false;
    VdPackageInfo info;
    if (lan_listener_take_success(&shell->listener, &restart_active, &info)) {
        snprintf(shell->message, sizeof(shell->message), "Installed %s %s.", info.display_name, info.version);
        if (restart_active) *request_runtime_restart = true;
    }
    if (shell->state == VD_SHELL_HOME) keep_entry_focus(shell);
}

void shell_poll_system_input(VdShell *shell, bool *request_runtime_restart)
{
    const bool pad = IsGamepadAvailable(0);
    const VdShellInput input = {
        .start = gamepad_pressed(pad, GAMEPAD_BUTTON_MIDDLE_RIGHT) || IsKeyPressed(KEY_F1),
        .confirm = gamepad_pressed(pad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsKeyPressed(KEY_ENTER),
        .back = gamepad_pressed(pad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || IsKeyPressed(KEY_BACKSPACE),
        .up = gamepad_pressed(pad, GAMEPAD_BUTTON_LEFT_FACE_UP) || IsKeyPressed(KEY_UP),
        .down = gamepad_pressed(pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || IsKeyPressed(KEY_DOWN),
        .left = gamepad_pressed(pad, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || IsKeyPressed(KEY_LEFT),
        .right = gamepad_pressed(pad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || IsKeyPressed(KEY_RIGHT),
    };
    shell_apply_system_input(shell, &input, request_runtime_restart);
}

void shell_apply_system_input(VdShell *shell, const VdShellInput *input, bool *request_runtime_restart)
{
    if (input->start) {
        if (shell->state == VD_SHELL_HIDDEN) {
            package_library_refresh();
            shell_show_home(shell);
        } else if (package_library_has_active_deck_app()) {
            hide_shell_to_deck(shell);
        }
    }

    if (shell->state == VD_SHELL_HIDDEN) return;
    if (shell->state == VD_SHELL_HOST_CONTROL) {
        apply_host_control_input(shell, input);
        return;
    }

    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    int row_count = package_count + SHELL_ENTRY_ROWS;

    if (shell->remove_confirm_package[0] != '\0') {
        if (input->back) {
            clear_remove_confirm(shell);
            return;
        }
        if (input->confirm) {
            remove_package_confirmed(shell, shell->remove_confirm_package, shell->remove_confirm_row);
        }
        return;
    }

    if (shell->focus_row >= row_count) shell->focus_row = row_count - 1;
    if (shell->focus_row < 0) shell->focus_row = 0;

    if (input->up && shell->focus_row > 0) {
        shell->focus_row--;
        shell->focus_col = 0;
    }
    if (input->down && shell->focus_row + 1 < row_count) {
        shell->focus_row++;
        shell->focus_col = 0;
    }

    if (shell->focus_row >= package_count) {
        shell->focus_col = 0;
    } else {
        if (input->left && shell->focus_col > 0) shell->focus_col--;
        if (input->right && shell->focus_col < 1) shell->focus_col++;
    }

    ensure_focus_row_visible(shell, package_count, row_count);

    if (input->back) {
        if (package_library_has_active_deck_app()) hide_shell_to_deck(shell);
        return;
    }

    if (!input->confirm) return;

    if (row_is_listener(shell->focus_row, package_count)) {
        toggle_listener(shell);
        return;
    }
    if (row_is_host_control(shell->focus_row, package_count)) {
        open_host_control(shell);
        return;
    }

    const VdPackageInfo *pkg = &packages[shell->focus_row];
    if (shell->focus_col == 0) {
        activate_package(shell, pkg, request_runtime_restart);
        return;
    }

    snprintf(shell->remove_confirm_package, sizeof(shell->remove_confirm_package), "%s", pkg->package_name);
    snprintf(shell->remove_confirm_label, sizeof(shell->remove_confirm_label), "Remove %s %s?", pkg->display_name,
             pkg->version);
    shell->remove_confirm_row = shell->focus_row;
}

static void draw_panel(void)
{
    DrawRectangle(0, 0, VD_SCREEN_WIDTH, VD_SCREEN_HEIGHT, (Color){18, 22, 30, 200});
    DrawRectangleLines(24, 24, VD_SCREEN_WIDTH - 48, VD_SCREEN_HEIGHT - 48, (Color){120, 150, 190, 200});
}

static void draw_tick_cell(int cx, int row_top, bool is_active, bool row_focused, bool col_focused)
{
    const int cell_h = 34;
    const int half = 22;
    int cell_y = row_top + (SHELL_ROW_H - cell_h) / 2;
    Rectangle r = {(float)(cx - half), (float)cell_y, (float)(half * 2), (float)cell_h};
    bool slot_focus = row_focused && col_focused;
    Color fill = slot_focus ? (Color){55, 75, 110, 255} : (Color){38, 46, 60, 255};
    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 1.5f, (Color){100, 120, 150, 255});
    int mid_y = row_top + SHELL_ROW_H / 2;
    if (is_active) {
        draw_checkmark(cx, mid_y + 2, GREEN);
    } else {
        Color hint = slot_focus ? RAYWHITE : (Color){160, 170, 190, 255};
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
    Rectangle r = {(float)(cx - half), (float)cell_y, (float)(half * 2), (float)cell_h};
    bool slot_focus = row_focused && col_focused;
    Color fill = slot_focus ? (Color){90, 55, 55, 255} : (Color){38, 46, 60, 255};
    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 1.5f, (Color){140, 100, 100, 255});
    Color t = slot_focus ? (Color){255, 200, 200, 255} : (Color){200, 140, 140, 255};
    const char *label = " X ";
    int ty = shell_text_baseline_y(row_top, SHELL_FONT_BODY);
    DrawText(label, cx - MeasureText(label, SHELL_FONT_BODY) / 2, ty, SHELL_FONT_BODY, t);
}

static void draw_overflow_row(int row_top, bool points_up)
{
    int row_w = VD_SCREEN_WIDTH - SHELL_PADDING - SHELL_PADDING;
    DrawRectangle(SHELL_PADDING, row_top, row_w, SHELL_OVERFLOW_ROW_H, (Color){24, 30, 40, 255});

    Color chevron = (Color){120, 140, 170, 255};
    int cy = row_top + SHELL_OVERFLOW_ROW_H / 2;
    for (int i = -1; i <= 1; i++) {
        int cx = VD_SCREEN_WIDTH / 2 + i * 20;
        if (points_up) {
            DrawLineEx((Vector2){(float)(cx - 6), (float)(cy + 3)}, (Vector2){(float)cx, (float)(cy - 4)}, 2.0f,
                       chevron);
            DrawLineEx((Vector2){(float)cx, (float)(cy - 4)}, (Vector2){(float)(cx + 6), (float)(cy + 3)}, 2.0f,
                       chevron);
        } else {
            DrawLineEx((Vector2){(float)(cx - 6), (float)(cy - 3)}, (Vector2){(float)cx, (float)(cy + 4)}, 2.0f,
                       chevron);
            DrawLineEx((Vector2){(float)cx, (float)(cy + 4)}, (Vector2){(float)(cx + 6), (float)(cy - 3)}, 2.0f,
                       chevron);
        }
    }
}

static void draw_message(const VdShell *shell)
{
    if (shell->message[0] == '\0') return;
    int msg_y = SHELL_FOOTER_Y - SHELL_SPACE_MD - SHELL_FONT_BODY;
    DrawText(shell->message, SHELL_PADDING, msg_y, SHELL_FONT_BODY, YELLOW);
}

static void draw_host_control_entry(int text_y)
{
    VdHostControlLink links[VD_HOST_CONTROL_MAX_LINKS];
    int count = host_control_links_list(links, VD_HOST_CONTROL_MAX_LINKS);
    int x = SHELL_NAME_X + SHELL_SPACE_MD;
    DrawText("Host Control", x, text_y, SHELL_FONT_BODY, RAYWHITE);
    int sub_y = text_y + SHELL_FONT_BODY + SHELL_SPACE_XS;
    if (count == 0) {
        DrawText("Not linked", x, sub_y, SHELL_FONT_CAPTION, (Color){130, 140, 160, 255});
        return;
    }
    char caption[VD_HOST_CONTROL_NAME_MAX + 32];
    if (count == 1) {
        snprintf(caption, sizeof(caption), "Linked: %s", links[0].name);
    } else {
        snprintf(caption, sizeof(caption), "Linked: %s +%d", links[0].name, count - 1);
    }
    DrawText(caption, x, sub_y, SHELL_FONT_CAPTION, GREEN);
}

static void draw_host_control_screen(VdShell *shell)
{
    DrawText("Host Control", SHELL_PADDING, SHELL_SECTION_Y, SHELL_FONT_BODY, GRAY);

    int y = SHELL_LIST_Y;
    if (lan_listener_is_running(&shell->listener)) {
        char line[160];
        snprintf(line, sizeof(line), "On the computer: vitadeck-host-companion link %s",
                 lan_listener_address(&shell->listener));
        DrawText(line, SHELL_PADDING, y, SHELL_FONT_BODY, GREEN);
    } else {
        DrawText("Not listening. Turn on Upload and link on the Home screen.", SHELL_PADDING, y, SHELL_FONT_BODY,
                 RAYWHITE);
    }
    y += SHELL_FONT_BODY + SHELL_SPACE_MD;

    DrawText("Linked computers", SHELL_PADDING, y, SHELL_FONT_BODY, GRAY);
    y += SHELL_FONT_BODY + SHELL_SPACE_SM;

    VdHostControlLink links[VD_HOST_CONTROL_MAX_LINKS];
    int count = host_control_links_list(links, VD_HOST_CONTROL_MAX_LINKS);
    if (count == 0) {
        DrawText("No computers linked yet.", SHELL_PADDING, y, SHELL_FONT_CAPTION, (Color){150, 160, 180, 255});
    }
    int row_w = VD_SCREEN_WIDTH - SHELL_PADDING - SHELL_PADDING;
    for (int i = 0; i < count; i++) {
        bool focused = i == shell->host_control_focus;
        DrawRectangle(SHELL_PADDING, y, row_w, SHELL_ROW_H,
                      focused ? (Color){42, 52, 72, 255} : (Color){28, 34, 44, 255});
        int text_y = shell_text_baseline_y(y, SHELL_FONT_BODY);
        int x = SHELL_NAME_X + SHELL_SPACE_MD;
        DrawText(links[i].name, x, text_y, SHELL_FONT_BODY, RAYWHITE);
        x += MeasureText(links[i].name, SHELL_FONT_BODY) + SHELL_SPACE_MD;
        DrawText(links[i].url, x, shell_text_baseline_y(y, SHELL_FONT_CAPTION), SHELL_FONT_CAPTION,
                 (Color){150, 160, 180, 255});
        y += SHELL_ROW_STEP;
    }

    draw_message(shell);
    shell_draw_footer_hint(count > 0 ? "Start / F1 · Confirm: forget · Back" : "Start / F1 · Back");
}

void shell_render(VdShell *shell)
{
    if (shell->state == VD_SHELL_HIDDEN) return;
    draw_panel();

    DrawText("VitaDeck", SHELL_PADDING, SHELL_TITLE_Y, SHELL_FONT_TITLE, RAYWHITE);
    if (shell->state == VD_SHELL_HOST_CONTROL) {
        draw_host_control_screen(shell);
        return;
    }
    DrawText("Deck Apps", SHELL_PADDING, SHELL_SECTION_Y, SHELL_FONT_BODY, GRAY);

    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    int row_count = package_count + SHELL_ENTRY_ROWS;

    int list_top = SHELL_LIST_Y;
    if (package_count == 0) {
        int hint_y = SHELL_SECTION_Y + SHELL_FONT_BODY + SHELL_SPACE_XS;
        DrawText("No Deck Apps installed yet.", SHELL_PADDING, hint_y, SHELL_FONT_CAPTION, (Color){150, 160, 180, 255});
        hint_y += SHELL_FONT_CAPTION + SHELL_SPACE_XS;
        DrawText("Open the URL under Upload and link on another device on this network", SHELL_PADDING, hint_y,
                 SHELL_FONT_CAPTION, (Color){130, 140, 160, 255});
        hint_y += SHELL_FONT_CAPTION + SHELL_SPACE_XS;
        DrawText("and upload a zip containing one .vdapp folder.", SHELL_PADDING, hint_y, SHELL_FONT_CAPTION,
                 (Color){130, 140, 160, 255});
        list_top = hint_y + SHELL_FONT_CAPTION + SHELL_SPACE_MD;
    }

    shell->scroll_row = clamp_scroll_row(shell->scroll_row, row_count);

    int row_top = list_top;
    if (shell->scroll_row > 0) {
        draw_overflow_row(row_top, true);
        row_top += SHELL_OVERFLOW_ROW_STEP;
    }

    bool restart_focused = false;
    int end_row = shell->scroll_row;
    for (int row = shell->scroll_row; row < row_count; row++) {
        bool row_focus = row == shell->focus_row && shell->remove_confirm_package[0] == '\0';
        bool listener_row = row_is_listener(row, package_count);
        bool listening = listener_row && lan_listener_is_running(&shell->listener);

        int row_w = VD_SCREEN_WIDTH - SHELL_PADDING - SHELL_PADDING;
        int band_h = row_band_height(row, package_count);
        int bottom_reserve = row + 1 < row_count ? SHELL_OVERFLOW_ROW_STEP : 0;
        if (row_top + band_h + bottom_reserve > SHELL_LIST_BOTTOM) break;
        Color band_color = row_focus ? (Color){42, 52, 72, 255} : (Color){28, 34, 44, 255};
        if (listening) band_color = row_focus ? (Color){34, 64, 58, 255} : (Color){24, 48, 44, 255};
        DrawRectangle(SHELL_PADDING, row_top, row_w, band_h, band_color);

        int text_y = shell_text_baseline_y(row_top, SHELL_FONT_BODY);
        if (row >= package_count) text_y = row_top + SHELL_SPACE_MD;

        if (row_is_host_control(row, package_count)) {
            draw_host_control_entry(text_y);
        } else if (listener_row) {
            int pad = SHELL_SPACE_MD;
            DrawText("Upload and link", SHELL_NAME_X + pad, text_y, SHELL_FONT_BODY, listening ? GREEN : RAYWHITE);
            int sub_y = text_y + SHELL_FONT_BODY + SHELL_SPACE_XS;
            if (listening) {
                DrawText(lan_listener_url(&shell->listener), SHELL_NAME_X + pad, sub_y, SHELL_FONT_CAPTION, GREEN);
            } else {
                DrawText("Off. Confirm to listen", SHELL_NAME_X + pad, sub_y, SHELL_FONT_CAPTION,
                         (Color){130, 140, 160, 255});
            }
        } else {
            const VdPackageInfo *pkg = &packages[row];
            char name_line[VD_DISPLAY_NAME_MAX + VD_VERSION_MAX + 12];
            snprintf(name_line, sizeof(name_line), "%s   %s", pkg->display_name, pkg->version);
            Color name_color = pkg->is_active ? GREEN : RAYWHITE;
            DrawText(name_line, SHELL_NAME_X + SHELL_SPACE_MD, text_y, SHELL_FONT_BODY, name_color);
            draw_tick_cell(SHELL_TICK_CX, row_top, pkg->is_active, row_focus, shell->focus_col == 0);
            if (row_focus && shell->focus_col == 0 && pkg->is_active) restart_focused = true;
            draw_x_cell(SHELL_X_CX, row_top, row_focus, shell->focus_col == 1);
        }
        row_top += band_h + SHELL_SPACE_SM;
        end_row = row + 1;
    }

    if (end_row < row_count) draw_overflow_row(row_top, false);

    if (shell->remove_confirm_package[0] != '\0') {
        int box_y = (int)(VD_SCREEN_HEIGHT * 0.40f);
        DrawRectangle(0, box_y, VD_SCREEN_WIDTH, 112, (Color){10, 12, 18, 255});
        int ly = box_y + SHELL_SPACE_MD;
        DrawText(shell->remove_confirm_label, SHELL_PADDING, ly, SHELL_FONT_BODY, RAYWHITE);
        DrawText("Confirm: remove · Back: cancel", SHELL_PADDING, ly + SHELL_FONT_BODY + SHELL_SPACE_SM,
                 SHELL_FONT_CAPTION, GRAY);
    }

    draw_message(shell);
    shell_draw_footer_hint(restart_focused ? "Start / F1 · Confirm: restart · Back" : "Start / F1 · Confirm · Back");
}
