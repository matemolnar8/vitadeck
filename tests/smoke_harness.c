#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/event_queue.h"
#include "core/js_runtime.h"
#include "core/package_library.h"
#include "ui/fonts.h"
#include "ui/instance_tree.h"
#include "ui/render.h"
#include "platform/thread.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define WAIT_TIMEOUT_SEC 5.0
#define MAX_PIXEL_MISMATCH 64

static void configure_window_flags(void)
{
#if defined(__APPLE__)
    // Hidden windows fail NSGL pixel-format init on GitHub Actions macOS runners.
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
#else
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
#endif
}

static void fail(const char *message)
{
    fprintf(stderr, "smoke_harness: %s\n", message);
    exit(1);
}

static void collect_text(const ReactInstance *inst, char *buffer, size_t buffer_size)
{
    if (!inst || buffer_size == 0) return;

    if (inst->type == NT_RAW_TEXT && inst->props.raw_text) {
        size_t used = strlen(buffer);
        if (used < buffer_size - 1) {
            strncat(buffer, inst->props.raw_text, buffer_size - used - 1);
        }
    }

    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        collect_text(inst->children[i], buffer, buffer_size);
    }
}

static void collect_tree_text(char *buffer, size_t buffer_size)
{
    instance_tree_render_lock();
    ReactInstance **roots = instance_get_root_children();
    int count = arrlen(roots);
    for (int i = 0; i < count; i++) {
        collect_text(roots[i], buffer, buffer_size);
    }
    instance_tree_render_unlock();
}

static bool tree_contains(const char *needle)
{
    char text[4096] = {0};
    collect_tree_text(text, sizeof(text));
    return strstr(text, needle) != NULL;
}

static int count_node_type(const ReactInstance *inst, NodeType type)
{
    if (!inst) return 0;

    int count = inst->type == type ? 1 : 0;
    int child_count = arrlen(inst->children);
    for (int i = 0; i < child_count; i++) {
        count += count_node_type(inst->children[i], type);
    }
    return count;
}

static int count_front_nodes(NodeType type)
{
    int count = 0;
    instance_tree_render_lock();
    ReactInstance **roots = instance_get_root_children();
    int root_count = arrlen(roots);
    for (int i = 0; i < root_count; i++) {
        count += count_node_type(roots[i], type);
    }
    instance_tree_render_unlock();
    return count;
}

static bool wait_for_smoke_ok(const VdJsRuntime *runtime)
{
    double deadline = GetTime() + WAIT_TIMEOUT_SEC;
    while (GetTime() < deadline) {
        if (js_runtime_failed(runtime)) fail("JS runtime failed");
        if (js_runtime_is_ready(runtime) && tree_contains("SMOKE_OK")) return true;
        if (js_runtime_is_ready(runtime) && tree_contains("SMOKE_INTERVAL_FAIL")) {
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

static bool load_active_package_fonts(char *error, size_t error_size)
{
    return font_registry_load_package(
        package_library_has_active_deck_app() ? package_library_active_package_path() : "", error, error_size);
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
    configure_window_flags();

    VdJsRuntime runtime;
    js_runtime_init(&runtime);

    if (!event_queue_init()) fail("could not initialize event queue");
    instance_tree_init();

    char init_error[256];
    if (!package_library_init(init_error, sizeof(init_error))) {
        fprintf(stderr, "smoke_harness: %s\n", init_error);
        return 1;
    }
    if (!package_library_has_active_deck_app()) fail("no active deck app configured");

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "VitaDeck Smoke");
    if (!IsWindowReady()) fail("could not initialize raylib window");
    SetTargetFPS(60);

    if (!font_registry_init(init_error, sizeof(init_error))) {
        fprintf(stderr, "smoke_harness: %s\n", init_error);
        return 1;
    }
    if (!load_active_package_fonts(init_error, sizeof(init_error))) {
        fprintf(stderr, "smoke_harness: %s\n", init_error);
        return 1;
    }

    if (!js_runtime_start(&runtime)) fail("could not start JS runtime");
    if (!wait_for_smoke_ok(&runtime)) fail("timed out waiting for SMOKE_OK");

    int text_nodes = count_front_nodes(NT_TEXT);
    int rect_nodes = count_front_nodes(NT_RECT);
    int scroll_nodes = count_front_nodes(NT_SCROLL);
    int button_nodes = count_front_nodes(NT_BUTTON);
    if (text_nodes < 8) {
        fprintf(stderr, "smoke_harness: expected at least 8 text nodes, got %d\n", text_nodes);
        return 1;
    }
    if (rect_nodes < 6) {
        fprintf(stderr, "smoke_harness: expected at least 6 rect nodes, got %d\n", rect_nodes);
        return 1;
    }
    if (scroll_nodes < 1) {
        fprintf(stderr, "smoke_harness: expected at least 1 scroll node, got %d\n", scroll_nodes);
        return 1;
    }
    if (button_nodes < 2) {
        fprintf(stderr, "smoke_harness: expected at least 2 button nodes, got %d\n", button_nodes);
        return 1;
    }
    if (!tree_contains("SMOKE_MONO")) {
        fprintf(stderr, "smoke_harness: custom font text not found in instance tree\n");
        return 1;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    render_draw_list();
    EndDrawing();

    TakeScreenshot(output_path);

    js_runtime_stop(&runtime);
    font_registry_shutdown();
    event_queue_shutdown();
    event_queue_destroy();
    CloseWindow();

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
        if (!update_golden) {
            fprintf(stderr, "smoke_harness: no golden image at %s; re-run with --update-golden\n", golden_path);
            return 1;
        }
    }

    if (golden_path && !update_golden && !images_match(output_path, golden_path, MAX_PIXEL_MISMATCH)) {
        return 1;
    }

    printf("smoke_harness: ok (screenshot: %s)\n", output_path);
    return 0;
}
