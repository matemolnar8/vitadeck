#include "sounds.h"

#include <stdio.h>
#include <string.h>

#include "core/util.h"
#include "miniaudio/miniaudio.h"

typedef struct {
    char name[VD_PACKAGE_ASSET_NAME_MAX];
    ma_sound sound;
} VdLoadedSound;

// Opened by the first package that declares a sound, so Deck Apps without sounds never hold the audio device.
static ma_engine g_engine;
static bool g_engine_ready = false;
static VdLoadedSound g_package_sounds[VD_PACKAGE_SOUND_MAX];
static int g_package_sound_count = 0;

static void unload_package_sounds(void)
{
    for (int i = 0; i < g_package_sound_count; i++) {
        ma_sound_uninit(&g_package_sounds[i].sound);
    }
    g_package_sound_count = 0;
}

static ma_sound *find_sound(const char *name)
{
    if (!name) return NULL;
    for (int i = 0; i < g_package_sound_count; i++) {
        if (strcmp(g_package_sounds[i].name, name) == 0) return &g_package_sounds[i].sound;
    }
    return NULL;
}

bool sound_registry_load_package(const char *package_path, const VdPackageManifest *manifest, char *error,
                                 size_t error_size)
{
    unload_package_sounds();
    if (!manifest || manifest->sound_count == 0) return true;

    if (!g_engine_ready) {
        if (ma_engine_init(NULL, &g_engine) != MA_SUCCESS) {
            vd_set_error(error, error_size, "Could not start audio.");
            return false;
        }
        g_engine_ready = true;
    }

    for (int i = 0; i < manifest->sound_count; i++) {
        char sound_path[VD_PATH_MAX];
        VdLoadedSound *loaded = &g_package_sounds[g_package_sound_count];
        // No decode or stream flag: the encoded file stays in memory and decodes as it plays.
        if (!vd_join_path(sound_path, sizeof(sound_path), package_path, manifest->sounds[i].path) ||
            ma_sound_init_from_file(&g_engine, sound_path, MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL,
                                    &loaded->sound) != MA_SUCCESS) {
            vd_set_error(error, error_size, "Could not load sound file.");
            unload_package_sounds();
            return false;
        }
        snprintf(loaded->name, sizeof(loaded->name), "%s", manifest->sounds[i].name);
        g_package_sound_count++;
    }
    return true;
}

void sound_registry_shutdown(void)
{
    unload_package_sounds();
    if (g_engine_ready) {
        ma_engine_uninit(&g_engine);
        g_engine_ready = false;
    }
}

bool sound_registry_play(const char *name, bool loop)
{
    ma_sound *sound = find_sound(name);
    if (!sound) return false;
    ma_sound_set_looping(sound, loop);
    ma_sound_seek_to_pcm_frame(sound, 0);
    ma_sound_start(sound);
    return true;
}

bool sound_registry_stop(const char *name)
{
    ma_sound *sound = find_sound(name);
    if (!sound) return false;
    ma_sound_stop(sound);
    return true;
}
