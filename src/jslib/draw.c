
static JSValue create_color_object(JSContext *ctx, Color c) {
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "r", JS_NewInt32(ctx, c.r));
    JS_SetPropertyStr(ctx, obj, "g", JS_NewInt32(ctx, c.g));
    JS_SetPropertyStr(ctx, obj, "b", JS_NewInt32(ctx, c.b));
    JS_SetPropertyStr(ctx, obj, "a", JS_NewInt32(ctx, c.a));
    return obj;
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
        ReactInstance *child = inst->children[i];
        if (!child) continue;
        render_instance(child, child_ctx);
        if (child->type == NT_TEXT) {
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
        if (child && child->type == NT_RAW_TEXT && child->props.raw_text) {
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
    instance_tree_render_lock();
    ReactInstance **root = instance_get_root_children();
    RenderContext ctx = { 0, 0, 0 };
    int count = arrlen(root);
    for (int i = 0; i < count; i++) {
        if (root[i]) {
            render_instance(root[i], ctx);
        }
    }
    instance_tree_render_unlock();
}

void register_js_draw(JSContext *ctx) {
    JSValue colors = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, colors, "LIGHTGRAY", create_color_object(ctx, LIGHTGRAY));
    JS_SetPropertyStr(ctx, colors, "GRAY", create_color_object(ctx, GRAY));
    JS_SetPropertyStr(ctx, colors, "DARKGRAY", create_color_object(ctx, DARKGRAY));
    JS_SetPropertyStr(ctx, colors, "YELLOW", create_color_object(ctx, YELLOW));
    JS_SetPropertyStr(ctx, colors, "GOLD", create_color_object(ctx, GOLD));
    JS_SetPropertyStr(ctx, colors, "ORANGE", create_color_object(ctx, ORANGE));
    JS_SetPropertyStr(ctx, colors, "PINK", create_color_object(ctx, PINK));
    JS_SetPropertyStr(ctx, colors, "RED", create_color_object(ctx, RED));
    JS_SetPropertyStr(ctx, colors, "MAROON", create_color_object(ctx, MAROON));
    JS_SetPropertyStr(ctx, colors, "GREEN", create_color_object(ctx, GREEN));
    JS_SetPropertyStr(ctx, colors, "LIME", create_color_object(ctx, LIME));
    JS_SetPropertyStr(ctx, colors, "DARKGREEN", create_color_object(ctx, DARKGREEN));
    JS_SetPropertyStr(ctx, colors, "SKYBLUE", create_color_object(ctx, SKYBLUE));
    JS_SetPropertyStr(ctx, colors, "BLUE", create_color_object(ctx, BLUE));
    JS_SetPropertyStr(ctx, colors, "DARKBLUE", create_color_object(ctx, DARKBLUE));
    JS_SetPropertyStr(ctx, colors, "PURPLE", create_color_object(ctx, PURPLE));
    JS_SetPropertyStr(ctx, colors, "VIOLET", create_color_object(ctx, VIOLET));
    JS_SetPropertyStr(ctx, colors, "DARKPURPLE", create_color_object(ctx, DARKPURPLE));
    JS_SetPropertyStr(ctx, colors, "BEIGE", create_color_object(ctx, BEIGE));
    JS_SetPropertyStr(ctx, colors, "BROWN", create_color_object(ctx, BROWN));
    JS_SetPropertyStr(ctx, colors, "DARKBROWN", create_color_object(ctx, DARKBROWN));
    JS_SetPropertyStr(ctx, colors, "WHITE", create_color_object(ctx, WHITE));
    JS_SetPropertyStr(ctx, colors, "BLACK", create_color_object(ctx, BLACK));
    JS_SetPropertyStr(ctx, colors, "BLANK", create_color_object(ctx, BLANK));
    JS_SetPropertyStr(ctx, colors, "MAGENTA", create_color_object(ctx, MAGENTA));
    JS_SetPropertyStr(ctx, colors, "RAYWHITE", create_color_object(ctx, RAYWHITE));
    
    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "Colors", colors);
    JS_FreeValue(ctx, global);
}
