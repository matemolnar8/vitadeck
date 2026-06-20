#ifndef DECK_HOST_H
#define DECK_HOST_H

#include <stdbool.h>
#include <stddef.h>

#include "js_runtime.h"

#define VD_SCREEN_WIDTH 960
#define VD_SCREEN_HEIGHT 544

typedef struct {
    VdJsRuntime js_runtime;
    bool window_open;
} VdDeckHost;

typedef struct {
    unsigned int raylib_config_flags;
    bool set_raylib_config_flags;
} VdDeckHostWindowConfig;

void deck_host_init(VdDeckHost *host);
bool deck_host_boot_subsystems(char *error, size_t error_size);
bool deck_host_open_window(VdDeckHost *host, const char *title, const VdDeckHostWindowConfig *window_config,
                           char *error, size_t error_size);
bool deck_host_start_active_deck_app(VdDeckHost *host, char *error, size_t error_size);
void deck_host_draw_loading_splash(void);
void deck_host_draw_deck_canvas(const VdDeckHost *host);
void deck_host_shutdown(VdDeckHost *host);

void deck_host_configure_smoke_window_flags(void);

#endif /* DECK_HOST_H */
