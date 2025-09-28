#include <psp2/kernel/processmgr.h>
#include <stdlib.h>
#include <mujs.h>
#include <raylib.h>

#define CONSOLE_MAX_LINES 256
#define CONSOLE_FONT_SIZE 18
#define CONSOLE_LINE_SPACING 4

static struct {
	const char *lines[CONSOLE_MAX_LINES];
	int count;
} g_console = {0};

static void console_append_line(const char *text) {
	if (!text) return;
	// Keep a bounded list of lines; drop oldest when full
    if (g_console.count >= CONSOLE_MAX_LINES) {
        // free oldest and shift
        for (int i = 1; i < g_console.count; ++i) g_console.lines[i-1] = g_console.lines[i];
        g_console.count = CONSOLE_MAX_LINES - 1;
    }
	// Duplicate string for storage
	int len = 0; while (text[len] != '\0') len++;
	char *copy = (char*)malloc((size_t)len + 1);
	if (!copy) return;
	for (int i = 0; i <= len; ++i) copy[i] = text[i];
	g_console.lines[g_console.count++] = copy;
}

static void console_draw(void) {
	int x = 8;
	int y = 8;
	for (int i = 0; i < g_console.count; ++i) {
		DrawText(g_console.lines[i], x, y, CONSOLE_FONT_SIZE, RAYWHITE);
		y += CONSOLE_FONT_SIZE + CONSOLE_LINE_SPACING;
	}
}

// JS: print(...args)
static void js_print(js_State *J) {
	int top = js_gettop(J);
	// join args with spaces
	// estimate length first
	int total = 0;
	for (int i = 1; i < top; ++i) {
		const char *s = js_tostring(J, i);
		for (const char *p = s; *p; ++p) total++;
		if (i + 1 < top) total++; // space
	}
	char *buf = (char*)malloc((size_t)total + 1);
	if (!buf) {
		js_pushundefined(J);
		return;
	}
	int k = 0;
	for (int i = 1; i < top; ++i) {
		const char *s = js_tostring(J, i);
		for (const char *p = s; *p; ++p) buf[k++] = *p;
		if (i + 1 < top) buf[k++] = ' ';
	}
	buf[k] = '\0';
	console_append_line(buf);
	// ownership transferred; do not free buf itself because console stores copy
	free(buf);
	js_pushundefined(J);
}

// Minimal Raylib bindings for JS main loop
static void js_beginDrawing(js_State *J) { BeginDrawing(); js_pushundefined(J); }
static void js_endDrawing(js_State *J) { EndDrawing(); js_pushundefined(J); }
static void js_clearBackground(js_State *J) {
	int r = js_tryinteger(J, 1, 0);
	int g = js_tryinteger(J, 2, 0);
	int b = js_tryinteger(J, 3, 0);
	int a = js_tryinteger(J, 4, 255);
	ClearBackground((Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a });
	js_pushundefined(J);
}
static void js_drawConsole(js_State *J) { console_draw(); js_pushundefined(J); }
static void js_shouldClose(js_State *J) { js_pushboolean(J, WindowShouldClose()); }

int main(int argc, char *argv[]) {
	const int screenWidth = 960;
	const int screenHeight = 544;

	InitWindow(screenWidth, screenHeight, "VitaDeck");
	SetTargetFPS(60);

	js_State *J = js_newstate(NULL, NULL, 0);
	if (!J) {
		CloseWindow();
		return 1;
	}

	// Register functions
	js_newcfunction(J, js_print, "print", 0); js_setglobal(J, "print");
	js_newcfunction(J, js_beginDrawing, "beginDrawing", 0); js_setglobal(J, "beginDrawing");
	js_newcfunction(J, js_endDrawing, "endDrawing", 0); js_setglobal(J, "endDrawing");
	js_newcfunction(J, js_clearBackground, "clearBackground", 4); js_setglobal(J, "clearBackground");
	js_newcfunction(J, js_drawConsole, "drawConsole", 0); js_setglobal(J, "drawConsole");
	js_newcfunction(J, js_shouldClose, "shouldClose", 0); js_setglobal(J, "shouldClose");

	// Provide a default JS mainLoop that renders the console
	js_dostring(J,
		"if (typeof mainLoop !== 'function') {\n"
		"  function mainLoop() {\n"
		"    if (shouldClose()) return false;\n"
		"    beginDrawing();\n"
		"    clearBackground(0,0,0,255);\n"
		"    drawConsole();\n"
		"    endDrawing();\n"
		"    return true;\n"
		"  }\n"
		"}\n"
	);

	// Demo lines
	js_dostring(J, "print('Hello, world!')");
	js_dostring(J, "print(34 + 35)");

	for (;;) {
		if (WindowShouldClose()) break;
		js_getglobal(J, "mainLoop");
		if (!js_iscallable(J, -1)) {
			BeginDrawing();
			ClearBackground(BLACK);
			console_draw();
			EndDrawing();
		} else {
			js_pushundefined(J);
			if (js_pcall(J, 0)) {
				const char *err = js_trystring(J, -1, "Error");
				console_append_line(err);
				js_pop(J, 1);
			} else {
				int cont = 1;
				if (js_isdefined(J, -1)) cont = js_toboolean(J, -1);
				js_pop(J, 1);
				if (!cont) break;
			}
		}
	}

	js_freestate(J);
	CloseWindow();
	return 0;
}
