#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <stdbool.h>
#include <stddef.h>

#include "js_runtime.h"

#define VD_SCREEN_WIDTH 960
#define VD_SCREEN_HEIGHT 544

// Shared init/teardown between main.c and the smoke harness — not a domain layer.
typedef struct {
    VdJsRuntime js_runtime;
    bool window_open;
} VdBootstrap;

typedef struct {
    unsigned int raylib_config_flags;
} VdBootstrapWindowConfig;

void bootstrap_init(VdBootstrap *bootstrap);
bool bootstrap_boot_subsystems(char *error, size_t error_size);
bool bootstrap_open_window(VdBootstrap *bootstrap, const char *title, const VdBootstrapWindowConfig *window_config,
                           char *error, size_t error_size);
bool bootstrap_reload_active_package_fonts(VdBootstrap *bootstrap, char *error, size_t error_size);
bool bootstrap_start_active_deck_app(VdBootstrap *bootstrap, char *error, size_t error_size);
void bootstrap_draw_loading_splash(void);
void bootstrap_draw_deck_canvas(const VdBootstrap *bootstrap);
void bootstrap_shutdown(VdBootstrap *bootstrap);

#endif /* BOOTSTRAP_H */
