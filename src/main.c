#include <raylib.h>
#include <stdio.h>
#include <mujs.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "jslib.c"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define FONT_SIZE 30

int line_count = 0;

static const char *stacktrace_js =
	"Error.prototype.toString = function() {\n"
	"var s = this.name;\n"
	"if ('message' in this) s += ': ' + this.message;\n"
	"if ('stack' in this) s += this.stack;\n"
	"return s;\n"
	"};\n"
;

static const char *console_js =
	"var console = { log: debug, debug: debug, warn: debug, error: debug };"
;

static int render(js_State *J)
{
	js_getglobal(J, "vitadeck");
	js_getproperty(J, -1, "render");

	// push the this value
	js_pushnull(J);
	
	if (js_pcall(J, 0)) {
		fprintf(stderr, "an exception occurred in the javascript function\n");
		fprintf(stderr, "%s\n", js_tostring(J, -1));
		js_pop(J, 1);
		return -1;
	}

	js_pop(J, 2);

	return 0;
}

int update_container(js_State *J) {
	js_getglobal(J, "vitadeck");
	js_getproperty(J, -1, "updateContainer");

	js_pushnull(J);

	if (js_pcall(J, 0)) {
		fprintf(stderr, "an exception occurred in the javascript function\n");
		fprintf(stderr, "%s\n", js_tostring(J, -1));
		js_pop(J, 1);
		return -1;
	}

	js_pop(J, 2);

	return 0;
}

int main(int argc, char *argv[]) {
	js_State *J = js_newstate(NULL, NULL, 0);
	if (!J) {
		fprintf(stderr, "Could not initialize MuJS state.\n");
		return 1;
	} 
	
	js_newcfunction(J, debug, "debug", 0);
	js_setglobal(J, "debug");

	js_newcfunction(J, print, "print", 0);
	js_setglobal(J, "print");

	js_newcfunction(J, set_timeout, "setTimeout", 0);
	js_setglobal(J, "setTimeout");

	js_dostring(J, console_js);
	js_dostring(J, stacktrace_js);

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "VitaDeck");	
	SetTargetFPS(60);

	if (js_dofile(J, "js/main.js")) {
		fprintf(stderr, "Could not load main.js.\n");
		return 1;
	}

	update_container(J);

	while (!WindowShouldClose()) {
		BeginDrawing();
			line_count = 0;
			ClearBackground(WHITE);
			DrawFPS(SCREEN_WIDTH - 100, 10); // top right corner
			run_timeout_queue(J);
			render(J);
		EndDrawing();
	}

	CloseWindow();
	js_freestate(J);

	return 0;
}
