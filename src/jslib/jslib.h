#ifndef JSLIB_H
#define JSLIB_H

#include <mujs.h>

/**
 * Initialize and register all JavaScript library bindings
 * Registers: log, draw, timeout, input
 */
void register_js_lib(js_State *J);

/**
 * Process mouse input and trigger appropriate JavaScript events
 * Called from the main loop to handle mouse interactions
 */
void process_mouse_input(js_State *J);

/**
 * Process touch input and trigger appropriate JavaScript events
 * Called from the main loop to handle touch interactions
 */
void process_touch_input(js_State *J);

/**
 * Run all timeouts and intervals
 * Called from the main loop every frame, checks if any timeouts or intervals are due and runs them
 */
void run_timeouts(js_State *J);

#endif /* JSLIB_H */
