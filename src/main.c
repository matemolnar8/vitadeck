#include <raylib.h>
#include <stdio.h>
#include <mujs.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "jslib/jslib.h"
#include "platform/thread.h"
#include "core/event_queue.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define FONT_SIZE 30

static volatile bool js_ready = false;
static volatile bool js_init_failed = false;

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

static int update_container(js_State *J) {
	return run_function(J, "updateContainer");
}

#define return_defer(value) do { result = (value); goto defer; } while(0)

// JS Thread entry point
static void* js_thread_func(void* arg) {
	(void)arg;
	void* result = NULL;
	
	js_State *J = js_newstate(NULL, NULL, 0);
	if (!J) {
		TraceLog(LOG_ERROR, "Could not initialize MuJS state.");
		js_init_failed = true;
		return NULL;
	} 
	
	register_js_lib(J);

	if (js_dofile(J, "js/main.js")) {
		TraceLog(LOG_ERROR, "Could not load main.js.");
		js_init_failed = true;
		return_defer(NULL);
	}

	if (update_container(J) > 0) {
		js_init_failed = true;
		return_defer(NULL);
	}

	// Initial swap to make content visible
	instance_tree_swap();
	js_ready = true;

	// JS thread main loop
	while (!event_queue_is_shutdown()) {
		process_input_events(J);
		run_timeouts(J);
		instance_tree_swap();
		vd_thread_yield();
	}

defer:
	js_freestate(J);
	return result;
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	int result = 0;
	vd_thread* js_thread = NULL;
	
	// SetTraceLogLevel(LOG_DEBUG);

	if (!event_queue_init()) {
		TraceLog(LOG_ERROR, "Could not initialize event queue.");
		return 1;
	}
	instance_tree_init();

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "VitaDeck");	
	SetTargetFPS(60);

	BeginDrawing();
		ClearBackground(BLACK);
		DrawText("Loading...", 10, 10, 20, WHITE);
	EndDrawing();

	js_thread = vd_thread_create(js_thread_func, NULL);
	if (!js_thread) {
		TraceLog(LOG_ERROR, "Could not create JS thread.");
		return_defer(1);
	}

	while (!js_ready && !js_init_failed) {
		BeginDrawing();
			ClearBackground(BLACK);
			DrawText("Loading JavaScript...", 10, 10, 20, WHITE);
		EndDrawing();
		vd_thread_yield();
	}

	if (js_init_failed) {
		TraceLog(LOG_ERROR, "JS thread failed to initialize.");
		return_defer(1);
	}

	while (!WindowShouldClose()) {
		poll_mouse_input();
		poll_touch_input();

		BeginDrawing();
			ClearBackground(BLACK);
			render_draw_list();
			DrawFPS(SCREEN_WIDTH - 100, 10);

			for (int i = 0; i < GetTouchPointCount(); i++) {
				Vector2 position = GetTouchPosition(i);
				DrawCircle(position.x, position.y, 10, RED);
			}
		EndDrawing();
	}

defer:
	event_queue_shutdown();
	if (js_thread) {
		vd_thread_join(js_thread);
		vd_thread_destroy(js_thread);
	}
	event_queue_destroy();
	CloseWindow();
	return result;
}
