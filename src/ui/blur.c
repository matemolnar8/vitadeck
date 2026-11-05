#include "blur.h"

#include <raylib.h>
#include <rlgl.h>
#include <stdbool.h>

// Each level halves the one before, and bilinear filtering averages 2x2 texels at each step, so no shader is needed.
// The last level, 1/16 of the screen, is stretched back over it. Every level is a GXM scene on the Vita, so there is
// no upsampling chain: under the Shell's tint it barely shows.
#define BLUR_LEVELS 5

static RenderTexture2D g_levels[BLUR_LEVELS];
static bool g_loaded;

static void load(void)
{
    for (int i = 0; i < BLUR_LEVELS; i++) {
        g_levels[i] = LoadRenderTexture(GetScreenWidth() >> i, GetScreenHeight() >> i);
        SetTextureFilter(g_levels[i].texture, TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(g_levels[i].texture, TEXTURE_WRAP_CLAMP);
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

void blur_begin(void)
{
    if (!g_loaded) load();
    BeginTextureMode(g_levels[0]);
    ClearBackground(BLACK);
}

void blur_end(void)
{
    EndTextureMode();
    for (int i = 1; i < BLUR_LEVELS; i++) {
        BeginTextureMode(g_levels[i]);
        copy(&g_levels[i - 1], g_levels[i].texture.width, g_levels[i].texture.height);
        EndTextureMode();
    }
    copy(&g_levels[BLUR_LEVELS - 1], GetScreenWidth(), GetScreenHeight());
}

void blur_unload(void)
{
    if (!g_loaded) return;
    for (int i = 0; i < BLUR_LEVELS; i++)
        UnloadRenderTexture(g_levels[i]);
    g_loaded = false;
}
