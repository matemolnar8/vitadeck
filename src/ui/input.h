#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

// Pad Buttons a Deck App can subscribe to. The D-pad, Cross, Start and Select never reach this list.
typedef enum { PAD_CIRCLE, PAD_TRIANGLE, PAD_SQUARE, PAD_L, PAD_R, PAD_BUTTON_COUNT } PadButton;

typedef struct {
    int x, y;
    bool left_down;
    float wheel;
} MouseSample;

// Directions are pressed this frame; confirm and pad_buttons are held.
typedef struct {
    bool up, down, left, right;
    bool confirm;
    float stick_y;
    float dt;
    bool pad_buttons[PAD_BUTTON_COUNT];
} GamepadSample;

// Sample raylib and apply it. Call from the UI thread while the Deck App owns input.
void poll_mouse_input(void);
void poll_touch_input(void);
void poll_gamepad_input(void);

// The input logic behind the polls, without raylib reads, so harnesses can drive it.
void input_apply_mouse(const MouseSample *sample);
void input_apply_gamepad(const GamepadSample *sample);

// Forget Focus and held Pad Buttons. Called when the Deck App runtime starts or stops.
void input_clear_focus(void);

// Ignore Pad Buttons held right now until they are released, e.g. the Circle that hides the Shell.
void input_ignore_held_pad_buttons(void);

bool input_is_hovered(const char *id);
bool input_is_focused(const char *id);
bool input_is_pressed(const char *id);

#endif /* INPUT_H */
