#include <raylib.h>
#include <stdio.h>
#include <mujs.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "jslib/jslib.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define FONT_SIZE 30

static int run_function(js_State *J, const char *func_name)
{
	js_getglobal(J, "vitadeck");
	js_getproperty(J, -1, func_name);

	// push the this value
	js_pushnull(J);
	
	if (js_pcall(J, 0)) {
		TraceLog(LOG_ERROR, "%s: an exception occurred in the javascript function", func_name);
		TraceLog(LOG_ERROR, "%s", js_tostring(J, -1));
		js_pop(J, 1);
		return -1;
	}

	js_pop(J, 2);

	return 0;
}

static int render(js_State *J)
{
	return run_function(J, "render");
}

static int update_container(js_State *J) {
	return run_function(J, "updateContainer");
}

int main(int argc, char *argv[]) {
	// SetTraceLogLevel(LOG_DEBUG);

	js_State *J = js_newstate(NULL, NULL, 0);
	if (!J) {
		TraceLog(LOG_ERROR, "Could not initialize MuJS state.");
		return 1;
	} 
	
	register_js_lib(J);

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "VitaDeck");	
	SetTargetFPS(60);

	BeginDrawing();
		ClearBackground(BLACK);
		DrawText("Loading...", 10, 10, 20, WHITE);
	EndDrawing();

	if (js_dofile(J, "js/main.js")) {
		TraceLog(LOG_ERROR, "Could not load main.js.");
		return 1;
	}

	if (update_container(J) > 0) {
		return 1;
	}

	while (!WindowShouldClose()) {
		process_mouse_input(J);
		process_touch_input(J);
		run_timeouts(J);

		BeginDrawing();
			ClearBackground(BLACK);
			render(J);
			DrawFPS(SCREEN_WIDTH - 100, 10); // top right corner

			for (int i = 0; i < GetTouchPointCount(); i++) {
				Vector2 position = GetTouchPosition(i);
				DrawCircle(position.x, position.y, 10, RED);
			}
		EndDrawing();
	}

	CloseWindow();
	js_freestate(J);

	return 0;
}
