#include <raylib.h>
#include <mujs.h>
#include <stdbool.h>

static void is_mouse_button_pressed(js_State *J)
{
    int button = js_tointeger(J, 1);
    bool mousePressed = IsMouseButtonPressed(button);
    js_pushboolean(J, mousePressed);
}

static void is_mouse_button_down(js_State *J)
{
    int button = js_tointeger(J, 1);
    bool down = IsMouseButtonDown(button);
    js_pushboolean(J, down);
}

static void is_mouse_button_released(js_State *J)
{
    int button = js_tointeger(J, 1);
    bool mouseReleased = IsMouseButtonReleased(button);
    js_pushboolean(J, mouseReleased);
}

static void get_mouse_x(js_State *J)
{
    int x = GetMouseX();
    js_pushnumber(J, x);
}

static void get_mouse_y(js_State *J)
{
    int y = GetMouseY();
    js_pushnumber(J, y);
}

void register_js_input(js_State *J)
{
    js_newcfunction(J, is_mouse_button_pressed, "isMouseButtonPressed", 0);
    js_setglobal(J, "isMouseButtonPressed");

    js_newcfunction(J, is_mouse_button_down, "isMouseButtonDown", 0);
    js_setglobal(J, "isMouseButtonDown");

    js_newcfunction(J, is_mouse_button_released, "isMouseButtonReleased", 0);
    js_setglobal(J, "isMouseButtonReleased");

    js_newcfunction(J, get_mouse_x, "getMouseX", 0);
    js_setglobal(J, "getMouseX");

    js_newcfunction(J, get_mouse_y, "getMouseY", 0);
    js_setglobal(J, "getMouseY");
}


