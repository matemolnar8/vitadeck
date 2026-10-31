#include "core/package_library.h"
#include "core/util.h"
#include "host_control/link.h"
#include "shell/shell.h"
#include "ui/input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures = 0;
static int ignore_held_calls = 0;

// The real one lives in src/ui/input.c, which drags in the instance tree; the Shell only needs the call.
void input_ignore_held_pad_buttons(void)
{
    ignore_held_calls++;
}

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "shell_input_harness: %s\n", message);
    failures++;
}

static void publish(const char *package_name, const char *display_name)
{
    char staged[VD_PATH_MAX];
    vd_join_path(staged, sizeof(staged), package_library_staging_root(), package_name);
    char manifest[256];
    snprintf(manifest, sizeof(manifest),
             "{\"schemaVersion\":1,\"name\":\"%s\",\"version\":\"1.0.0\",\"entry\":\"app.js\"}", display_name);
    char path[VD_PATH_MAX];
    vd_join_path(path, sizeof(path), staged, "manifest.json");
    if (!vd_mkdir_p(staged) || !vd_write_text_file(path, manifest)) expect(false, "could not write manifest");
    vd_join_path(path, sizeof(path), staged, "app.js");
    if (!vd_write_text_file(path, "globalThis.vitadeckPackage.register(() => null);\n")) {
        expect(false, "could not write app.js");
    }
    char error[256] = "";
    if (!package_library_publish_package(staged, package_name, NULL, NULL, error, sizeof(error))) expect(false, error);
}

// Applies one frame with a single input pressed and returns whether it asked for a Deck App Runtime Restart.
static bool press(VdShell *shell, VdShellInput input)
{
    bool restart = false;
    shell_apply_system_input(shell, &input, &restart);
    return restart;
}

static bool active_is(const char *package_name)
{
    char active[VD_PACKAGE_NAME_MAX];
    package_library_active_package_name(active, sizeof(active));
    return strcmp(active, package_name) == 0;
}

static bool is_installed(const char *package_name)
{
    VdPackageInfo items[VD_PACKAGE_LIST_MAX];
    int count = package_library_list(items, VD_PACKAGE_LIST_MAX);
    for (int i = 0; i < count; i++) {
        if (strcmp(items[i].package_name, package_name) == 0) return true;
    }
    return false;
}

static const VdShellInput START = {.start = true};
static const VdShellInput CONFIRM = {.confirm = true};
static const VdShellInput BACK = {.back = true};
static const VdShellInput DOWN = {.down = true};
static const VdShellInput RIGHT = {.right = true};

static void test_open_and_hide(VdShell *shell)
{
    expect(!press(shell, START) && shell_is_visible(shell), "Start should open the Shell");
    expect(shell->focus_row == 0 && shell->focus_col == 0, "opening the Shell should focus the first row");

    int ignored = ignore_held_calls;
    expect(!press(shell, START) && !shell_is_visible(shell), "Start should hide the Shell");
    expect(ignore_held_calls == ignored + 1, "hiding the Shell should ignore held Pad Buttons");

    press(shell, START);
    expect(!press(shell, BACK) && !shell_is_visible(shell), "Back should hide the Shell");
}

static void test_restart_active(VdShell *shell)
{
    press(shell, START);
    expect(press(shell, CONFIRM), "confirming the Active Deck App should restart it");
    expect(!shell_is_visible(shell), "restarting should hide the Shell");
    expect(active_is("a.vdapp"), "restarting should keep the Active Deck App");
}

static void test_activate_other(VdShell *shell)
{
    press(shell, START);
    press(shell, DOWN);
    expect(press(shell, CONFIRM), "confirming another package should restart into it");
    expect(!shell_is_visible(shell), "activating should hide the Shell");
    expect(active_is("b.vdapp"), "activating should move the Active Deck App");
}

static void test_remove_cancel_and_confirm(VdShell *shell)
{
    press(shell, START);
    press(shell, RIGHT);
    expect(!press(shell, CONFIRM), "confirming X should not restart");
    expect(strcmp(shell->remove_confirm_package, "a.vdapp") == 0, "confirming X should ask to remove the package");

    expect(!press(shell, BACK), "Back on the removal confirm should not restart");
    expect(shell_is_visible(shell), "Back on the removal confirm should keep the Shell open");
    expect(shell->remove_confirm_package[0] == '\0', "Back should cancel the removal confirm");
    expect(is_installed("a.vdapp"), "cancelling should keep the package");

    press(shell, CONFIRM);
    expect(shell->remove_confirm_package[0] != '\0', "confirming X again should ask to remove the package again");
    press(shell, CONFIRM);
    expect(shell->remove_confirm_package[0] == '\0', "confirming the removal should close the confirm");
    expect(!is_installed("a.vdapp"), "confirming the removal should remove the package");
    expect(strcmp(shell->message, "Removed a.vdapp.") == 0, "removing should say so");
    expect(shell->focus_row == 0 && shell->focus_col == 0, "removing should focus the remaining row's Run cell");

    press(shell, BACK);
}

static void test_listener_toggle(VdShell *shell)
{
    expect(lan_listener_is_running(&shell->listener), "the listener should start with VitaDeck");
    press(shell, START);
    press(shell, DOWN);
    press(shell, CONFIRM);
    expect(!lan_listener_is_running(&shell->listener), "confirming Upload and link should stop the listener");
    expect(strcmp(shell->message, "Stopped listening for uploads and links.") == 0, "stopping should say so");

    press(shell, CONFIRM);
    expect(lan_listener_is_running(&shell->listener), "confirming it again should start the listener");
    expect(strcmp(shell->message, "Listening for uploads and links.") == 0, "starting should say so");

    press(shell, BACK);
    expect(lan_listener_is_running(&shell->listener), "hiding the Shell should keep the listener running");
    press(shell, START);
    expect(shell->message[0] == '\0', "reopening the Shell should not show the message from before it was hidden");
    press(shell, BACK);
}

static int link_count(void)
{
    VdHostControlLink links[VD_HOST_CONTROL_MAX_LINKS];
    return host_control_links_list(links, VD_HOST_CONTROL_MAX_LINKS);
}

static void test_host_control(VdShell *shell)
{
    const VdHostControlLink studio = {"studio", "Studio Mac", "http://192.168.1.20:8797/"};
    const VdHostControlLink den = {"den", "Den PC", "http://192.168.1.21:8797/"};
    expect(host_control_links_save(&studio) && host_control_links_save(&den), "could not save links");

    // Rows: Beta, the Shell Listener Entry, then the Shell Host Control Entry.
    press(shell, START);
    press(shell, DOWN);
    press(shell, DOWN);
    press(shell, CONFIRM);
    expect(shell->state == VD_SHELL_HOST_CONTROL, "confirming Host Control should open its screen");

    expect(!press(shell, BACK) && shell_is_visible(shell), "Circle on Host Control should keep the Shell open");
    expect(shell->state == VD_SHELL_HOME, "Circle should return to Home");
    expect(shell->focus_row == 2, "Circle should focus the Shell Host Control Entry");

    press(shell, CONFIRM);
    press(shell, DOWN);
    expect(!press(shell, CONFIRM), "forgetting a link should not restart");
    VdHostControlLink left[VD_HOST_CONTROL_MAX_LINKS];
    expect(host_control_links_list(left, VD_HOST_CONTROL_MAX_LINKS) == 1 && strcmp(left[0].id, "den") == 0,
           "Cross should forget the focused computer at once");
    expect(strcmp(shell->message, "Forgot Studio Mac.") == 0, "forgetting should say so");
    expect(shell->host_control_focus == 0, "forgetting the last row should focus the one above");
    press(shell, CONFIRM);
    expect(link_count() == 0, "Cross should forget the remaining computer");
    press(shell, CONFIRM);
    expect(shell->state == VD_SHELL_HOST_CONTROL, "Cross with no computers should do nothing");

    press(shell, START);
    expect(!shell_is_visible(shell), "Start should hide the Shell from Host Control");
    expect(lan_listener_is_running(&shell->listener), "Host Control should leave the listener running");
}

// Runs before anything is installed; the Active Deck App cannot be removed, so only a fresh library is empty.
static void test_empty_library_focus(VdShell *shell)
{
    press(shell, START);
    expect(shell->focus_row == 1, "an empty Shell should focus Host Control, not Upload and link");
    press(shell, CONFIRM);
    expect(shell->state == VD_SHELL_HOST_CONTROL && lan_listener_is_running(&shell->listener),
           "the first Cross on an empty Shell should keep the listener running");
    press(shell, BACK);
    press(shell, BACK);
    expect(shell_is_visible(shell), "without an Active Deck App the Shell should stay open");
}

int main(void)
{
    char root[] = "/tmp/vitadeck-shell-input-XXXXXX";
    if (!mkdtemp(root) || setenv("VITADECK_DATA_ROOT", root, 1) != 0) {
        fprintf(stderr, "shell_input_harness: could not create temp data root\n");
        return 1;
    }
    char error[256] = "";
    if (!package_library_init(error, sizeof(error)) || !host_control_links_init(root)) {
        fprintf(stderr, "shell_input_harness: could not init the data root: %s\n", error);
        vd_remove_tree(root);
        return 1;
    }
    VdShell shell;
    shell_init(&shell);
    shell_start_listener(&shell);
    test_empty_library_focus(&shell);

    // Rows sort by display name: Alpha, Beta, then the Shell Listener Entry. Alpha is active.
    publish("a.vdapp", "Alpha");
    publish("b.vdapp", "Beta");
    expect(active_is("a.vdapp"), "the first publish should activate it");
    expect(!press(&shell, START) && !shell_is_visible(&shell), "Start should hide the Shell once a Deck App is active");

    // Each test leaves the Shell hidden and builds on the library the one before left: activating makes b active so
    // a can be removed, and the upload and Host Control tests run with b alone.
    test_open_and_hide(&shell);
    test_restart_active(&shell);
    test_activate_other(&shell);
    test_remove_cancel_and_confirm(&shell);
    test_listener_toggle(&shell);
    test_host_control(&shell);

    shell_shutdown(&shell);
    vd_remove_tree(root);

    if (failures > 0) {
        fprintf(stderr, "shell_input_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("shell_input_harness: ok\n");
    return 0;
}
