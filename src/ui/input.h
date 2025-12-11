#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

void poll_mouse_input(void);
void poll_touch_input(void);

bool input_is_hovered(const char *id);
bool input_is_pressed(const char *id);

#endif /* INPUT_H */
