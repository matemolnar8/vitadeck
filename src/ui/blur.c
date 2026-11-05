#include "blur.h"

#include <raylib.h>
#include <rlgl.h>
#include <stdbool.h>

// Each level halves the one before; bilinear filtering averages 2x2 texels on the way down and spreads them on the
// way back up, so no shader is needed. The last level is 1/16 of the screen.
#define BLUR_LEVELS 5

static RenderTexture2D g_levels[BLUR_LEVELS];
static bool g_loaded;

static void load(void)
{
    for (int i = 0; i < BLUR_LEVELS; i++) {
        g_levels[i] = LoadRenderTexture(GetScreenWidth() >> i, GetScreenHeight() >> i);
        SetTextureFilter(g_levels[i].texture, TEXTURE_FILTER_BILINEAR);
    }
    g_loaded = true;
}

// Copies with blending off: the source's alpha is not coverage, and the result must stay opaque.
static void copy(const RenderTexture2D *from, int to_width, int to_height)
{
    const Texture2D texture = from->texture;
    const Rectangle source = {0, 0, (float)texture.width, -(float)texture.height};
    const Rectangle dest = {0, 0, (float)to_width, (float)to_height};
    rlDrawRenderBatchActive();
    rlDisableColorBlend();
    DrawTexturePro(texture, source, dest, (Vector2){0, 0}, 0, WHITE);
    rlDrawRenderBatchActive();
    rlEnableColorBlend();
}

static void copy_to(const RenderTexture2D *from, const RenderTexture2D *to)
{
    BeginTextureMode(*to);
    copy(from, to->texture.width, to->texture.height);
    EndTextureMode();
}

void blur_begin(void)
{
    if (!g_loaded) load();
    BeginTextureMode(g_levels[0]);
    ClearBackground(BLACK);
}

void blur_end(void)
{
    EndTextureMode();
    for (int i = 1; i < BLUR_LEVELS; i++)
        copy_to(&g_levels[i - 1], &g_levels[i]);
    for (int i = BLUR_LEVELS - 1; i > 1; i--)
        copy_to(&g_levels[i], &g_levels[i - 1]);
    copy(&g_levels[1], GetScreenWidth(), GetScreenHeight());
}

void blur_unload(void)
{
    if (!g_loaded) return;
    for (int i = 0; i < BLUR_LEVELS; i++)
        UnloadRenderTexture(g_levels[i]);
    g_loaded = false;
}
