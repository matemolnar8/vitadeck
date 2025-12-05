
static void push_color_object(js_State *J, Color c) {
    js_newobject(J);
    js_pushnumber(J, c.r);
    js_setproperty(J, -2, "r");
    js_pushnumber(J, c.g);
    js_setproperty(J, -2, "g");
    js_pushnumber(J, c.b);
    js_setproperty(J, -2, "b");
    js_pushnumber(J, c.a);
    js_setproperty(J, -2, "a");
}

static Color mix_color(Color c, Color mix_with, float amount)
{
    Color out = {0};
    out.r = (unsigned char)(c.r + (mix_with.r - c.r) * amount);
    out.g = (unsigned char)(c.g + (mix_with.g - c.g) * amount);
    out.b = (unsigned char)(c.b + (mix_with.b - c.b) * amount);
    out.a = c.a;
    return out;
}

// =============================================================================
// Tree Rendering
// =============================================================================

typedef struct {
    int x, y;
    int text_index;
} RenderContext;

static void render_instance(ReactInstance *inst, RenderContext ctx);

static void render_rect_instance(ReactInstance *inst, RenderContext ctx)
{
    RectProps *r = &inst->props.rect;
    int abs_x = ctx.x + r->x;
    int abs_y = ctx.y + r->y;
    
    if (r->has_fill) {
        DrawRectangle(abs_x, abs_y, r->width, r->height, r->fill_color);
    }
    if (r->has_outline) {
        DrawRectangleLines(abs_x, abs_y, r->width, r->height, r->border_color);
    }
    
    RenderContext child_ctx = { abs_x, abs_y, 0 };
    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        render_instance(inst->children[i], child_ctx);
        if (inst->children[i]->type == NT_TEXT) {
            child_ctx.text_index++;
        }
    }
}

static void render_text_instance(ReactInstance *inst, RenderContext ctx)
{
    TextProps *t = &inst->props.text;
    
    char text_buffer[1024] = {0};
    int count = arrlen(inst->children);
    for (int i = 0; i < count; i++) {
        ReactInstance *child = inst->children[i];
        if (child->type == NT_RAW_TEXT && child->props.raw_text) {
            strncat(text_buffer, child->props.raw_text, sizeof(text_buffer) - strlen(text_buffer) - 1);
        }
    }
    
    const int padding = 8;
    int font_size = t->font_size > 0 ? t->font_size : 30;
    int text_x = ctx.x + padding;
    int text_y = ctx.y + padding + ctx.text_index * font_size;
    Color color = t->has_color ? t->color : BLACK;
    
    if (t->border) {
        int text_width = MeasureText(text_buffer, font_size);
        int border_padding = 4;
        Rectangle rect = { text_x - border_padding, text_y - border_padding, 
                          text_width + border_padding * 2, font_size + border_padding * 2 };
        DrawRectangleLinesEx(rect, 2, color);
    }
    
    DrawText(text_buffer, text_x, text_y, font_size, color);
}

static void render_button_instance(ReactInstance *inst, RenderContext ctx)
{
    ButtonProps *b = &inst->props.button;
    int abs_x = ctx.x + b->x;
    int abs_y = ctx.y + b->y;
    
    bool hovered = input_is_hovered(inst->id);
    bool pressed = input_is_pressed(inst->id);
    
    Color visual = b->color;
    if (pressed) visual = mix_color(visual, BLACK, 0.5f);
    else if (hovered) visual = mix_color(visual, WHITE, 0.4f);
    
    DrawRectangle(abs_x, abs_y, b->width, b->height, visual);
    
    const int padding = 8;
    int font_size = b->font_size > 0 ? b->font_size : 20;
    DrawText(b->label, abs_x + padding, abs_y + padding, font_size, RAYWHITE);
}

static void render_instance(ReactInstance *inst, RenderContext ctx)
{
    if (!inst) return;
    
    switch (inst->type) {
        case NT_RECT:
            render_rect_instance(inst, ctx);
            break;
        case NT_TEXT:
            render_text_instance(inst, ctx);
            break;
        case NT_BUTTON:
            render_button_instance(inst, ctx);
            break;
        case NT_RAW_TEXT:
            break;
    }
}

void render_draw_list(void)
{
    ReactInstance **root = instance_get_root_children();
    RenderContext ctx = { 0, 0, 0 };
    int count = arrlen(root);
    for (int i = 0; i < count; i++) {
        render_instance(root[i], ctx);
    }
}

void register_js_draw(js_State *J) {
    js_newobject(J);
    push_color_object(J, LIGHTGRAY); js_setproperty(J, -2, "LIGHTGRAY");
    push_color_object(J, GRAY); js_setproperty(J, -2, "GRAY");
    push_color_object(J, DARKGRAY); js_setproperty(J, -2, "DARKGRAY");
    push_color_object(J, YELLOW); js_setproperty(J, -2, "YELLOW");
    push_color_object(J, GOLD); js_setproperty(J, -2, "GOLD");
    push_color_object(J, ORANGE); js_setproperty(J, -2, "ORANGE");
    push_color_object(J, PINK); js_setproperty(J, -2, "PINK");
    push_color_object(J, RED); js_setproperty(J, -2, "RED");
    push_color_object(J, MAROON); js_setproperty(J, -2, "MAROON");
    push_color_object(J, GREEN); js_setproperty(J, -2, "GREEN");
    push_color_object(J, LIME); js_setproperty(J, -2, "LIME");
    push_color_object(J, DARKGREEN); js_setproperty(J, -2, "DARKGREEN");
    push_color_object(J, SKYBLUE); js_setproperty(J, -2, "SKYBLUE");
    push_color_object(J, BLUE); js_setproperty(J, -2, "BLUE");
    push_color_object(J, DARKBLUE); js_setproperty(J, -2, "DARKBLUE");
    push_color_object(J, PURPLE); js_setproperty(J, -2, "PURPLE");
    push_color_object(J, VIOLET); js_setproperty(J, -2, "VIOLET");
    push_color_object(J, DARKPURPLE); js_setproperty(J, -2, "DARKPURPLE");
    push_color_object(J, BEIGE); js_setproperty(J, -2, "BEIGE");
    push_color_object(J, BROWN); js_setproperty(J, -2, "BROWN");
    push_color_object(J, DARKBROWN); js_setproperty(J, -2, "DARKBROWN");
    push_color_object(J, WHITE); js_setproperty(J, -2, "WHITE");
    push_color_object(J, BLACK); js_setproperty(J, -2, "BLACK");
    push_color_object(J, BLANK); js_setproperty(J, -2, "BLANK");
    push_color_object(J, MAGENTA); js_setproperty(J, -2, "MAGENTA");
    push_color_object(J, RAYWHITE); js_setproperty(J, -2, "RAYWHITE");
    js_setglobal(J, "Colors");
}
