Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 4d13a180782047fd4564b62704c0469b4e25bcd1` in this repository.
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

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               4d13a180782047fd4564b62704c0469b4e25bcd1

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... 4d13a180782047fd4564b62704c0469b4e25bcd1

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 4d13a180782047fd4564b62704c0469b4e25bcd1

Commits:
- 4d13a18 fix(core): guard package library state with a mutex

Sources:
- ticket #42 Upload thread writes g_active_name/g_active_path via publish while main and JS threads read them; guard with a vd_mutex and hand out copies.
  https://github.com/matemolnar8/vitadeck/issues/42
- pr PR #65 Mutex around active state and directory changes; copy-out accessors; host tier passed in Linux Docker.
  https://github.com/matemolnar8/vitadeck/pull/65

The title:

fix(core): guard package library state with a mutex

The why:

The upload handler thread writes the active name and path while the main thread and the JS thread read them, with no lock in `package_library.c` ([issue #42](source:s1)).

The what (small):

`package_library.c` guards the Active Deck App name and path with one `vd_mutex`, and its name and path accessors now copy into caller buffers instead of returning pointers to globals.

Look for:
- Not done: [issue #42](source:s1) asks for coverage by the package library harness from the test-coverage issue. No harness is added; only the existing CTest and smoke harness ran.

## Review concerns

### 01 Locking inside the package library (`library`)

`g_mutex` guards `g_active_name`/`g_active_path`, and every public `package_library_*` function reads or writes them only under it, using the new unlocked helpers `read_package_info` and `set_active_locked`.

[groups/library.md](groups/library.md)

### 02 Call sites copy into stack buffers (`callers`)

`bootstrap.c` and `jslib.c` copy the active path into a `VD_PATH_MAX` buffer before using it.

Depends on:
- 01 Locking inside the package library (`library`)

[groups/callers.md](groups/callers.md)

### 03 Font and image registries copy the active name (`mirrors-1`)

`fonts.c` and `images.c` get the same two-line change, so read one and skip the other.

Depends on:
- 01 Locking inside the package library (`library`)

[groups/mirrors-1.md](groups/mirrors-1.md)