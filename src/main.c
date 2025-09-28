#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <mujs.h>

#include "debugScreen.h"

#define printf psvDebugScreenPrintf

void print(js_State *J) {
	const char *str = js_tostring(J, 1);
	printf("%s\n", str);
	js_pushundefined(J);
}

int main(int argc, char *argv[]) {
	psvDebugScreenInit();

	js_State *J = js_newstate(NULL, NULL, 0);
	if (!J) {
		printf("Could not initialize MuJS.\n");
		return 1;
	} 

	js_newcfunction(J, print, "print", 0);
	js_setglobal(J, "print");

	js_dostring(J, "print('Hello, world!');");
	js_dostring(J, "print(34 + 35);");
	js_freestate(J);

	sceKernelDelayThread(3*1000000); // Wait for 3 seconds
	return 0;
}
