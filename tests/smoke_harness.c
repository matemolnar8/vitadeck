#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/deck_bootstrap.h"
#include "core/js_runtime.h"
#include "core/package_library.h"
#include "platform/thread.h"
#include "ui/instance_tree.h"

#define WAIT_TIMEOUT_SEC 5.0
#define MAX_PIXEL_MISMATCH 64

static void fail(const char *message)
{
    fprintf(stderr, "smoke_harness: %s\n", message);
    exit(1);
}

static bool wait_for_smoke_ok(const VdDeckBootstrap *bootstrap)
{
    double deadline = GetTime() + WAIT_TIMEOUT_SEC;
    while (GetTime() < deadline) {
        if (js_runtime_failed(&bootstrap->js_runtime)) fail("JS runtime failed");
        if (js_runtime_is_ready(&bootstrap->js_runtime) && instance_tree_contains_text("SMOKE_OK")) return true;
        if (js_runtime_is_ready(&bootstrap->js_runtime) && instance_tree_contains_text("SMOKE_INTERVAL_FAIL")) {
            fail("timer interval did not tick before timeout");
        }
        vd_thread_yield();
    }
    return false;
}

static bool images_match(const char *actual_path, const char *golden_path, int max_mismatches)
{
    Image actual = LoadImage(actual_path);
    Image golden = LoadImage(golden_path);
    if (!actual.data || !golden.data) {
        if (actual.data) UnloadImage(actual);
        if (golden.data) UnloadImage(golden);
        fprintf(stderr, "smoke_harness: could not load images for comparison\n");
        return false;
    }

    if (actual.width != golden.width || actual.height != golden.height) {
        fprintf(stderr, "smoke_harness: image size mismatch (%dx%d vs %dx%d)\n", actual.width, actual.height,
                golden.width, golden.height);
        UnloadImage(actual);
        UnloadImage(golden);
        return false;
    }

    int mismatches = 0;
    int pixel_count = actual.width * actual.height;
    Color *a = actual.data;
    Color *b = golden.data;
    for (int i = 0; i < pixel_count; i++) {
        if (a[i].r != b[i].r || a[i].g != b[i].g || a[i].b != b[i].b || a[i].a != b[i].a) {
            mismatches++;
            if (mismatches > max_mismatches) break;
        }
    }

    UnloadImage(actual);
    UnloadImage(golden);

    if (mismatches > max_mismatches) {
        fprintf(stderr, "smoke_harness: %d pixels differ (allowed %d)\n", mismatches, max_mismatches);
        return false;
    }

    return true;
}

static bool verify_instance_tree(void)
{
    int text_nodes = instance_tree_count_nodes(NT_TEXT);
    int rect_nodes = instance_tree_count_nodes(NT_RECT);
    int scroll_nodes = instance_tree_count_nodes(NT_SCROLL);
    int button_nodes = instance_tree_count_nodes(NT_BUTTON);
    if (text_nodes < 8) {
        fprintf(stderr, "smoke_harness: expected at least 8 text nodes, got %d\n", text_nodes);
        return false;
    }
    if (rect_nodes < 6) {
        fprintf(stderr, "smoke_harness: expected at least 6 rect nodes, got %d\n", rect_nodes);
        return false;
    }
    if (scroll_nodes < 1) {
        fprintf(stderr, "smoke_harness: expected at least 1 scroll node, got %d\n", scroll_nodes);
        return false;
    }
    if (button_nodes < 2) {
        fprintf(stderr, "smoke_harness: expected at least 2 button nodes, got %d\n", button_nodes);
        return false;
    }
    if (!instance_tree_contains_text("SMOKE_MONO")) {
        fprintf(stderr, "smoke_harness: custom font text not found in instance tree\n");
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    const char *golden_path = NULL;
    const char *output_path = "smoke_screenshot.png";
    bool update_golden = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--update-golden") == 0) {
            update_golden = true;
        } else if (strcmp(argv[i], "--golden") == 0 && i + 1 < argc) {
            golden_path = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else {
            fprintf(stderr, "usage: %s [--golden PATH] [--output PATH] [--update-golden]\n", argv[0]);
            return 2;
        }
    }

    SetTraceLogLevel(LOG_WARNING);
    deck_bootstrap_configure_smoke_window_flags();

    VdDeckBootstrap bootstrap;
    deck_bootstrap_init(&bootstrap);

    char init_error[256];
    if (!deck_bootstrap_boot_subsystems(init_error, sizeof(init_error))) {
        fprintf(stderr, "smoke_harness: %s\n", init_error);
        return 1;
    }
    if (!package_library_has_active_deck_app()) fail("no active deck app configured");

    if (!deck_bootstrap_open_window(&bootstrap, "VitaDeck Smoke", NULL, init_error, sizeof(init_error))) {
        fprintf(stderr, "smoke_harness: %s\n", init_error);
        return 1;
    }

    if (!deck_bootstrap_start_active_deck_app(&bootstrap, init_error, sizeof(init_error))) {
        fprintf(stderr, "smoke_harness: %s\n", init_error);
        return 1;
    }
    if (!wait_for_smoke_ok(&bootstrap)) fail("timed out waiting for SMOKE_OK");
    if (!verify_instance_tree()) return 1;

    BeginDrawing();
    ClearBackground(BLACK);
    deck_bootstrap_draw_deck_canvas(&bootstrap);
    EndDrawing();

    TakeScreenshot(output_path);
    deck_bootstrap_shutdown(&bootstrap);

    if (update_golden) {
        if (!golden_path) fail("--update-golden requires --golden PATH");
        if (rename(output_path, golden_path) != 0) {
            fprintf(stderr, "smoke_harness: could not update golden image at %s\n", golden_path);
            return 1;
        }
        printf("smoke_harness: updated golden image at %s\n", golden_path);
        return 0;
    }

    if (golden_path && access(golden_path, R_OK) != 0) {
        fprintf(stderr, "smoke_harness: no golden image at %s; re-run with --update-golden\n", golden_path);
        return 1;
    }

    if (golden_path && !images_match(output_path, golden_path, MAX_PIXEL_MISMATCH)) {
        return 1;
    }

    printf("smoke_harness: ok (screenshot: %s)\n", output_path);
    return 0;
}
