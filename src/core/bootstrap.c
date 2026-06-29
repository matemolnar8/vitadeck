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
    const char *package_path =
        package_library_has_active_deck_app() ? package_library_active_package_path() : "";
    if (!font_registry_load_package(package_path, error, error_size)) {
        bootstrap->js_runtime.failed = true;
        return false;
    }
    if (!image_registry_load_package(package_path, error, error_size)) {
        bootstrap->js_runtime.failed = true;
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

void bootstrap_draw_deck_canvas(const VdBootstrap *bootstrap)
{
    if (!package_library_has_active_deck_app()) return;

    if (js_runtime_failed(&bootstrap->js_runtime)) {
        DrawText("Deck App runtime failed to start.", 10, 10, 24, RED);
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
