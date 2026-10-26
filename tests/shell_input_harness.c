#include "core/package_library.h"
#include "core/util.h"
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

static void test_upload_cancel(VdShell *shell)
{
    press(shell, START);
    press(shell, DOWN);
    press(shell, CONFIRM);
    expect(upload_server_is_running(&shell->upload_server), "confirming Upload should start the listener");
    expect(strcmp(shell->message, "Upload server started.") == 0, "starting the listener should say so");

    press(shell, BACK);
    expect(upload_server_is_running(&shell->upload_server), "hiding the Shell should keep the listener running");
    press(shell, START);
    expect(shell->message[0] == '\0', "reopening the Shell should not show the message from before it was hidden");

    press(shell, DOWN);
    press(shell, CONFIRM);
    expect(!upload_server_is_running(&shell->upload_server), "confirming Upload again should stop the listener");
    expect(strcmp(shell->message, "Upload server stopped.") == 0, "stopping the listener should say so");
    press(shell, BACK);
}

int main(void)
{
    char root[] = "/tmp/vitadeck-shell-input-XXXXXX";
    if (!mkdtemp(root) || setenv("VITADECK_DATA_ROOT", root, 1) != 0) {
        fprintf(stderr, "shell_input_harness: could not create temp data root\n");
        return 1;
    }
    char error[256] = "";
    if (!package_library_init(error, sizeof(error))) {
        fprintf(stderr, "shell_input_harness: %s\n", error);
        return 1;
    }
    // Rows sort by display name: Alpha, Beta, then the Shell Upload Entry. Alpha is active.
    publish("a.vdapp", "Alpha");
    publish("b.vdapp", "Beta");

    VdShell shell;
    shell_init(&shell);

    test_open_and_hide(&shell);
    test_restart_active(&shell);
    test_activate_other(&shell);
    test_remove_cancel_and_confirm(&shell);
    test_upload_cancel(&shell);

    shell_shutdown(&shell);
    vd_remove_tree(root);

    if (failures > 0) {
        fprintf(stderr, "shell_input_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("shell_input_harness: ok\n");
    return 0;
}
