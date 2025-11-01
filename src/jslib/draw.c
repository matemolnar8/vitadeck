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

// =============================================================================
// Flat draw list: sync from JS and render per frame
// =============================================================================

typedef enum {
    DC_RECT = 1,
    DC_TEXT = 2,
    DC_BUTTON = 3
} DrawCommandType;

typedef struct {
    int x, y, w, h;
    bool has_fill, has_outline;
    Color fill, outline;
} DcRect;

typedef struct {
    int x, y, font_size;
    char *text;
    Color color;
    bool has_color;
    bool border;
} DcText;

typedef struct {
    char *id;
    int x, y, w, h;
    Color base_color;
    char *label;
    int font_size; // optional, 0 when unset
} DcButton;

typedef struct {
    DrawCommandType type;
    union {
        DcRect rect;
        DcText text;
        DcButton button;
    } as;
} DrawCommand;

static DrawCommand *draw_commands = NULL;

static void free_draw_command(DrawCommand *cmd)
{
    if (!cmd) return;
    switch (cmd->type) {
        case DC_TEXT:
            if (cmd->as.text.text) free(cmd->as.text.text);
            break;
        case DC_BUTTON:
            if (cmd->as.button.id) free(cmd->as.button.id);
            if (cmd->as.button.label) free(cmd->as.button.label);
            break;
        default:
            break;
    }
}

static Color mix_color(Color c, Color mix_with, float amount)
{
    Color out = {0};
    out.r = (unsigned char)(c.r + (mix_with.r - c.r) * amount);
    out.g = (unsigned char)(c.g + (mix_with.g - c.g) * amount);
    out.b = (unsigned char)(c.b + (mix_with.b - c.b) * amount);
    out.a = c.a;
    return out;
}

static void sync_draw_list(js_State *J)
{
    // Free previous
    if (draw_commands) {
        int count = arrlen(draw_commands);
        for (int i = 0; i < count; i++) free_draw_command(&draw_commands[i]);
        arrfree(draw_commands);
        draw_commands = NULL;
    }

    // vitadeck.draw.commands
    js_getglobal(J, "vitadeck");
    js_getproperty(J, -1, "draw");
    js_getproperty(J, -1, "commands");

    int length = js_getlength(J, -1);
    for (int i = 0; i < length; i++) {
        js_getindex(J, -1, i);

        js_getproperty(J, -1, "type");
        const char *type = js_tostring(J, -1);
        js_pop(J, 1);

        if (strcmp(type, "rect") == 0) {
            DrawCommand cmd = {0};
            cmd.type = DC_RECT;
            js_getproperty(J, -1, "x"); cmd.as.rect.x = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "y"); cmd.as.rect.y = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "width"); cmd.as.rect.w = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "height"); cmd.as.rect.h = js_tointeger(J, -1); js_pop(J, 1);

            js_getproperty(J, -1, "fillColor");
            if (!js_isundefined(J, -1) && js_isobject(J, -1)) {
                cmd.as.rect.fill = parse_color_arg_or_default(J, -1, BLANK);
                cmd.as.rect.has_fill = true;
            }
            js_pop(J, 1);

            js_getproperty(J, -1, "outlineColor");
            if (!js_isundefined(J, -1) && js_isobject(J, -1)) {
                cmd.as.rect.outline = parse_color_arg_or_default(J, -1, DARKGRAY);
                cmd.as.rect.has_outline = true;
            }
            js_pop(J, 1);

            arrput(draw_commands, cmd);
        } else if (strcmp(type, "text") == 0) {
            DrawCommand cmd = {0};
            cmd.type = DC_TEXT;
            js_getproperty(J, -1, "x"); cmd.as.text.x = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "y"); cmd.as.text.y = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "fontSize"); cmd.as.text.font_size = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "text"); cmd.as.text.text = strdup(js_tostring(J, -1)); js_pop(J, 1);

            js_getproperty(J, -1, "color");
            if (!js_isundefined(J, -1) && js_isobject(J, -1)) {
                cmd.as.text.color = parse_color_arg_or_default(J, -1, BLACK);
                cmd.as.text.has_color = true;
            }
            js_pop(J, 1);

            js_getproperty(J, -1, "border");
            cmd.as.text.border = js_toboolean(J, -1);
            js_pop(J, 1);

            arrput(draw_commands, cmd);
        } else if (strcmp(type, "button") == 0) {
            DrawCommand cmd = {0};
            cmd.type = DC_BUTTON;
            js_getproperty(J, -1, "id"); cmd.as.button.id = strdup(js_tostring(J, -1)); js_pop(J, 1);
            js_getproperty(J, -1, "x"); cmd.as.button.x = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "y"); cmd.as.button.y = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "width"); cmd.as.button.w = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "height"); cmd.as.button.h = js_tointeger(J, -1); js_pop(J, 1);
            js_getproperty(J, -1, "baseColor"); cmd.as.button.base_color = parse_color_arg_or_default(J, -1, DARKBLUE); js_pop(J, 1);
            js_getproperty(J, -1, "label"); cmd.as.button.label = strdup(js_tostring(J, -1)); js_pop(J, 1);
            js_getproperty(J, -1, "fontSize");
            if (!js_isundefined(J, -1) && js_isnumber(J, -1)) cmd.as.button.font_size = js_tointeger(J, -1);
            js_pop(J, 1);

            arrput(draw_commands, cmd);
        }

        js_pop(J, 1); // pop command object
    }

    // pop stack: commands, draw, vitadeck
    js_pop(J, 3);
}

static void render_rect(const DcRect *r)
{
    if (r->has_fill) DrawRectangle(r->x, r->y, r->w, r->h, r->fill);
    if (r->has_outline) DrawRectangleLines(r->x, r->y, r->w, r->h, r->outline);
}

static void render_text(const DcText *t)
{
    Color color = t->has_color ? t->color : BLACK;
    if (t->border) {
        int text_width = MeasureText(t->text, t->font_size);
        int border_padding = 4;
        Rectangle rect = { t->x - border_padding, t->y - border_padding, text_width + border_padding * 2, t->font_size + border_padding * 2 };
        DrawRectangleLinesEx(rect, 2, color);
    }
    DrawText(t->text, t->x, t->y, t->font_size, color);
}

static void render_button(const DcButton *b)
{
    bool hovered = input_is_hovered(b->id);
    bool pressed = input_is_pressed(b->id);

    Color visual = b->base_color;
    if (pressed) visual = mix_color(visual, BLACK, 0.5f);
    else if (hovered) visual = mix_color(visual, WHITE, 0.4f);

    DrawRectangle(b->x, b->y, b->w, b->h, visual);

    const int padding = 8;
    const int font_size = b->font_size > 0 ? b->font_size : 20;
    DrawText(b->label, b->x + padding, b->y + padding, font_size, RAYWHITE);
}

void render_draw_list(void)
{
    if (!draw_commands) return;
    int count = arrlen(draw_commands);
    for (int i = 0; i < count; i++) {
        DrawCommand *cmd = &draw_commands[i];
        switch (cmd->type) {
            case DC_RECT:   render_rect(&cmd->as.rect); break;
            case DC_TEXT:   render_text(&cmd->as.text); break;
            case DC_BUTTON: render_button(&cmd->as.button); break;
            default: break;
        }
    }
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

    if (border) {
        int text_width = MeasureText(str, font_size);
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

    // Sync draw list from JS
    js_newcfunction(J, sync_draw_list, "syncDrawListToNative", 0);
    js_setglobal(J, "syncDrawListToNative");

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