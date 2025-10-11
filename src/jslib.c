#include <raylib.h>
#include <stdio.h>
#include <mujs.h>

#define FONT_SIZE 30

extern int line_count;

/*
	Log helpers wired to raylib TraceLog. Joins all args with spaces using js_tryrepr.
*/
static void log_with_level(js_State *J, int level)
{
	int top = js_gettop(J);
	char buffer[2048];
	int pos = 0;

	buffer[0] = '\0';

	for (int i = 1; i < top; i++) {
		const char *s;

		if(js_isstring(J, i)) {
			s = js_tostring(J, i);
		} else {
			s = js_torepr(J, i);
		}
		
		if (s == NULL) s = "";

		if (i > 1) {
			if (pos < sizeof(buffer) - 2) {
				buffer[pos++] = ' ';
				buffer[pos] = '\0';
			}
		}

		int written = snprintf(buffer + pos, sizeof(buffer) - pos, "%s", s);
		if (written < 0) {
			break;
		}
		pos += written;
		if (pos >= sizeof(buffer) - 1) {
			pos = sizeof(buffer) - 1;
			break;
		}
	}

	buffer[pos] = '\0';
	TraceLog(level, "%s", buffer);
	js_pushundefined(J);
}

static void logInfo(js_State *J) { log_with_level(J, LOG_INFO); }
static void logDebug(js_State *J) { log_with_level(J, LOG_DEBUG); }
static void logWarn(js_State *J) { log_with_level(J, LOG_WARNING); }
static void logError(js_State *J) { log_with_level(J, LOG_ERROR); }

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
    drawRect(x: number, y: number, width: number, height: number, color?: Color)
*/
void draw_rect(js_State *J) {
    const int x = js_tointeger(J, 1);
    const int y = js_tointeger(J, 2);
    const int w = js_tointeger(J, 3);
    const int h = js_tointeger(J, 4);

    const Color color = parse_color_arg_or_default(J, 5, LIGHTGRAY);

    DrawRectangle(x, y, w, h, color);

    js_pushundefined(J);
}

/*
    drawRectOutline(x: number, y: number, width: number, height: number, color?: Color)
*/
void draw_rect_outline(js_State *J) {
    const int x = js_tointeger(J, 1);
    const int y = js_tointeger(J, 2);
    const int w = js_tointeger(J, 3);
    const int h = js_tointeger(J, 4);

    const Color color = parse_color_arg_or_default(J, 5, DARKGRAY);

    DrawRectangleLines(x, y, w, h, color);

    js_pushundefined(J);
}

/*
    drawText(x: number, y: number, fontSize: number, text: string, color?: Color, border?: boolean)
*/
void draw_text(js_State *J) {
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

void register_js_lib(js_State *J) {
    js_newcfunction(J, logInfo, "logInfo", 0);
	js_setglobal(J, "logInfo");

	js_newcfunction(J, logDebug, "logDebug", 0);
	js_setglobal(J, "logDebug");

	js_newcfunction(J, logWarn, "logWarn", 0);
	js_setglobal(J, "logWarn");

	js_newcfunction(J, logError, "logError", 0);
	js_setglobal(J, "logError");

	js_newcfunction(J, draw_rect, "drawRect", 0);
	js_setglobal(J, "drawRect");

	js_newcfunction(J, draw_rect_outline, "drawRectOutline", 0);
	js_setglobal(J, "drawRectOutline");

	js_newcfunction(J, draw_text, "drawText", 0);
	js_setglobal(J, "drawText");
}