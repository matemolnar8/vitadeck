#ifndef JSLIB_H
#define JSLIB_H

#include <stdbool.h>
#include "quickjs.h"

/**
 * Initialize and register all JavaScript library bindings
 * Registers: log, draw, timeout, input
 */
void register_js_lib(JSContext *ctx);

/**
 * Poll mouse input and push events to queue (UI thread)
 */
void poll_mouse_input(void);

/**
 * Poll touch input and push events to queue (UI thread)
 */
void poll_touch_input(void);

/**
 * Process input events from queue (JS thread)
 */
void process_input_events(JSContext *ctx);

/**
 * Legacy: Process mouse input (calls poll_mouse_input internally)
 */
void process_mouse_input(JSContext *ctx);

/**
 * Legacy: Process touch input (calls poll_touch_input internally)
 */
void process_touch_input(JSContext *ctx);

/**
 * Run all timeouts and intervals
 * Called from the main loop every frame, checks if any timeouts or intervals are due and runs them
 */
void run_timeouts(JSContext *ctx);

// Draw list rendering API
void render_draw_list(void);

// Input helpers for rendering logic
bool input_is_hovered(const char *id);
bool input_is_pressed(const char *id);

// Instance tree initialization (call once at startup before any threads)
void instance_tree_init(void);

// Instance tree double-buffer swap (call from JS thread after mutations)
void instance_tree_swap(void);

// Render lock/unlock - UI thread must hold lock during entire render
void instance_tree_render_lock(void);
void instance_tree_render_unlock(void);

#endif /* JSLIB_H */
