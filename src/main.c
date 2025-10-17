#include <raylib.h>
#include <stdio.h>
#include <mujs.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "jslib.c"
#include "jstimeout.c"
#include "jsdraw.c"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define FONT_SIZE 30

int line_count = 0;

static int render(js_State *J)
{
	js_getglobal(J, "vitadeck");
	js_getproperty(J, -1, "render");

	// push the this value
	js_pushnull(J);
	
	if (js_pcall(J, 0)) {
		TraceLog(LOG_ERROR, "an exception occurred in the javascript function");
		TraceLog(LOG_ERROR, "%s", js_tostring(J, -1));
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
		TraceLog(LOG_ERROR, "an exception occurred in the javascript function");
		TraceLog(LOG_ERROR, "%s", js_tostring(J, -1));
		js_pop(J, 1);
		return -1;
	}

	js_pop(J, 2);

	return 0;
}

int main(int argc, char *argv[]) {
	// SetTraceLogLevel(LOG_DEBUG);

	js_State *J = js_newstate(NULL, NULL, 0);
	if (!J) {
		TraceLog(LOG_ERROR, "Could not initialize MuJS state.");
		return 1;
	} 
	
	register_js_lib(J);
	register_js_timeout(J);
	register_js_draw(J);

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "VitaDeck");	
	SetTargetFPS(60);

	if (js_dofile(J, "js/main.js")) {
		TraceLog(LOG_ERROR, "Could not load main.js.");
		return 1;
	}

	update_container(J);

	while (!WindowShouldClose()) {
		BeginDrawing();
			line_count = 0;
			ClearBackground(WHITE);
			run_timeouts(J);
			render(J);
			DrawFPS(SCREEN_WIDTH - 100, 10); // top right corner
		EndDrawing();
	}

	CloseWindow();
	js_freestate(J);

	return 0;
}
