#include "bootstrap.h"

#include <raylib.h>
#include <stdio.h>

#include "core/event_queue.h"
#include "core/package_library.h"
#include "ui/fonts.h"
#include "ui/images.h"
#include "ui/instance_tree.h"
#include "ui/render.h"

static bool load_active_package_assets(VdBootstrap *bootstrap, char *error, size_t error_size)
{
    char package_path[VD_PATH_MAX] = "";
    if (package_library_has_active_deck_app()) package_library_active_package_path(package_path, sizeof(package_path));
    if (!font_registry_load_package(package_path, error, error_size)) {
        js_runtime_fail(&bootstrap->js_runtime, error, NULL);
        image_registry_load_package("", NULL, 0);
        return false;
    }
    if (!image_registry_load_package(package_path, error, error_size)) {
        js_runtime_fail(&bootstrap->js_runtime, error, NULL);
        font_registry_load_package("", error, error_size);
        return false;
    }
    return true;
}

void bootstrap_init(VdBootstrap *bootstrap)
{
    js_runtime_init(&bootstrap->js_runtime);
    bootstrap->window_open = false;
}

bool bootstrap_boot_subsystems(char *error, size_t error_size)
{
    if (!event_queue_init()) {
        if (error && error_size > 0) snprintf(error, error_size, "Could not initialize event queue.");
        return false;
    }

    instance_tree_init();

    if (!package_library_init(error, error_size)) return false;
    return true;
}

bool bootstrap_open_window(VdBootstrap *bootstrap, const char *title, const VdBootstrapWindowConfig *window_config,
                           char *error, size_t error_size)
{
    if (window_config) SetConfigFlags(window_config->raylib_config_flags);

    InitWindow(VD_SCREEN_WIDTH, VD_SCREEN_HEIGHT, title ? title : "VitaDeck");
    if (!IsWindowReady()) {
        if (error && error_size > 0) snprintf(error, error_size, "Could not initialize raylib window.");
        return false;
    }

    bootstrap->window_open = true;
    SetTargetFPS(60);

    if (!font_registry_init(error, error_size)) return false;
    load_active_package_assets(bootstrap, error, error_size);
    return true;
}

bool bootstrap_reload_active_package_fonts(VdBootstrap *bootstrap, char *error, size_t error_size)
{
    return load_active_package_assets(bootstrap, error, error_size);
}

bool bootstrap_start_active_deck_app(VdBootstrap *bootstrap, char *error, size_t error_size)
{
    if (!package_library_has_active_deck_app()) {
        if (error && error_size > 0) snprintf(error, error_size, "No active Deck App configured.");
        return false;
    }

    if (!js_runtime_start(&bootstrap->js_runtime)) {
        if (error && error_size > 0) snprintf(error, error_size, "Could not create JS thread.");
        return false;
    }

    return true;
}

void bootstrap_draw_loading_splash(void)
{
    BeginDrawing();
    ClearBackground(BLACK);
    if (package_library_has_active_deck_app()) DrawText("Loading...", 10, 10, 20, WHITE);
    EndDrawing();
}

#define FAILURE_MARGIN 20

// Draws up to max_lines of text, breaking at newlines and at the screen edge. Returns the y below the last line.
static int draw_failure_lines(const char *text, int y, int font_size, Color color, int max_lines)
{
    const int max_width = VD_SCREEN_WIDTH - 2 * FAILURE_MARGIN;
    char line[256];
    for (int drawn = 0; *text && drawn < max_lines; drawn++) {
        size_t len = 0;
        int width = 0;
        while (text[len] && text[len] != '\n' && len < sizeof(line) - 1) {
            char glyph[2] = {text[len], '\0'};
            width += MeasureText(glyph, font_size) + font_size / 10;
            if (width > max_width && len > 0) break;
            line[len] = text[len];
            len++;
        }
        line[len] = '\0';
        DrawText(line, FAILURE_MARGIN, y, font_size, color);
        y += font_size + 4;
        text += len;
        if (*text == '\n') text++;
    }
    return y;
}

static void draw_deck_app_failure_screen(const VdJsRuntime *runtime)
{
    DrawText("Deck App failed", FAILURE_MARGIN, FAILURE_MARGIN, 30, RED);
    int y = draw_failure_lines(runtime->failure_message, FAILURE_MARGIN + 46, 20, RAYWHITE, 4);
    draw_failure_lines(runtime->failure_stack, y + 12, 16, GRAY, 12);
    DrawText("Press Start (F1) to open the VitaDeck Shell.", FAILURE_MARGIN, VD_SCREEN_HEIGHT - FAILURE_MARGIN - 20, 20,
             RAYWHITE);
}

void bootstrap_draw_deck_canvas(const VdBootstrap *bootstrap)
{
    if (!package_library_has_active_deck_app()) return;

    if (js_runtime_failed(&bootstrap->js_runtime)) {
        draw_deck_app_failure_screen(&bootstrap->js_runtime);
    } else if (!js_runtime_is_ready(&bootstrap->js_runtime)) {
        DrawText("Loading JavaScript...", 10, 10, 20, WHITE);
    } else {
        render_draw_list();
    }
}

void bootstrap_shutdown(VdBootstrap *bootstrap)
{
    js_runtime_stop(&bootstrap->js_runtime);
    image_registry_shutdown();
    font_registry_shutdown();
    event_queue_shutdown();
    event_queue_destroy();
    if (bootstrap->window_open) {
        CloseWindow();
        bootstrap->window_open = false;
    }
}
