#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

void poll_mouse_input(void);
void poll_touch_input(void);
void poll_gamepad_input(void);

void input_clear_focus(void);

bool input_is_hovered(const char *id);
bool input_is_focused(const char *id);
bool input_is_pressed(const char *id);

#ifdef VITADECK_INPUT_TESTING
void input_test_reset_state(void);
void input_test_set_touch_hovered(const char *id);
void input_test_set_focused(const char *id);
void input_test_begin_touch_scroll_release(const char *id, float velocity_y);
void input_test_release_touch_scroll(void);
bool input_test_momentum_active(const char *id);
float input_test_momentum_velocity(void);
#endif

#endif /* INPUT_H */
