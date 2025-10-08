#include <raylib.h>
#include <stdio.h>
#include <mujs.h>

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

void print(js_State *J) {
	const char *str = js_tostring(J, 1);
	
	DrawText(str, 10, 10 + line_count * FONT_SIZE, FONT_SIZE, RED);
	line_count++;

	js_pushundefined(J);
}

void debug(js_State *J) {
	int i, top = js_gettop(J);
	for (i = 1; i < top; ++i) {
		const char *s = js_tostring(J, i);
		if (i > 1) putchar(' ');
		fputs(s, stderr);
	}
	putchar('\n');
	js_pushundefined(J);
}

void set_timeout(js_State *J) {
	const int delay = js_tointeger(J, 2);
	
	fprintf(stderr, "set_timeout: %d\n", delay);

	js_pushundefined(J);
}

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
	float last_update = 0;

	while (!WindowShouldClose()) {
		line_count = 0;
		last_update += GetFrameTime();
		if (last_update >= 1.0f) {
			last_update = 0;
			update_container(J);
		}

		BeginDrawing();
			ClearBackground(WHITE);
			DrawFPS(SCREEN_WIDTH - 100, 10); // top right corner
			render(J);
		EndDrawing();
	}

	CloseWindow();
	js_freestate(J);

	return 0;
}
