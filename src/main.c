#include <raylib.h>
#include <stdio.h>
#include "quickjs.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "jslib/jslib.h"
#include "ui/instance_tree.h"
#include "ui/input.h"
#include "platform/thread.h"
#include "core/event_queue.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define FONT_SIZE 30

static volatile bool js_ready = false;
static volatile bool js_init_failed = false;

static int run_function(JSContext *ctx, const char *func_name)
{
	JSValue global = JS_GetGlobalObject(ctx);
	JSValue vitadeck = JS_GetPropertyStr(ctx, global, "vitadeck");
	JSValue func = JS_GetPropertyStr(ctx, vitadeck, func_name);

	JSValue result = JS_Call(ctx, func, vitadeck, 0, NULL);
	
	int ret = 0;
	if (JS_IsException(result)) {
		JSValue exc = JS_GetException(ctx);
		const char *str = JS_ToCString(ctx, exc);
		TraceLog(LOG_ERROR, "%s: an exception occurred in the javascript function", func_name);
		TraceLog(LOG_ERROR, "%s", str ? str : "unknown error");
		JS_FreeCString(ctx, str);
		JS_FreeValue(ctx, exc);
		ret = -1;
	}

	JS_FreeValue(ctx, result);
	JS_FreeValue(ctx, func);
	JS_FreeValue(ctx, vitadeck);
	JS_FreeValue(ctx, global);

	return ret;
}

static int update_container(JSContext *ctx) {
	return run_function(ctx, "updateContainer");
}

#define return_defer(value) do { result = (value); goto defer; } while(0)

static char *read_file(const char *filename, size_t *out_len) {
	FILE *f = fopen(filename, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc(len + 1);
	if (!buf) { fclose(f); return NULL; }
	fread(buf, 1, len, f);
	buf[len] = '\0';
	fclose(f);
	if (out_len) *out_len = len;
	return buf;
}

// JS Thread entry point
static void* js_thread_func(void* arg) {
	(void)arg;
	void* result = NULL;
	JSRuntime *rt = NULL;
	JSContext *ctx = NULL;
	
	rt = JS_NewRuntime();
	if (!rt) {
		TraceLog(LOG_ERROR, "Could not initialize QuickJS runtime.");
		js_init_failed = true;
		return_defer(NULL);
	}

	ctx = JS_NewContext(rt);
	if (!ctx) {
		TraceLog(LOG_ERROR, "Could not initialize QuickJS context.");
		js_init_failed = true;
		return_defer(NULL);
	}
	
	register_js_lib(ctx);

	size_t len;
	char *code = read_file("js/main.js", &len);
	if (!code) {
		TraceLog(LOG_ERROR, "Could not load main.js.");
		js_init_failed = true;
		return_defer(NULL);
	}

	JSValue eval_result = JS_Eval(ctx, code, len, "main.js", JS_EVAL_TYPE_GLOBAL);
	free(code);
	code = NULL;

	if (JS_IsException(eval_result)) {
		JSValue exc = JS_GetException(ctx);
		const char *str = JS_ToCString(ctx, exc);
		TraceLog(LOG_ERROR, "Error evaluating main.js: %s", str ? str : "unknown error");
		JS_FreeCString(ctx, str);
		JS_FreeValue(ctx, exc);
		JS_FreeValue(ctx, eval_result);
		js_init_failed = true;
		return_defer(NULL);
	}
	JS_FreeValue(ctx, eval_result);

	if (update_container(ctx) > 0) {
		js_init_failed = true;
		return_defer(NULL);
	}

	// Initial swap to make content visible
	instance_tree_swap();
	js_ready = true;

	// JS thread main loop
	while (!event_queue_is_shutdown()) {
		process_input_events(ctx);
		run_timeouts(ctx);
		instance_tree_swap();
		vd_thread_yield();
	}

defer:
	if (ctx) JS_FreeContext(ctx);
	if (rt) JS_FreeRuntime(rt);
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
