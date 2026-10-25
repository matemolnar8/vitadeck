Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 06341f44bca0e9643f246ebca9f9f173697c5600` and `git rev-parse --verify 5cbf7dacf365fc2a6999c53b263160e1c53db5d8` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  06341f44bca0e9643f246ebca9f9f173697c5600

head               5cbf7dacf365fc2a6999c53b263160e1c53db5d8

Named refs at pin: origin/refactor/native-util-module ... HEAD

Read the diff:

git diff --find-renames 06341f44bca0e9643f246ebca9f9f173697c5600 5cbf7dacf365fc2a6999c53b263160e1c53db5d8

Commits:
- 5cbf7da perf(runtime): skip native updates when no prop changed by value
- 8b2a2df perf(ui): copy hit and scroll ids into caller buffers
- fe5fce8 perf(core): cache the installed package list
- da0206a perf(core): swap the instance tree only when dirty and sleep when idle

Sources:
- ticket #54 Dirty flag for the swap, cache the package list, use or drop the prepareUpdate payload.
  https://github.com/matemolnar8/vitadeck/issues/54
- ticket #54 comment Replace the per-frame strdup in instance_hit_test with a caller-provided fixed buffer.
  https://github.com/matemolnar8/vitadeck/issues/54

The title:

perf: swap the instance tree only when dirty, cache the package list, drop per-frame allocations

The why:

The JS thread busy-loops a full tree copy and the Shell re-parses manifests every frame ([#54](source:s1)); the hit test also allocates a copy of the hit id every frame ([Máté's note](source:s2)).

The what (medium):

Four independent performance fixes: the JS thread swaps the instance tree only after a mutation and sleeps when idle, the Installed Deck App Library caches its package list, input hit tests copy ids into fixed buffers, and the reconciler skips native updates when no prop changed by value.

## Review concerns

### 01 Dirty flag and idle sleep (`dirty-swap`)

Every `native*` instance call in `instance.c` sets the flag that `instance_tree_swap` now checks, and `js_runtime.c` sleeps through the new `vd_sleep_ms` when no input, timer, fetch or swap ran.

[groups/dirty-swap.md](groups/dirty-swap.md)

### 02 Cached package list and one manifest parse (`package-cache`)

`package_library_list` copies from a list that publish, remove and `package_library_refresh` (called when the Shell opens) invalidate, and `bootstrap.c` hands one parsed manifest to `font_registry_load_package` and `image_registry_load_package`; the new harness checks each invalidation.

[groups/package-cache.md](groups/package-cache.md)

### 03 Caller buffers for hit and scroll ids (`id-buffers`)

`instance_hit_test`, `instance_scroll_at` and `instance_scroll_for_descendant` copy into a `VD_INSTANCE_ID_MAX` buffer under the snapshot lock, `input.c` passes stack buffers, and native create calls reject ids that would not fit.

[groups/id-buffers.md](groups/id-buffers.md)

### 04 prepareUpdate payload is used (`prepare-update`)

`diffProps` sets `native` when a non-handler, non-children prop changed by value and `handlers` when a handler identity changed, and `commitUpdate` runs only the work those flags name.

[groups/prepare-update.md](groups/prepare-update.md)