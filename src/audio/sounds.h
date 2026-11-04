#ifndef SOUNDS_H
#define SOUNDS_H

#include <stdbool.h>
#include <stddef.h>
#include "core/package_manifest.h"

// Replaces the loaded package sounds with the manifest's; a NULL manifest only unloads them. Call it only while the JS
// thread is stopped: the JS thread reads the loaded sounds without a lock.
bool sound_registry_load_package(const char *package_path, const VdPackageManifest *manifest, char *error,
                                 size_t error_size);
void sound_registry_shutdown(void);
// Both return false when no loaded sound has that name.
bool sound_registry_play(const char *name, bool loop);
bool sound_registry_stop(const char *name);

#endif /* SOUNDS_H */
