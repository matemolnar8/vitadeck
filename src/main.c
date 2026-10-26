#include <raylib.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/bench.h"
#include "core/bootstrap.h"
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

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int result = 0;
    VdBootstrap bootstrap;
    VdShell shell;
    char init_error[256];

    bootstrap_init(&bootstrap);
    shell_init(&shell);

    if (!bootstrap_boot_subsystems(init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }
    bench_init();
    if (!package_library_has_active_deck_app() && !bench_active()) shell_show_home(&shell);

    if (!bootstrap_open_window(&bootstrap, "VitaDeck", NULL, init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }
    if (js_runtime_failed(&bootstrap.js_runtime)) TraceLog(LOG_ERROR, "%s", init_error);

    bootstrap_draw_loading_splash();

    if (package_library_has_active_deck_app() &&
        !bootstrap_start_active_deck_app(&bootstrap, init_error, sizeof(init_error))) {
        TraceLog(LOG_ERROR, "%s", init_error);
        return_defer(1);
    }

    while (!WindowShouldClose()) {
        bool request_runtime_restart = false;
        if (!bench_update(&bootstrap.js_runtime, &request_runtime_restart)) break;
        shell_update(&shell, &request_runtime_restart);
        shell_poll_system_input(&shell, &request_runtime_restart);
        if (request_runtime_restart) {
            js_runtime_stop(&bootstrap.js_runtime);
            scroll_reset();
            if (package_library_has_active_deck_app()) {
                char font_error[256];
                if (!bootstrap_reload_active_package_fonts(&bootstrap, font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                } else if (!bootstrap_start_active_deck_app(&bootstrap, font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                    js_runtime_fail(&bootstrap.js_runtime, font_error, NULL);
                }
            } else {
                char font_error[256];
                if (!bootstrap_reload_active_package_fonts(&bootstrap, font_error, sizeof(font_error))) {
                    TraceLog(LOG_ERROR, "%s", font_error);
                }
            }
        }

        const bool deck_app_owns_input = !shell_is_visible(&shell) && package_library_has_active_deck_app() &&
                                         js_runtime_is_ready(&bootstrap.js_runtime) &&
                                         !js_runtime_failed(&bootstrap.js_runtime);
        if (!deck_app_owns_input) input_cancel_text_input();
        poll_text_input();
        if (deck_app_owns_input && !input_is_editing_text()) {
            if (bench_active()) {
                bench_apply_input();
            } else {
                poll_mouse_input();
                poll_touch_input();
                poll_gamepad_input();
            }
        }
        bench_frame_mark(VD_BENCH_FRAME_INPUT);

        BeginDrawing();
        ClearBackground(BLACK);
        bench_frame_mark(VD_BENCH_FRAME_BEGIN);
        bootstrap_draw_deck_canvas(&bootstrap);
        shell_render(&shell);
        bench_frame_mark(VD_BENCH_FRAME_SHELL);
        EndDrawing();
        bench_frame_mark(VD_BENCH_FRAME_PRESENT);
    }

defer:
    shell_shutdown(&shell);
    bootstrap_shutdown(&bootstrap);
    return result;
}
