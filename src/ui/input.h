#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include "platform/text_input.h"

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

// Text Input editing. A Press on a Text Input starts it; while it runs, skip the other polls and call
// poll_text_input every frame, even while the Shell is open, so a closing Vita IME dialog finishes.
bool input_is_editing_text(void);
void poll_text_input(void);
void input_apply_text_input(VdTextInputStatus status);
// Ends the edit without a change, e.g. when the Shell opens.
void input_cancel_text_input(void);
// The text to draw in Text Input id while it is edited in place (the desktop host), or NULL.
const char *input_editing_text(const char *id);

// Forget Focus, held Pad Buttons and any edit. Called when the Deck App runtime starts or stops. A Pad Button
// still held presses on the next sample unless input_ignore_held_pad_buttons asked otherwise.
void input_clear_focus(void);

// Ignore Pad Buttons held right now until they are released, e.g. the Circle that hides the Shell.
void input_ignore_held_pad_buttons(void);

bool input_is_hovered(const char *id);
bool input_is_focused(const char *id);
bool input_is_pressed(const char *id);

#endif /* INPUT_H */
