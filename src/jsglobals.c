#include <mujs.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>

static const char *console_js =
	"var console = { log: logInfo, info: logInfo, debug: logDebug, warn: logWarn, error: logError };";

static const char *stacktrace_js =
	"Error.prototype.toString = function() {\n"
	"var s = this.name;\n"
	"if ('message' in this) s += ': ' + this.message;\n"
	"if ('stack' in this) s += this.stack;\n"
	"return s;\n"
	"};\n";

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

/*
    Register all JavaScript global functions and objects
*/
void register_js_globals(js_State *J) {
	js_newcfunction(J, logInfo, "logInfo", 0);
	js_setglobal(J, "logInfo");

	js_newcfunction(J, logDebug, "logDebug", 0);
	js_setglobal(J, "logDebug");

	js_newcfunction(J, logWarn, "logWarn", 0);
	js_setglobal(J, "logWarn");

	js_newcfunction(J, logError, "logError", 0);
	js_setglobal(J, "logError");
	js_newcfunction(J, set_timeout, "setTimeout", 0);
	js_setglobal(J, "setTimeout");

	js_newcfunction(J, draw_rect, "drawRect", 0);
	js_setglobal(J, "drawRect");

	js_newcfunction(J, draw_rect_outline, "drawRectOutline", 0);
	js_setglobal(J, "drawRectOutline");

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

	js_dostring(J, console_js);
	js_dostring(J, stacktrace_js);
}
