#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <raylib.h>
#include <mujs.h>

void print(js_State *J) {
	const char *str = js_tostring(J, 1);
	
	DrawText(str, 10, 10, 20, RED);

	js_pushundefined(J);
}

int main(int argc, char *argv[]) {
	js_State *J = js_newstate(NULL, NULL, 0);
	if (!J) {
		return 1;
	} 

	js_newcfunction(J, print, "print", 0);
	js_setglobal(J, "print");

	InitWindow(960, 544, "Raylib");	
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(WHITE);
		js_dostring(J, "print('Hello, world!');");
		js_dostring(J, "print(34 + 35);");
		EndDrawing();
	}

	js_freestate(J);
	CloseWindow();

	return 0;
}
