#include <raylib.h>
#include <stdio.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "ui/fonts.h"
#include "ui/instance_tree.h"
#include "ui/input.h"
#include "ui/render.h"
#include "ui/scroll.h"
#include "core/event_queue.h"
#include "core/js_runtime.h"
#include "core/package_library.h"
#include "shell/shell.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

static bool load_active_package_fonts(char *error, size_t error_size)
{
    return font_registry_load_package(
        package_library_has_active_deck_app() ? package_library_active_package_path() : "", error, error_size);
}

#define return_defer(value)                                                                                            \
    do {                                                                                                               \
        result = (value);                                                                                              \
        goto defer;                                                                                                    \
    } while (0)

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int result = 0;
    bool window_ready = false;
    VdJsRuntime js_runtime;
    VdShell shell;
    char init_error[256];
    js_runtime_init(&js_runtime);
    shell_init(&shell);

    if (!event_queue_init()) {
        TraceLog(LOG_ERROR, "Could not initialize event queue.");
        return 1;
    }
    instance_tree_init();
    if (!package_library_init(init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "VitaDeck");
    window_ready = true;
    SetTargetFPS(60);

    if (!font_registry_init(init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }
    if (!load_active_package_fonts(init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        js_runtime.failed = true;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    if (package_library_has_active_deck_app()) DrawText("Loading...", 10, 10, 20, WHITE);
    EndDrawing();

    if (!js_runtime.failed && package_library_has_active_deck_app() && !js_runtime_start(&js_runtime)) {
        TraceLog(LOG_ERROR, "Could not create JS thread.");
        return_defer(1);
    }

    while (!WindowShouldClose()) {
        bool request_runtime_restart = false;
        shell_update(&shell, &request_runtime_restart);
        shell_poll_system_input(&shell, &request_runtime_restart);
        if (request_runtime_restart) {
            js_runtime_stop(&js_runtime);
            scroll_reset();
            if (package_library_has_active_deck_app()) {
                char font_error[256];
                if (!load_active_package_fonts(font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                    js_runtime.failed = true;
                } else if (!js_runtime_start(&js_runtime)) {
                    TraceLog(LOG_ERROR, "Could not create JS thread.");
                    js_runtime.failed = true;
                }
            } else {
                char font_error[256];
                if (!load_active_package_fonts(font_error, sizeof(font_error))) TraceLog(LOG_ERROR, "%s", font_error);
            }
        }

        if (!shell_is_visible(&shell) && package_library_has_active_deck_app() && js_runtime_is_ready(&js_runtime)) {
            poll_mouse_input();
            poll_touch_input();
            poll_gamepad_input();
        }

        BeginDrawing();
        ClearBackground(BLACK);
        if (!package_library_has_active_deck_app()) {
            /* Shell shows setup instructions; keep canvas black underneath. */
        } else if (js_runtime_failed(&js_runtime)) {
            DrawText("Deck App Runtime failed to start.", 10, 10, 24, RED);
            DrawText("Open the Shell with F1/Start to choose another Deck App.", 10, 44, 20, RAYWHITE);
        } else if (!js_runtime_is_ready(&js_runtime)) {
            DrawText("Loading JavaScript...", 10, 10, 20, WHITE);
        } else {
            render_draw_list();
        }
        shell_render(&shell);
        DrawFPS(SCREEN_WIDTH - 100, 10);

        for (int i = 0; i < GetTouchPointCount(); i++) {
            Vector2 position = GetTouchPosition(i);
            DrawCircle(position.x, position.y, 10, RED);
        }
        EndDrawing();
    }

defer:
    shell_shutdown(&shell);
    js_runtime_stop(&js_runtime);
    font_registry_shutdown();
    event_queue_shutdown();
    event_queue_destroy();
    if (window_ready) CloseWindow();
    return result;
}
