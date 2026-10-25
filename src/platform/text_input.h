#ifndef VD_TEXT_INPUT_H
#define VD_TEXT_INPUT_H

#include <stdbool.h>

// One Text Input editing session at a time, driven from the UI thread. The Vita opens the system IME dialog, which
// EndDrawing draws over the frame; the desktop host edits in place with the keyboard. Text is UTF-8, and lengths
// count UTF-16 code units like a JS string's length.

// The Vita IME dialog's limit.
#define VD_TEXT_INPUT_MAX_LENGTH 2048

typedef enum {
    VD_TEXT_INPUT_IDLE,
    VD_TEXT_INPUT_EDITING,
    VD_TEXT_INPUT_CONFIRMED,
    VD_TEXT_INPUT_CANCELLED,
} VdTextInputStatus;

// Starts editing initial, cut to max_length (clamped to 1..VD_TEXT_INPUT_MAX_LENGTH). title is the Vita IME
// dialog's title. Returns false when a session is still open or the dialog would not start.
bool vd_text_input_begin(const char *title, const char *initial, int max_length);

// Call every frame. Returns CONFIRMED or CANCELLED once when a session ends, EDITING while it runs, IDLE otherwise.
VdTextInputStatus vd_text_input_poll(void);

// While editing in place, the text so far; after CONFIRMED, the confirmed text until the next begin.
const char *vd_text_input_text(void);

// Ends the session without confirming. The Vita dialog closes over the next polls.
void vd_text_input_cancel(void);

// True on the desktop host, where the field itself shows the text being edited.
bool vd_text_input_edits_in_place(void);

// Desktop host only: the keyboard input behind vd_text_input_poll, so harnesses can drive it without a window.
typedef struct {
    const int *codepoints;
    int codepoint_count;
    bool backspace;
    bool confirm;
    bool cancel;
} VdTextInputKeys;

VdTextInputStatus vd_text_input_apply_keys(const VdTextInputKeys *keys);

#endif /* VD_TEXT_INPUT_H */
