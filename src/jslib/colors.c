static JSValue create_color_object(JSContext *ctx, Color c) {
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "r", JS_NewInt32(ctx, c.r));
    JS_SetPropertyStr(ctx, obj, "g", JS_NewInt32(ctx, c.g));
    JS_SetPropertyStr(ctx, obj, "b", JS_NewInt32(ctx, c.b));
    JS_SetPropertyStr(ctx, obj, "a", JS_NewInt32(ctx, c.a));
    return obj;
}

void register_js_colors(JSContext *ctx) {
    JSValue colors = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, colors, "LIGHTGRAY", create_color_object(ctx, LIGHTGRAY));
    JS_SetPropertyStr(ctx, colors, "GRAY", create_color_object(ctx, GRAY));
    JS_SetPropertyStr(ctx, colors, "DARKGRAY", create_color_object(ctx, DARKGRAY));
    JS_SetPropertyStr(ctx, colors, "YELLOW", create_color_object(ctx, YELLOW));
    JS_SetPropertyStr(ctx, colors, "GOLD", create_color_object(ctx, GOLD));
    JS_SetPropertyStr(ctx, colors, "ORANGE", create_color_object(ctx, ORANGE));
    JS_SetPropertyStr(ctx, colors, "PINK", create_color_object(ctx, PINK));
    JS_SetPropertyStr(ctx, colors, "RED", create_color_object(ctx, RED));
    JS_SetPropertyStr(ctx, colors, "MAROON", create_color_object(ctx, MAROON));
    JS_SetPropertyStr(ctx, colors, "GREEN", create_color_object(ctx, GREEN));
    JS_SetPropertyStr(ctx, colors, "LIME", create_color_object(ctx, LIME));
    JS_SetPropertyStr(ctx, colors, "DARKGREEN", create_color_object(ctx, DARKGREEN));
    JS_SetPropertyStr(ctx, colors, "SKYBLUE", create_color_object(ctx, SKYBLUE));
    JS_SetPropertyStr(ctx, colors, "BLUE", create_color_object(ctx, BLUE));
    JS_SetPropertyStr(ctx, colors, "DARKBLUE", create_color_object(ctx, DARKBLUE));
    JS_SetPropertyStr(ctx, colors, "PURPLE", create_color_object(ctx, PURPLE));
    JS_SetPropertyStr(ctx, colors, "VIOLET", create_color_object(ctx, VIOLET));
    JS_SetPropertyStr(ctx, colors, "DARKPURPLE", create_color_object(ctx, DARKPURPLE));
    JS_SetPropertyStr(ctx, colors, "BEIGE", create_color_object(ctx, BEIGE));
    JS_SetPropertyStr(ctx, colors, "BROWN", create_color_object(ctx, BROWN));
    JS_SetPropertyStr(ctx, colors, "DARKBROWN", create_color_object(ctx, DARKBROWN));
    JS_SetPropertyStr(ctx, colors, "WHITE", create_color_object(ctx, WHITE));
    JS_SetPropertyStr(ctx, colors, "BLACK", create_color_object(ctx, BLACK));
    JS_SetPropertyStr(ctx, colors, "BLANK", create_color_object(ctx, BLANK));
    JS_SetPropertyStr(ctx, colors, "MAGENTA", create_color_object(ctx, MAGENTA));
    JS_SetPropertyStr(ctx, colors, "RAYWHITE", create_color_object(ctx, RAYWHITE));
    
    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "Colors", colors);
    JS_FreeValue(ctx, global);
}
