Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe` in this repository.
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

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe

Commits:
- ad548e1 fix(core): stop stb_ds writing its global hash seed, log dropped events
- e8c85a2 fix(runtime): report unhandled promise rejections to the Failure Screen

Sources:
- ticket #69 Install a rejection tracker; report or log; document the choice; cover with a harness scenario.
  https://github.com/matemolnar8/vitadeck/issues/69
- ticket #78 Seed stb_ds once at startup and ensure nothing re-seeds later.
  https://github.com/matemolnar8/vitadeck/issues/78
- pr PR #81 Fixes #69 and #78; host, sanitized, smoke and Vita builds pass in Docker.
  https://github.com/matemolnar8/vitadeck/pull/81

The title:

Report unhandled rejections, stop stb_ds seed race, log dropped events

The why:

[An unhandled promise rejection is swallowed](source:s1) because the JS thread never installs QuickJS's rejection tracker. [The JS and main threads both write stb_ds's global hash seed](source:s2), a data race.

The what (small):

The JS thread now reports promise rejections that stay unhandled to the Deck App Failure Screen. Separately, the vendored stb_ds no longer writes its global hash seed, and the event queue logs a warning when it drops an event.

Look for:
- [#78 asks to call `stbds_rand_seed` once at startup](source:s2); the change instead removes the LCG step in `stbds_make_hash_index`, because that step re-seeds the global on every new map, so a startup call alone would not remove the write.

## Review concerns

### 01 Rejection tracker on the JS thread (`rejections`)

`track_rejection` in `js_runtime.c` collects unhandled promises, `drain_microtasks` reports the survivors through the new `js_report_rejection`, which shares `report_value` with `js_report_exception`.

[groups/rejections.md](groups/rejections.md)

### 02 Harness scenarios (`rejection-tests`)

`js_limits_harness` runs an unhandled rejection at boot, one in a timer, and one whose `catch` attaches in a later microtask, which must leave the app running until a sentinel `nativeReportDeckAppError` fires.

Depends on:
- 01 Rejection tracker on the JS thread (`rejections`)

[groups/rejection-tests.md](groups/rejection-tests.md)

### 03 stb_ds stops writing its global seed (`seed`)

The vendored `stbds_make_hash_index` copies the global seed into each new table and no longer advances it, so after startup the global is only read.

[groups/seed.md](groups/seed.md)

### 04 Warn on dropped input events (`dropped-events`)

`event_queue_push` logs a `TraceLog` warning after unlocking when the queue is full, and `event_queue_harness` now links raylib and silences the log.

[groups/dropped-events.md](groups/dropped-events.md)