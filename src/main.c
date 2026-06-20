#include <raylib.h>
#include <stdio.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/deck_bootstrap.h"
#include "core/js_runtime.h"
#include "core/package_library.h"
#include "shell/shell.h"
#include "ui/input.h"
#include "ui/scroll.h"

#define return_defer(value)                                                                                            \
    do {                                                                                                               \
        result = (value);                                                                                              \
        goto defer;                                                                                                    \
    } while (0)

static void draw_shell_runtime_recovery_hint(const VdDeckBootstrap *bootstrap)
{
    if (!js_runtime_failed(&bootstrap->js_runtime)) return;
    DrawText("Open the Shell with F1/Start to choose another Deck App.", 10, 44, 20, RAYWHITE);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int result = 0;
    VdDeckBootstrap bootstrap;
    VdShell shell;
    char init_error[256];

    deck_bootstrap_init(&bootstrap);
    shell_init(&shell);

    if (!deck_bootstrap_boot_subsystems(init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }
    if (!package_library_has_active_deck_app()) shell_show_home(&shell);

    if (!deck_bootstrap_open_window(&bootstrap, "VitaDeck", NULL, init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }
    if (bootstrap.js_runtime.failed) TraceLog(LOG_ERROR, "%s", init_error);

    deck_bootstrap_draw_loading_splash();

    if (package_library_has_active_deck_app() &&
        !deck_bootstrap_start_active_deck_app(&bootstrap, init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }

    while (!WindowShouldClose()) {
        bool request_runtime_restart = false;
        shell_update(&shell, &request_runtime_restart);
        shell_poll_system_input(&shell, &request_runtime_restart);
        if (request_runtime_restart) {
            js_runtime_stop(&bootstrap.js_runtime);
            scroll_reset();
            if (package_library_has_active_deck_app()) {
                char font_error[256];
                if (!deck_bootstrap_reload_active_package_fonts(&bootstrap, font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                } else if (!deck_bootstrap_start_active_deck_app(&bootstrap, font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                    bootstrap.js_runtime.failed = true;
                }
            } else {
                char font_error[256];
                if (!deck_bootstrap_reload_active_package_fonts(&bootstrap, font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                }
            }
        }

        if (!shell_is_visible(&shell) && package_library_has_active_deck_app() &&
            js_runtime_is_ready(&bootstrap.js_runtime)) {
            poll_mouse_input();
            poll_touch_input();
            poll_gamepad_input();
        }

        BeginDrawing();
        ClearBackground(BLACK);
        deck_bootstrap_draw_deck_canvas(&bootstrap);
        draw_shell_runtime_recovery_hint(&bootstrap);
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
    deck_bootstrap_shutdown(&bootstrap);
    return result;
}
