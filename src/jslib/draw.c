/*
    Helper: push a JS Color object from a raylib Color
*/
static void push_color_object(js_State *J, Color c) {
    js_newobject(J);
    js_pushnumber(J, c.r);
    js_setproperty(J, -2, "r");
    js_pushnumber(J, c.g);
    js_setproperty(J, -2, "g");
    js_pushnumber(J, c.b);
    js_setproperty(J, -2, "b");
    js_pushnumber(J, c.a);
    js_setproperty(J, -2, "a");
}

static Color parse_color_arg_or_default(js_State *J, int index, Color fallback)
{
    if (js_isundefined(J, index) || !js_isobject(J, index)) {
        return fallback;
    }

    Color c = fallback;

    js_getproperty(J, index, "r");
    if (js_isnumber(J, -1)) c.r = (unsigned char)js_tointeger(J, -1);
    js_pop(J, 1);

    js_getproperty(J, index, "g");
    if (js_isnumber(J, -1)) c.g = (unsigned char)js_tointeger(J, -1);
    js_pop(J, 1);

    js_getproperty(J, index, "b");
    if (js_isnumber(J, -1)) c.b = (unsigned char)js_tointeger(J, -1);
    js_pop(J, 1);

    js_getproperty(J, index, "a");
    if (js_isnumber(J, -1)) c.a = (unsigned char)js_tointeger(J, -1);
    js_pop(J, 1);

    return c;
}

/*
    drawRect(x: number, y: number, width: number, height: number, fillColor?: Color, outlineColor?: Color): void
    Draws a rectangle with optional fill and/or outline in a single call.
    If fillColor is provided, draws the filled rectangle.
    If outlineColor is provided, draws the outline.
*/
static void draw_rect(js_State *J) {
    const int x = js_tointeger(J, 1);
    const int y = js_tointeger(J, 2);
    const int w = js_tointeger(J, 3);
    const int h = js_tointeger(J, 4);

    // Check if fillColor is provided (not undefined/null)
    Color fillColor;
    bool hasFill = false;
    if (!js_isundefined(J, 5) && js_isobject(J, 5)) {
        fillColor = parse_color_arg_or_default(J, 5, BLANK);
        hasFill = true;
    }

    // Check if outlineColor is provided (not undefined/null)
    Color outlineColor;
    bool hasOutline = false;
    if (!js_isundefined(J, 6) && js_isobject(J, 6)) {
        outlineColor = parse_color_arg_or_default(J, 6, DARKGRAY);
        hasOutline = true;
    }

    // Draw fill first (if provided)
    if (hasFill) {
        DrawRectangle(x, y, w, h, fillColor);
    }

    // Draw outline (if provided)
    if (hasOutline) {
        DrawRectangleLines(x, y, w, h, outlineColor);
    }

    js_pushundefined(J);
}

/*
    drawText(x: number, y: number, fontSize: number, text: string, color?: Color, border?: boolean): void
*/
static void draw_text(js_State *J) {
    const int x = js_tointeger(J, 1);
    const int y = js_tointeger(J, 2);
    const int font_size = js_tointeger(J, 3);
    const char *str = js_tostring(J, 4);
    const bool border = js_toboolean(J, 6);
    const Color color = parse_color_arg_or_default(J, 5, BLACK);

    int text_width = MeasureText(str, font_size);
    if (border) {
        int border_padding = 4;
        Rectangle rect = { x - border_padding, y - border_padding, text_width + border_padding * 2, font_size + border_padding * 2 };
        DrawRectangleLinesEx(rect, 2, color);
    }

    DrawText(str, x, y, font_size, color);

    js_pushundefined(J);
}

void register_js_draw(js_State *J) {
    js_newcfunction(J, draw_rect, "drawRect", 0);
	js_setglobal(J, "drawRect");

	js_newcfunction(J, draw_text, "drawText", 0);
	js_setglobal(J, "drawText");

    // Create Colors object from raylib's Color constants
	js_newobject(J);
	push_color_object(J, LIGHTGRAY); js_setproperty(J, -2, "LIGHTGRAY");
	push_color_object(J, GRAY); js_setproperty(J, -2, "GRAY");
	push_color_object(J, DARKGRAY); js_setproperty(J, -2, "DARKGRAY");
	push_color_object(J, YELLOW); js_setproperty(J, -2, "YELLOW");
	push_color_object(J, GOLD); js_setproperty(J, -2, "GOLD");
	push_color_object(J, ORANGE); js_setproperty(J, -2, "ORANGE");
	push_color_object(J, PINK); js_setproperty(J, -2, "PINK");
	push_color_object(J, RED); js_setproperty(J, -2, "RED");
	push_color_object(J, MAROON); js_setproperty(J, -2, "MAROON");
	push_color_object(J, GREEN); js_setproperty(J, -2, "GREEN");
	push_color_object(J, LIME); js_setproperty(J, -2, "LIME");
	push_color_object(J, DARKGREEN); js_setproperty(J, -2, "DARKGREEN");
	push_color_object(J, SKYBLUE); js_setproperty(J, -2, "SKYBLUE");
	push_color_object(J, BLUE); js_setproperty(J, -2, "BLUE");
	push_color_object(J, DARKBLUE); js_setproperty(J, -2, "DARKBLUE");
	push_color_object(J, PURPLE); js_setproperty(J, -2, "PURPLE");
	push_color_object(J, VIOLET); js_setproperty(J, -2, "VIOLET");
	push_color_object(J, DARKPURPLE); js_setproperty(J, -2, "DARKPURPLE");
	push_color_object(J, BEIGE); js_setproperty(J, -2, "BEIGE");
	push_color_object(J, BROWN); js_setproperty(J, -2, "BROWN");
	push_color_object(J, DARKBROWN); js_setproperty(J, -2, "DARKBROWN");
	push_color_object(J, WHITE); js_setproperty(J, -2, "WHITE");
	push_color_object(J, BLACK); js_setproperty(J, -2, "BLACK");
	push_color_object(J, BLANK); js_setproperty(J, -2, "BLANK");
	push_color_object(J, MAGENTA); js_setproperty(J, -2, "MAGENTA");
	push_color_object(J, RAYWHITE); js_setproperty(J, -2, "RAYWHITE");
	js_setglobal(J, "Colors");
}