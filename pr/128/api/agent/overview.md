Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e` and `git rev-parse --verify 9cf3bfdd0d86573d2ee442f914c142a8bfc74f4d` in this repository.
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

base (merge-base)  3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e

head               9cf3bfdd0d86573d2ee442f914c142a8bfc74f4d

Named refs at pin: 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e ... t3/tech-debt-117

Read the diff:

git diff --find-renames 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e 9cf3bfdd0d86573d2ee442f914c142a8bfc74f4d

Commits:
- 9cf3bfd Merge remote-tracking branch 'origin/main' into t3/tech-debt-117
- 33b2ca7 fix(shell): free the upload server mutex on shutdown
- 5a358ab test(shell): drive Shell input headless and clear its message on hide

Sources:
- ticket #117 Extract Shell input into a snapshot-driven function and cover restart, removal confirm/cancel and upload cancel; tighten the re-activation assertion; optionally clear the Shell message on hide.
  https://github.com/matemolnar8/vitadeck/issues/117

The title:

Shell input coverage and a stronger re-activation assertion

The why:

[The Shell reads raylib input directly, so Shell behaviour has no CTest coverage](source:s1), and the re-activation case in `package_library_harness` only asserted something that was already true before the call.

The what (small):

The VitaDeck Shell's input handling moves behind an input snapshot (`shell_apply_system_input`) so a new CTest harness drives restart, removal confirm/cancel and upload cancel headless; the Shell also clears its message on hide, frees the upload server mutex on shutdown, and `package_library_harness` checks re-activation more strictly.

## Review concerns

### 01 Shell input snapshot (`snapshot`)

`shell_poll_system_input` samples the same seven raylib bindings into a `VdShellInput` and hands it to the new `shell_apply_system_input`, which holds the unchanged Shell logic with `input->x` in place of the old `x_pressed()` helpers.

[groups/snapshot.md](groups/snapshot.md)

### 02 Shell message cleared on hide (`message`)

`hide_shell_to_deck` clears `shell->message`, so `activate_package` no longer writes the success message it would immediately wipe.

[groups/message.md](groups/message.md)

### 03 Upload server mutex freed on shutdown (`mutex`)

`shell_shutdown` destroys the upload server mutex after `upload_server_stop` has joined the listener and handler threads.

[groups/mutex.md](groups/mutex.md)

### 04 shell_input_harness (`harness`)

The harness publishes two packages into a temp `VITADECK_DATA_ROOT`, stubs `input_ignore_held_pad_buttons`, and walks the Shell through open/hide, restart, activation, removal and the Upload listener with single-button `VdShellInput` frames; CMake builds and registers it with the real package library and upload server.

Depends on:
- 01 Shell input snapshot (`snapshot`)
- 02 Shell message cleared on hide (`message`)
- 03 Upload server mutex freed on shutdown (`mutex`)

[groups/harness.md](groups/harness.md)

### 05 Re-activation assertion (`reactivation`)

After re-activating `b.vdapp`, the harness also checks `!items[0].is_active` and that `package_library_active_package_name` still returns `b.vdapp`.

[groups/reactivation.md](groups/reactivation.md)