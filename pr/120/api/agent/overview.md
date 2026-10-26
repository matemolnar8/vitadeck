Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 02d88881c71ebf1091332268ecbcd587dddafb01` and `git rev-parse --verify 538d2833a7caafed2a5042946bfa378f95553eb0` in this repository.
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

base (merge-base)  02d88881c71ebf1091332268ecbcd587dddafb01

head               538d2833a7caafed2a5042946bfa378f95553eb0

Named refs at pin: 02d88881c71ebf1091332268ecbcd587dddafb01 ... HEAD

Read the diff:

git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 538d2833a7caafed2a5042946bfa378f95553eb0

Commits:
- 538d283 test(smoke): screenshot every demo section instead of the smoke Deck App
- b46abfe fix(jslib): free deleted raw text and keep child links consistent

Sources:
- transcript Claude Code session · Oct 26 Máté asks for golden screenshots of every page of the demo Deck App and for the smoke app to be deleted if no longer needed; later asks to commit the memory leak fix too.
- pr PR #120 Demo goldens replace the smoke app; separate commit fixes a raw text leak and a use-after-free in instance links.
  https://github.com/matemolnar8/vitadeck/pull/120
- commit fix(jslib): free deleted raw text and keep child links consistent

The title:

Golden screenshots of every demo section; fix raw text leak

The why:

The demo Deck App is [more comprehensive than the smoke app](source:s1), so the goldens should cover all of its pages, and the smoke app is no longer needed.

The what (medium):

The smoke harness screenshots all twelve demo sections and the Deck App Failure Screen instead of the single smoke Deck App screen, and the smoke example is deleted. Separately, `src/jslib/instance.c` frees raw text React deletes and keeps parent/child links consistent, a leak the new test surfaced under ASan.

## Review concerns

### 01 Harness steps through the demo (`harness`)

`run_smoke_test.sh` installs the demo with `demo_stubs.js` prepended to `app.js`, and `smoke_harness.c` presses R per `PAGES` entry, then D-pad right twice and Cross to hit "Throw an error", comparing each frame against `tests/fixtures/golden/Linux/`.

[groups/harness.md](groups/harness.md)

### 02 Delete the smoke example (`remove-smoke`)

The `smoke` example is deleted and its DejaVu Sans Mono moves into `demo/assets/fonts`, replacing the demo's symlinks into it.

Depends on:
- 01 Harness steps through the demo (`harness`)

[groups/remove-smoke.md](groups/remove-smoke.md)

### 03 Docs point at the new goldens (`docs`)

ADR 0002, `AGENTS.md` and the `vitadeck-smoke` skill now name `tests/fixtures/golden/Linux/` and the demo instead of `smoke_golden.Linux.png` and the smoke app; wording only.

Depends on:
- 01 Harness steps through the demo (`harness`)

[groups/docs.md](groups/docs.md)

### 04 Free deleted raw text; consistent child links (`leak`)

`unlink_child` runs before every append, insert, remove and destroy, so `native_destroy_instance` can safely free its raw text children and clear its other children's `parent`.

[groups/leak.md](groups/leak.md)