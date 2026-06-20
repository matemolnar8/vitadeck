#ifndef DECK_BOOTSTRAP_H
#define DECK_BOOTSTRAP_H

#include <stdbool.h>
#include <stddef.h>

#include "js_runtime.h"

#define VD_SCREEN_WIDTH 960
#define VD_SCREEN_HEIGHT 544

// Shared init/teardown between main.c and the smoke harness — not a domain layer.
typedef struct {
    VdJsRuntime js_runtime;
    bool window_open;
} VdDeckBootstrap;

typedef struct {
    unsigned int raylib_config_flags;
} VdDeckBootstrapWindowConfig;

void deck_bootstrap_init(VdDeckBootstrap *bootstrap);
bool deck_bootstrap_boot_subsystems(char *error, size_t error_size);
bool deck_bootstrap_open_window(VdDeckBootstrap *bootstrap, const char *title,
                                const VdDeckBootstrapWindowConfig *window_config, char *error, size_t error_size);
bool deck_bootstrap_start_active_deck_app(VdDeckBootstrap *bootstrap, char *error, size_t error_size);
void deck_bootstrap_draw_loading_splash(void);
void deck_bootstrap_draw_deck_canvas(const VdDeckBootstrap *bootstrap);
void deck_bootstrap_shutdown(VdDeckBootstrap *bootstrap);

#endif /* DECK_BOOTSTRAP_H */
