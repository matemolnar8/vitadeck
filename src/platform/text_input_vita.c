#include <string.h>
#include <psp2/kernel/clib.h>
#include <psp2/common_dialog.h>
#include <psp2/ime_dialog.h>
#include <psp2/sysmodule.h>
#include <raylib.h>
#include "text_input.h"

// raylib-vita's SDL swap calls vglSwapBuffers(GL_TRUE), so EndDrawing already updates the common dialog each frame.
typedef enum { IME_IDLE, IME_RUNNING, IME_ABORTING } ImeState;

static ImeState state = IME_IDLE;
static SceWChar16 title16[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
static SceWChar16 initial16[VD_TEXT_INPUT_MAX_LENGTH + 1];
static SceWChar16 input16[VD_TEXT_INPUT_MAX_LENGTH + 1];
static char result[VD_TEXT_INPUT_MAX_LENGTH * 3 + 1];

// Writes at most capacity - 1 code units and a terminator, never splitting a surrogate pair.
static void utf8_to_utf16(const char *src, SceWChar16 *out, int capacity)
{
    int n = 0;
    while (*src != '\0') {
        int size = 0;
        int codepoint = GetCodepointNext(src, &size);
        src += size;
        if (codepoint >= 0x10000) {
            if (n + 2 >= capacity) break;
            codepoint -= 0x10000;
            out[n++] = (SceWChar16)(0xd800 + (codepoint >> 10));
            out[n++] = (SceWChar16)(0xdc00 + (codepoint & 0x3ff));
        } else {
            if (n + 1 >= capacity) break;
            out[n++] = (SceWChar16)codepoint;
        }
    }
    out[n] = 0;
}

// Unpaired surrogates are dropped.
static void utf16_to_utf8(const SceWChar16 *src, char *out, size_t capacity)
{
    size_t n = 0;
    for (; *src != 0; src++) {
        int codepoint = *src;
        if (codepoint >= 0xd800 && codepoint <= 0xdbff && src[1] >= 0xdc00 && src[1] <= 0xdfff) {
            codepoint = 0x10000 + ((codepoint - 0xd800) << 10) + (src[1] - 0xdc00);
            src++;
        } else if (codepoint >= 0xd800 && codepoint <= 0xdfff) {
            continue;
        }
        int size = 0;
        const char *utf8 = CodepointToUTF8(codepoint, &size);
        if (n + (size_t)size >= capacity) break;
        memcpy(out + n, utf8, (size_t)size);
        n += (size_t)size;
    }
    out[n] = '\0';
}

bool vd_text_input_begin(const char *title, const char *initial, int max_length)
{
    static bool ime_module_loaded = false;
    if (state != IME_IDLE) return false;
    if (!ime_module_loaded) {
        sceSysmoduleLoadModule(SCE_SYSMODULE_IME);
        ime_module_loaded = true;
    }

    int max_units = max_length < 1 ? 1 : max_length > VD_TEXT_INPUT_MAX_LENGTH ? VD_TEXT_INPUT_MAX_LENGTH : max_length;
    utf8_to_utf16(title ? title : "", title16, SCE_IME_DIALOG_MAX_TITLE_LENGTH);
    utf8_to_utf16(initial ? initial : "", initial16, max_units + 1);
    memset(input16, 0, sizeof(input16));

    SceImeDialogParam param;
    sceImeDialogParamInit(&param);
    param.supportedLanguages = SCE_IME_LANGUAGE_ENGLISH;
    param.languagesForced = SCE_FALSE;
    param.type = SCE_IME_TYPE_DEFAULT;
    param.dialogMode = SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL;
    param.textBoxMode = SCE_IME_DIALOG_TEXTBOX_MODE_WITH_CLEAR;
    param.title = title16;
    param.maxTextLength = (SceUInt32)max_units;
    param.initialText = initial16;
    param.inputTextBuffer = input16;
    if (sceImeDialogInit(&param) < 0) {
        TraceLog(LOG_WARNING, "IME dialog did not start");
        return false;
    }
    state = IME_RUNNING;
    return true;
}

VdTextInputStatus vd_text_input_poll(void)
{
    if (state == IME_IDLE) return VD_TEXT_INPUT_IDLE;
    if (sceImeDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        return state == IME_RUNNING ? VD_TEXT_INPUT_EDITING : VD_TEXT_INPUT_IDLE;
    }

    VdTextInputStatus status = VD_TEXT_INPUT_CANCELLED;
    if (state == IME_RUNNING) {
        SceImeDialogResult dialog_result;
        memset(&dialog_result, 0, sizeof(dialog_result));
        sceImeDialogGetResult(&dialog_result);
        if (dialog_result.button == SCE_IME_DIALOG_BUTTON_ENTER) {
            utf16_to_utf8(input16, result, sizeof(result));
            status = VD_TEXT_INPUT_CONFIRMED;
        }
    }
    sceImeDialogTerm();
    state = IME_IDLE;
    return status;
}

const char *vd_text_input_text(void)
{
    return result;
}

void vd_text_input_cancel(void)
{
    if (state != IME_RUNNING) return;
    sceImeDialogAbort();
    state = IME_ABORTING;
}

bool vd_text_input_edits_in_place(void)
{
    return false;
}
