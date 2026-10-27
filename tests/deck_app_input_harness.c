#include <raylib.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/bootstrap.h"
#include "core/event_queue.h"
#include "core/js_runtime.h"
#include "core/package_library.h"
#include "ui/input.h"
#include "ui/scroll.h"

#define WAIT_TIMEOUT_SEC 5.0
#define WATCHDOG_SEC 10
#define SETTLE_FRAMES 3
// Each round ends with L, R, R, one demo section on, so 12 rounds press every Pad Button on every demo section.
#define ROUNDS 12

static const PadButton ROUND[] = {PAD_CIRCLE, PAD_TRIANGLE, PAD_SQUARE, PAD_L, PAD_R, PAD_R};

static const char *const PAD_BUTTON_NAMES[PAD_BUTTON_COUNT] = {
    [PAD_CIRCLE] = "Circle", [PAD_TRIANGLE] = "Triangle", [PAD_SQUARE] = "Square", [PAD_L] = "L", [PAD_R] = "R",
};

static char watchdog_message[512];

// A stuck JS thread never returns from js_runtime_stop's join, so a blocked main thread is reported from here.
static void on_watchdog(int signal_number)
{
    (void)signal_number;
    ssize_t written = write(STDERR_FILENO, watchdog_message, strlen(watchdog_message));
    (void)written;
    _exit(1);
}

static void watch(const char *package, const char *step)
{
    snprintf(watchdog_message, sizeof(watchdog_message), "deck_app_input_harness: hung in %s %s\n", package, step);
    alarm(WATCHDOG_SEC);
}

static void fail(const char *package, const char *message)
{
    fprintf(stderr, "deck_app_input_harness: %s: %s\n", package, message);
    exit(1);
}

static void draw_frame(const VdBootstrap *bootstrap, GamepadSample sample)
{
    sample.dt = 1.0f / 60.0f;
    input_apply_gamepad(&sample);
    BeginDrawing();
    ClearBackground(BLACK);
    bootstrap_draw_deck_canvas(bootstrap);
    EndDrawing();
}

static void check_not_failed(const VdBootstrap *bootstrap, const char *package)
{
    if (!js_runtime_failed(&bootstrap->js_runtime)) return;
    char message[VD_FAILURE_MESSAGE_MAX + 32];
    snprintf(message, sizeof(message), "Deck App failed: %s", bootstrap->js_runtime.failure_message);
    fail(package, message);
}

static void wait_for_ready(const VdBootstrap *bootstrap, const char *package)
{
    double deadline = GetTime() + WAIT_TIMEOUT_SEC;
    while (!js_runtime_is_ready(&bootstrap->js_runtime)) {
        check_not_failed(bootstrap, package);
        if (GetTime() >= deadline) fail(package, "timed out waiting for the Deck App to start");
        draw_frame(bootstrap, (GamepadSample){0});
    }
}

// Presses and releases button, then waits for the JS thread to take both events. A handler that never returns
// leaves the release queued.
static void press(const VdBootstrap *bootstrap, const char *package, PadButton button)
{
    char step[64];
    snprintf(step, sizeof(step), "after %s", PAD_BUTTON_NAMES[button]);
    watch(package, step);

    GamepadSample held = {0};
    held.pad_buttons[button] = true;
    draw_frame(bootstrap, held);
    draw_frame(bootstrap, (GamepadSample){0});

    double deadline = GetTime() + WAIT_TIMEOUT_SEC;
    while (!event_queue_is_empty()) {
        check_not_failed(bootstrap, package);
        if (GetTime() >= deadline) {
            char message[96];
            snprintf(message, sizeof(message), "JS thread hung after %s", PAD_BUTTON_NAMES[button]);
            fail(package, message);
        }
        draw_frame(bootstrap, (GamepadSample){0});
    }
    for (int i = 0; i < SETTLE_FRAMES; i++)
        draw_frame(bootstrap, (GamepadSample){0});
    check_not_failed(bootstrap, package);
}

static VdBootstrapWindowConfig window_config(void)
{
#if defined(__APPLE__)
    return (VdBootstrapWindowConfig){.raylib_config_flags = FLAG_WINDOW_UNDECORATED};
#else
    return (VdBootstrapWindowConfig){.raylib_config_flags = FLAG_WINDOW_HIDDEN};
#endif
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    signal(SIGALRM, on_watchdog);

    VdBootstrap bootstrap;
    VdBootstrapWindowConfig config = window_config();
    bootstrap_init(&bootstrap);

    char error[256];
    if (!bootstrap_boot_subsystems(error, sizeof(error))) fail("boot", error);
    VdPackageInfo packages[VD_PACKAGE_LIST_MAX];
    int package_count = package_library_list(packages, VD_PACKAGE_LIST_MAX);
    if (package_count == 0) fail("boot", "no installed Deck Apps");
    if (!bootstrap_open_window(&bootstrap, "VitaDeck Deck App Input", &config, error, sizeof(error))) {
        fail("boot", error);
    }

    for (int i = 0; i < package_count; i++) {
        const char *package = packages[i].package_name;
        watch(package, "starting");
        if (!package_library_set_active_transient(package, error, sizeof(error))) fail(package, error);
        if (!bootstrap_reload_active_package_fonts(&bootstrap, error, sizeof(error))) fail(package, error);
        if (!bootstrap_start_active_deck_app(&bootstrap, error, sizeof(error))) fail(package, error);
        wait_for_ready(&bootstrap, package);

        for (int round = 0; round < ROUNDS; round++) {
            for (size_t b = 0; b < sizeof(ROUND) / sizeof(ROUND[0]); b++)
                press(&bootstrap, package, ROUND[b]);
        }

        watch(package, "stopping");
        js_runtime_stop(&bootstrap.js_runtime);
        scroll_reset();
        printf("deck_app_input_harness: %s ok\n", package);
    }

    watch("shutdown", "after the last Deck App");
    bootstrap_shutdown(&bootstrap);
    alarm(0);
    printf("deck_app_input_harness: ok (%d Deck Apps)\n", package_count);
    return 0;
}
