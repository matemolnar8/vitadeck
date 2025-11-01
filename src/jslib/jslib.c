/*
	This file is the entry point for the JS library for vitadeck.
	It's a single translation unit including all the JS library functions.
	Headers are not included in the js*.c files, they are included here.
*/
#include <stdio.h>
#include <mujs.h>
#include <raylib.h>
#include "stb_ds.h"

#include "jslib.h"

#include "draw.c"
#include "timeout.c"
#include "input.c"
#include "log.c"

static const char *stacktrace_js =
	"Error.prototype.toString = function() {\n"
	"var s = this.name;\n"
	"if ('message' in this) s += ': ' + this.message;\n"
	"if ('stack' in this) s += this.stack;\n"
	"return s;\n"
	"};\n";


static void get_time(js_State *J) {
	js_pushnumber(J, GetTime());
}

void register_js_lib(js_State *J) {
	js_dostring(J, stacktrace_js);

    register_js_log(J);
	register_js_draw(J);
	register_js_timeout(J);
	register_js_input(J);

	js_newcfunction(J, get_time, "getTime", 0);
	js_setglobal(J, "getTime");
}