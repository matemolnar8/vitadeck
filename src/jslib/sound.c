/*
        Deck App Sounds for the Active Deck App, called by `playSound` and `stopSound` in the SDK.
*/
#include "jslib_internal.h"
#include "audio/sounds.h"

static JSValue js_native_play_sound(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 2 || !JS_IsString(argv[0]) || !JS_IsBool(argv[1])) {
        return JS_ThrowTypeError(ctx, "nativePlaySound requires a sound name and a loop flag");
    }
    const char *name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_EXCEPTION;
    bool found = sound_registry_play(name, JS_ToBool(ctx, argv[1]));
    JSValue result = found ? JS_UNDEFINED : JS_ThrowRangeError(ctx, "Unknown Deck App Sound \"%s\"", name);
    JS_FreeCString(ctx, name);
    return result;
}

static JSValue js_native_stop_sound(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 1 || !JS_IsString(argv[0])) return JS_ThrowTypeError(ctx, "nativeStopSound requires a sound name");
    const char *name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_EXCEPTION;
    bool found = sound_registry_stop(name);
    JSValue result = found ? JS_UNDEFINED : JS_ThrowRangeError(ctx, "Unknown Deck App Sound \"%s\"", name);
    JS_FreeCString(ctx, name);
    return result;
}

void register_js_sound(JSContext *ctx)
{
    js_set_global_function(ctx, "nativePlaySound", js_native_play_sound, 2);
    js_set_global_function(ctx, "nativeStopSound", js_native_stop_sound, 1);
}
