#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/bootstrap.h"
#include "core/js_runtime.h"
#include "core/package_library.h"
#include "ui/input.h"
#include "ui/instance_tree.h"

#define WAIT_TIMEOUT_SEC 5.0
#define SETTLE_SEC 0.5
#define MAX_PIXEL_MISMATCH 128

// The demo Deck App's sections in order, each matched by the "N/12 · name" its header shows.
typedef struct {
    const char *slug;
    const char *header;
} Page;

static const Page PAGES[] = {
    {"01-rect-theme", "1/12 · Rect & theme"},
    {"02-text", "2/12 · Text"},
    {"03-image", "3/12 · Image"},
    {"04-button", "4/12 · Button"},
    {"05-pad-buttons", "5/12 · Pad Buttons"},
    {"06-stack", "6/12 · Stack"},
    {"07-scroll", "7/12 · Scroll"},
    {"08-text-input", "8/12 · Text Input"},
    {"09-storage", "9/12 · Storage"},
    {"10-timers", "10/12 · Timers"},
    {"11-fetch", "11/12 · fetch"},
    {"12-failure-section", "12/12 · Failure Screen"},
};

#define FAILURE_SCREEN_SLUG "13-failure-screen"

static void fail(const char *message)
{
    fprintf(stderr, "smoke_harness: %s\n", message);
    exit(1);
}

static void draw_frame(const VdBootstrap *bootstrap, GamepadSample sample)
{
    if (!js_runtime_failed(&bootstrap->js_runtime)) {
        sample.dt = 1.0f / 60.0f;
        input_apply_gamepad(&sample);
    }
    BeginDrawing();
    ClearBackground(BLACK);
    bootstrap_draw_deck_canvas(bootstrap);
    EndDrawing();
}

static void settle(const VdBootstrap *bootstrap)
{
    double deadline = GetTime() + SETTLE_SEC;
    while (GetTime() < deadline)
        draw_frame(bootstrap, (GamepadSample){0});
}

static bool wait_for_text(const VdBootstrap *bootstrap, const char *text)
{
    double deadline = GetTime() + WAIT_TIMEOUT_SEC;
    while (GetTime() < deadline) {
        if (js_runtime_failed(&bootstrap->js_runtime)) fail(bootstrap->js_runtime.failure_message);
        if (js_runtime_is_ready(&bootstrap->js_runtime) && instance_tree_contains_text(text)) return true;
        draw_frame(bootstrap, (GamepadSample){0});
    }
    return false;
}

static bool wait_for_failure(const VdBootstrap *bootstrap)
{
    double deadline = GetTime() + WAIT_TIMEOUT_SEC;
    while (GetTime() < deadline) {
        if (js_runtime_failed(&bootstrap->js_runtime)) return true;
        draw_frame(bootstrap, (GamepadSample){0});
    }
    return false;
}

static void press_pad(const VdBootstrap *bootstrap, PadButton button)
{
    GamepadSample held = {0};
    held.pad_buttons[button] = true;
    draw_frame(bootstrap, held);
    draw_frame(bootstrap, (GamepadSample){0});
}

static bool images_match(Image actual, const char *golden_path)
{
    Image golden = LoadImage(golden_path);
    if (!golden.data) {
        fprintf(stderr, "smoke_harness: could not load %s\n", golden_path);
        return false;
    }
    if (actual.width != golden.width || actual.height != golden.height) {
        fprintf(stderr, "smoke_harness: %s size mismatch (%dx%d vs %dx%d)\n", golden_path, actual.width, actual.height,
                golden.width, golden.height);
        UnloadImage(golden);
        return false;
    }

    int mismatches = 0;
    int pixel_count = actual.width * actual.height;
    Color *a = actual.data;
    Color *b = golden.data;
    for (int i = 0; i < pixel_count; i++) {
        if (a[i].r != b[i].r || a[i].g != b[i].g || a[i].b != b[i].b || a[i].a != b[i].a) mismatches++;
    }
    UnloadImage(golden);

    if (mismatches > MAX_PIXEL_MISMATCH) {
        fprintf(stderr, "smoke_harness: %s: %d pixels differ (allowed %d)\n", golden_path, mismatches,
                MAX_PIXEL_MISMATCH);
        return false;
    }
    return true;
}

// Saves the last drawn frame to OUTPUT_DIR/SLUG.png and compares it with GOLDEN_DIR/SLUG.png, or writes the
// golden instead when updating. Returns false on a mismatch or a missing golden.
static bool check_screenshot(const char *slug, const char *output_dir, const char *golden_dir, bool update_golden)
{
    char output_path[512];
    char golden_path[512];
    snprintf(output_path, sizeof(output_path), "%s/%s.png", output_dir, slug);
    snprintf(golden_path, sizeof(golden_path), "%s/%s.png", golden_dir, slug);

    Image actual = LoadImageFromScreen();
    bool ok = ExportImage(actual, update_golden ? golden_path : output_path);
    if (!ok) {
        fprintf(stderr, "smoke_harness: could not write %s\n", update_golden ? golden_path : output_path);
    } else if (update_golden) {
        printf("smoke_harness: updated %s\n", golden_path);
    } else if (access(golden_path, R_OK) != 0) {
        fprintf(stderr, "smoke_harness: no golden image at %s; re-run with --update-golden\n", golden_path);
        ok = false;
    } else {
        ok = images_match(actual, golden_path);
    }
    UnloadImage(actual);
    return ok;
}

static VdBootstrapWindowConfig smoke_window_config(void)
{
#if defined(__APPLE__)
    return (VdBootstrapWindowConfig){.raylib_config_flags = FLAG_WINDOW_UNDECORATED};
#else
    return (VdBootstrapWindowConfig){.raylib_config_flags = FLAG_WINDOW_HIDDEN};
#endif
}

int main(int argc, char *argv[])
{
    const char *golden_dir = NULL;
    const char *output_dir = NULL;
    bool update_golden = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--update-golden") == 0) {
            update_golden = true;
        } else if (strcmp(argv[i], "--golden-dir") == 0 && i + 1 < argc) {
            golden_dir = argv[++i];
        } else if (strcmp(argv[i], "--output-dir") == 0 && i + 1 < argc) {
            output_dir = argv[++i];
        } else {
            fprintf(stderr, "usage: %s --golden-dir DIR --output-dir DIR [--update-golden]\n", argv[0]);
            return 2;
        }
    }
    if (!golden_dir || !output_dir) fail("--golden-dir and --output-dir are required");

    SetTraceLogLevel(LOG_WARNING);

    VdBootstrap bootstrap;
    VdBootstrapWindowConfig window_config = smoke_window_config();
    bootstrap_init(&bootstrap);

    char init_error[256];
    if (!bootstrap_boot_subsystems(init_error, sizeof(init_error))) fail(init_error);
    if (!package_library_has_active_deck_app()) fail("no active deck app configured");
    if (!bootstrap_open_window(&bootstrap, "VitaDeck Smoke", &window_config, init_error, sizeof(init_error))) {
        fail(init_error);
    }
    if (!bootstrap_start_active_deck_app(&bootstrap, init_error, sizeof(init_error))) fail(init_error);

    int failures = 0;
    const int page_count = (int)(sizeof(PAGES) / sizeof(PAGES[0]));
    for (int i = 0; i < page_count; i++) {
        if (i > 0) press_pad(&bootstrap, PAD_R);
        if (!wait_for_text(&bootstrap, PAGES[i].header)) {
            fprintf(stderr, "smoke_harness: timed out waiting for \"%s\"\n", PAGES[i].header);
            return 1;
        }
        settle(&bootstrap);
        if (!check_screenshot(PAGES[i].slug, output_dir, golden_dir, update_golden)) failures++;
    }

    // Nothing has Focus here, so the first right focuses the first nav Button and the second reaches "Throw an error".
    draw_frame(&bootstrap, (GamepadSample){.right = true});
    draw_frame(&bootstrap, (GamepadSample){.right = true});
    draw_frame(&bootstrap, (GamepadSample){.confirm = true});
    draw_frame(&bootstrap, (GamepadSample){0});
    if (!wait_for_failure(&bootstrap)) fail("timed out waiting for the Deck App Failure Screen");
    settle(&bootstrap);
    if (!check_screenshot(FAILURE_SCREEN_SLUG, output_dir, golden_dir, update_golden)) failures++;

    bootstrap_shutdown(&bootstrap);

    if (failures > 0) {
        fprintf(stderr, "smoke_harness: %d screenshot(s) differ; see %s\n", failures, output_dir);
        return 1;
    }
    printf("smoke_harness: ok (screenshots: %s)\n", output_dir);
    return 0;
}
