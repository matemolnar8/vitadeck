#include <raylib.h>
#include <mujs.h>
#include <stdbool.h>

/*
    isMouseButtonPressed: (button: number) => boolean
*/
static void is_mouse_button_pressed(js_State *J)
{
    int button = js_tointeger(J, 1);
    bool mousePressed = IsMouseButtonPressed(button);
    js_pushboolean(J, mousePressed);
}

/*
    isMouseButtonDown: (button: number) => boolean
*/
static void is_mouse_button_down(js_State *J)
{
    int button = js_tointeger(J, 1);
    bool down = IsMouseButtonDown(button);
    js_pushboolean(J, down);
}

/*
    isMouseButtonReleased: (button: number) => boolean
*/
static void is_mouse_button_released(js_State *J)
{
    int button = js_tointeger(J, 1);
    bool mouseReleased = IsMouseButtonReleased(button);
    js_pushboolean(J, mouseReleased);
}

/*
    getMouseX: () => number
*/
static void get_mouse_x(js_State *J)
{
    int x = GetMouseX();
    js_pushnumber(J, x);
}

/*
    getMouseY: () => number
*/
static void get_mouse_y(js_State *J)
{
    int y = GetMouseY();
    js_pushnumber(J, y);
}

/*
    getTouchPositions: () => { x: number; y: number }[]
*/
static void get_touch_positions(js_State *J)
{
    js_newarray(J);

    int touchCount = GetTouchPointCount();
    for (int i = 0; i < touchCount; i++) {
        Vector2 position = GetTouchPosition(i);
        js_newobject(J);
        js_pushnumber(J, position.x);
        js_setproperty(J, -2, "x");
        js_pushnumber(J, position.y);
        js_setproperty(J, -2, "y");
        js_setindex(J, -2, i);
    }
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

    js_newcfunction(J, get_touch_positions, "getTouchPositions", 0);
    js_setglobal(J, "getTouchPositions");
}
