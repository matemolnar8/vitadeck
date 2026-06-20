#include <raylib.h>
#include <stdio.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/deck_host.h"
#include "core/event_queue.h"
#include "core/js_runtime.h"
#include "core/package_library.h"
#include "shell/shell.h"
#include "ui/fonts.h"
#include "ui/input.h"
#include "ui/instance_tree.h"
#include "ui/render.h"
#include "ui/scroll.h"

#define return_defer(value)                                                                                            \
    do {                                                                                                               \
        result = (value);                                                                                              \
        goto defer;                                                                                                    \
    } while (0)

static bool reload_active_package_fonts(VdDeckHost *host, char *error, size_t error_size)
{
    if (!font_registry_load_package(
            package_library_has_active_deck_app() ? package_library_active_package_path() : "", error, error_size)) {
        host->js_runtime.failed = true;
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int result = 0;
    VdDeckHost host;
    VdShell shell;
    char init_error[256];

    deck_host_init(&host);
    shell_init(&shell);

    if (!deck_host_boot_subsystems(init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return 1;
    }
    if (!package_library_has_active_deck_app()) shell_show_home(&shell);

    if (!deck_host_open_window(&host, "VitaDeck", NULL, init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }
    if (host.js_runtime.failed) TraceLog(LOG_ERROR, "%s", init_error);

    deck_host_draw_loading_splash();

    if (package_library_has_active_deck_app() && !deck_host_start_active_deck_app(&host, init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }

    while (!WindowShouldClose()) {
        bool request_runtime_restart = false;
        shell_update(&shell, &request_runtime_restart);
        shell_poll_system_input(&shell, &request_runtime_restart);
        if (request_runtime_restart) {
            js_runtime_stop(&host.js_runtime);
            scroll_reset();
            if (package_library_has_active_deck_app()) {
                char font_error[256];
                if (!reload_active_package_fonts(&host, font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                } else if (!deck_host_start_active_deck_app(&host, font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                    host.js_runtime.failed = true;
                }
            } else {
                char font_error[256];
                if (!reload_active_package_fonts(&host, font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                }
            }
        }

        if (!shell_is_visible(&shell) && package_library_has_active_deck_app() && js_runtime_is_ready(&host.js_runtime)) {
            poll_mouse_input();
            poll_touch_input();
            poll_gamepad_input();
        }

        BeginDrawing();
        ClearBackground(BLACK);
        deck_host_draw_deck_canvas(&host);
        shell_render(&shell);
        DrawFPS(VD_SCREEN_WIDTH - 100, 10);

        for (int i = 0; i < GetTouchPointCount(); i++) {
            Vector2 position = GetTouchPosition(i);
            DrawCircle(position.x, position.y, 10, RED);
        }
        EndDrawing();
    }

defer:
    shell_shutdown(&shell);
    deck_host_shutdown(&host);
    return result;
}
