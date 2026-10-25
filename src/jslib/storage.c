/*
        Deck App Storage for the Active Deck App. The SDK's `storage` keeps the items in memory
        and rewrites the whole JSON object on every change.
*/
#include "jslib_internal.h"
#include "core/deck_app_storage.h"
#include "core/package_library.h"

// Captured when the JS thread starts, so a Shell switch cannot redirect a late write to the next Deck App.
static char g_package_name[VD_PACKAGE_NAME_MAX];

static JSValue js_native_read_deck_app_storage(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    (void)argc;
    (void)argv;
    size_t len = 0;
    char *json = deck_app_storage_read(package_library_data_root(), g_package_name, &len);
    if (!json) return JS_ThrowInternalError(ctx, "Could not read Deck App Storage.");
    JSValue result = JS_NewStringLen(ctx, json, len);
    free(json);
    return result;
}

static JSValue js_native_write_deck_app_storage(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 1 || !JS_IsString(argv[0])) {
        return JS_ThrowTypeError(ctx, "nativeWriteDeckAppStorage requires a JSON string");
    }
    size_t len = 0;
    const char *json = JS_ToCStringLen(ctx, &len, argv[0]);
    if (!json) return JS_EXCEPTION;

    char error[128] = "";
    bool ok = deck_app_storage_write(package_library_data_root(), g_package_name, json, len, error, sizeof(error));
    JS_FreeCString(ctx, json);
    if (!ok) {
        return len > VD_DECK_APP_STORAGE_MAX ? JS_ThrowRangeError(ctx, "%s", error)
                                             : JS_ThrowInternalError(ctx, "%s", error);
    }
    return JS_UNDEFINED;
}

void register_js_storage(JSContext *ctx)
{
    package_library_active_package_name(g_package_name, sizeof(g_package_name));
    js_set_global_function(ctx, "nativeReadDeckAppStorage", js_native_read_deck_app_storage, 0);
    js_set_global_function(ctx, "nativeWriteDeckAppStorage", js_native_write_deck_app_storage, 1);
}
