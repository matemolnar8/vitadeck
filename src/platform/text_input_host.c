#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "text_input.h"

// Room for VD_TEXT_INPUT_MAX_LENGTH code units: at most 3 UTF-8 bytes per unit, plus the terminator.
static char text[VD_TEXT_INPUT_MAX_LENGTH * 3 + 1];
static size_t text_bytes = 0;
static int text_units = 0;
static int max_units = 0;
static bool editing = false;

static int utf16_units(int codepoint)
{
    return codepoint >= 0x10000 ? 2 : 1;
}

static bool append_codepoint(int codepoint)
{
    if (codepoint < 0x20 || codepoint == 0x7f || codepoint > 0x10ffff) return false;
    if (codepoint >= 0xd800 && codepoint <= 0xdfff) return false;
    if (text_units + utf16_units(codepoint) > max_units) return false;

    int size = 0;
    const char *utf8 = CodepointToUTF8(codepoint, &size);
    memcpy(text + text_bytes, utf8, (size_t)size);
    text_bytes += (size_t)size;
    text[text_bytes] = '\0';
    text_units += utf16_units(codepoint);
    return true;
}

static void delete_last_codepoint(void)
{
    if (text_bytes == 0) return;
    size_t start = text_bytes - 1;
    while (start > 0 && ((unsigned char)text[start] & 0xc0) == 0x80)
        start--;
    int size = 0;
    text_units -= utf16_units(GetCodepointNext(text + start, &size));
    text_bytes = start;
    text[text_bytes] = '\0';
}

// Escape quits a raylib app by default; while editing it cancels instead.
static void set_editing(bool value)
{
    editing = value;
    SetExitKey(value ? KEY_NULL : KEY_ESCAPE);
}

bool vd_text_input_begin(const char *title, const char *initial, int max_length)
{
    (void)title;
    if (editing) return false;

    max_units = max_length < 1 ? 1 : max_length > VD_TEXT_INPUT_MAX_LENGTH ? VD_TEXT_INPUT_MAX_LENGTH : max_length;
    text_bytes = 0;
    text_units = 0;
    text[0] = '\0';
    for (const char *p = initial ? initial : ""; *p != '\0';) {
        int size = 0;
        int codepoint = GetCodepointNext(p, &size);
        p += size;
        if (text_units + utf16_units(codepoint) > max_units) break;
        append_codepoint(codepoint);
    }

    set_editing(true);
    return true;
}

VdTextInputStatus vd_text_input_apply_keys(const VdTextInputKeys *keys)
{
    if (!editing) return VD_TEXT_INPUT_IDLE;

    if (keys->cancel) {
        set_editing(false);
        return VD_TEXT_INPUT_CANCELLED;
    }
    for (int i = 0; i < keys->codepoint_count; i++)
        append_codepoint(keys->codepoints[i]);
    if (keys->backspace) delete_last_codepoint();
    if (keys->confirm) {
        set_editing(false);
        return VD_TEXT_INPUT_CONFIRMED;
    }
    return VD_TEXT_INPUT_EDITING;
}

VdTextInputStatus vd_text_input_poll(void)
{
    if (!editing) return VD_TEXT_INPUT_IDLE;

    int codepoints[32];
    int count = 0;
    for (int codepoint = GetCharPressed(); codepoint != 0 && count < 32; codepoint = GetCharPressed())
        codepoints[count++] = codepoint;

    const VdTextInputKeys keys = {
        .codepoints = codepoints,
        .codepoint_count = count,
        .backspace = IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE),
        .confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER),
        .cancel = IsKeyPressed(KEY_ESCAPE),
    };
    return vd_text_input_apply_keys(&keys);
}

const char *vd_text_input_text(void)
{
    return text;
}

void vd_text_input_cancel(void)
{
    if (editing) set_editing(false);
}

bool vd_text_input_edits_in_place(void)
{
    return true;
}
