#include <raylib.h>
#include <stdio.h>
#include <mujs.h>

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define FONT_SIZE 30

int line_count = 0;

void print(js_State *J) {
	const char *str = js_tostring(J, 1);
	
	DrawText(str, 10, 10 + line_count * FONT_SIZE, FONT_SIZE, RED);
	line_count++;

	js_pushundefined(J);
}

static int call_render(js_State *J)
{
	int result;

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

	js_pop(J, 1);

	return 0;
}

int main(int argc, char *argv[]) {
	js_State *J = js_newstate(NULL, NULL, 0);
	if (!J) {
		return 1;
	} 

	js_newcfunction(J, print, "print", 0);
	js_setglobal(J, "print");

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib");	
	SetTargetFPS(60);

	js_dofile(J, "js/main.js");

	while (!WindowShouldClose()) {
		line_count = 0;
		BeginDrawing();
			ClearBackground(WHITE);
			DrawFPS(SCREEN_WIDTH - 100, 10); // top right corner
			call_render(J);
		EndDrawing();
	}

	js_freestate(J);
	CloseWindow();

	return 0;
}
