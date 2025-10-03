#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <raylib.h>
#include <mujs.h>

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define FONT_SIZE 48

int line_count = 0;

void print(js_State *J) {
	const char *str = js_tostring(J, 1);
	
	DrawText(str, 10, 10 + line_count * FONT_SIZE, FONT_SIZE, RED);
	line_count++;

	js_pushundefined(J);
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

	while (!WindowShouldClose()) {
		line_count = 0;
		BeginDrawing();
			ClearBackground(WHITE);
			DrawFPS(SCREEN_WIDTH - 100, 10); // top right corner
			
			js_dostring(J, "print('Hello, world!');");
			js_dostring(J, "print(34 + 35);");
		EndDrawing();
	}

	js_freestate(J);
	CloseWindow();

	return 0;
}
