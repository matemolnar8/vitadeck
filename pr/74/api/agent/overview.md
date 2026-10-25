Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b7df893f29a6ec038ac778734faa2ee18f88c214` and `git rev-parse --verify 06341f44bca0e9643f246ebca9f9f173697c5600` in this repository.
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

base (merge-base)  b7df893f29a6ec038ac778734faa2ee18f88c214

head               06341f44bca0e9643f246ebca9f9f173697c5600

Named refs at pin: b7df893f29a6ec038ac778734faa2ee18f88c214 ... 06341f44bca0e9643f246ebca9f9f173697c5600

Read the diff:

git diff --find-renames b7df893f29a6ec038ac778734faa2ee18f88c214 06341f44bca0e9643f246ebca9f9f173697c5600

Commits:
- 06341f4 refactor: delete dead code
- f0d728a refactor(ui): assert font and image registries fit the manifest limits
- 6fab564 refactor(shell): use the bootstrap screen size constants
- aab37c1 refactor(ui): free_instance_tree reuses instance_back_free
- dbae048 test(core): use the util module in package_manifest_harness
- db8d313 fix(core): reject publish when the .replacing backup path does not fit
- 543a1aa fix(core): fail file reads on fseek, ftell or short fread errors
- 3598302 refactor(core): move duplicated path and file helpers into util module

Sources:
- ticket #55 Move set_error, join_path, has_suffix, is_dir, mkdir_p, read_text_file, remove_tree, safe_package_name into one util module with checked file reads; one screen-size constant; static asserts; delete dead code.
  https://github.com/matemolnar8/vitadeck/issues/55

The title:

Share path and file helpers in a util module

The why:

The helpers were [copied across seven files and the copies already differ](source:s1), and the `read_file` variants [ignore `fseek` / `ftell` failures](source:s1).

The what (medium):

Duplicated C helpers move into `src/core/util.{c,h}` with `vd_` names, the file readers fail on `fseek`/`ftell`/short `fread` errors, and a few small cleanups land beside it: one free path in `instance_tree.c`, one screen-size constant, registry static asserts, and dead code removal.

Look for:
- The ticket lists [`package_library_root`](source:s1) as dead code; it does not exist on `main` at the base, so nothing is deleted for it.
- `path_exists` stays a static in `package_manifest.c` because it has one caller; the ticket does not name it.

## Review concerns

### 01 util module and caller switch (`util`)

`src/core/util.c` defines the `vd_` helpers once, and every former owner, including `package_manifest_harness`, deletes its static copy and calls the shared one.

[groups/util.md](groups/util.md)

### 02 Checked file reads (`reads`)

`vd_read_file` fails on any `fseek`, `ftell`, or short `fread` error, and `jslib.c` and `js_runtime.c` replace their unchecked readers with it, while `archive.c`'s arena reader gets the same `fseek` checks.

Depends on:
- 01 util module and caller switch (`util`)

[groups/reads.md](groups/reads.md)

### 03 Publish rejects an over-long backup path (`replacing`)

`package_library_publish_package` checks the `snprintf` result for `<destination>.replacing` and fails with "Deck App Package path is too long." before touching the disk.

Depends on:
- 01 util module and caller switch (`util`)

[groups/replacing.md](groups/replacing.md)

### 04 One instance free body (`instance-free`)

`free_instance_tree` frees the children recursively and then calls `instance_back_free` for the node itself.

[groups/instance-free.md](groups/instance-free.md)

### 05 One screen-size constant (`screen`)

`shell.c` includes `core/bootstrap.h` and uses `VD_SCREEN_WIDTH`/`VD_SCREEN_HEIGHT` in place of its own copies.

[groups/screen.md](groups/screen.md)

### 06 Registry capacity asserts (`asserts`)

`_Static_assert`s in `fonts.c` and `images.c` fail the build if `VD_FONT_MAX` or `VD_IMAGE_MAX` drops below the manifest limit.

[groups/asserts.md](groups/asserts.md)

### 07 Dead code (`dead`)

`EVT_SHUTDOWN`, `js_runtime_restart`, `VdShell.bind_error`, and the FPS counter and touch circles in `main.c` go away with their only writers.

[groups/dead.md](groups/dead.md)