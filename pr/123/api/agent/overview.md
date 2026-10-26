Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 37890a509945b80e7dbaeab3a5a87c58234b5e14` and `git rev-parse --verify a724ceea7f72a91e7420366537b01c70afbb9126` in this repository.
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

base (merge-base)  37890a509945b80e7dbaeab3a5a87c58234b5e14

head               a724ceea7f72a91e7420366537b01c70afbb9126

Named refs at pin: 37890a509945b80e7dbaeab3a5a87c58234b5e14 ... t3/perf-benchmark-suite

Read the diff:

git diff --find-renames 37890a509945b80e7dbaeab3a5a87c58234b5e14 a724ceea7f72a91e7420366537b01c70afbb9126

Commits:
- a724cee fix(bench): update a baseline only from a full, passing run
- 959fabf perf(bench): skip the commit hooks outside Benchmark Mode
- 7be7157 test(bench): record the Vita baseline
- 4202f0d test(bench): record the Vita3K baseline
- d1977c6 perf(runtime): drop the always-on reconciler method timing
- c3dd19a feat(bench): Benchmark Mode for the Vita, Vita3K and host

Sources:
- pr PR #123 Benchmark Mode, runner, device and Vita3K baselines, and removal of always-on reconciler timing.
  https://github.com/matemolnar8/vitadeck/pull/123
- transcript Claude Code session · Oct 26 Set up a benchmark suite for native and JS performance, mostly Vita via emulator and device, with little to no overhead, to establish metrics and a baseline.
- commit perf(runtime): drop the always-on reconciler method timing

The title:

feat(bench): Benchmark Mode for the Vita, Vita3K and host

The why:

Máté wants a performance feedback loop with good metrics and a baseline, mostly for the Vita in the emulator and on the device, with little to no measurement overhead ([request](source:s2)).

The what (large):

Adds Benchmark Mode to the shipping binary: a request file makes the next launch play scripted input against example Deck Apps, time UI frames and the JS loop, write result.json and exit; scripts/bench.sh runs it on the host, in Vita3K or on the Vita and compares against committed baselines. Separately, it deletes the reconciler's always-on method timing.

Look for:
- Perf: [The PR](source:s1) says every hook returns on its first branch when Benchmark Mode is off. Two small costs still run unconditionally: `event_queue_push` stamps `queued_us` and `process_input_events` reads the clock once per event, and `instance_back_mark_dirty` bumps a counter per mutation.
- `scripts/bench.sh vita` uploads any font or emoji the Vita lacks; on a Vita without them the first run spends most of an hour uploading about 4000 emoji over VitaShell FTP.

## Review concerns

### 01 Benchmark Mode state machine and clocks (`core`)

`src/core/bench.c` runs each scenario through boot, setup, warmup, measure and drain using the per-platform `vd_now_us` and `vd_thread_cpu_us` clocks, then writes `result.json`; ADR 0005 records why it lives in the shipping binary.

[groups/core.md](groups/core.md)

### 02 Timing hooks in the UI and JS loops (`hooks`)

`main.c`, `render.c`, `js_runtime.c` and `jslib.c` call the `bench_*` marks around existing work, the reconciler reports commits through `nativeBenchCommit` only when `nativeBenchActive()` was true at load, and `package_library_set_active_transient` lets a scenario switch Deck App without persisting it.

Depends on:
- 01 Benchmark Mode state machine and clocks (`core`)

[groups/hooks.md](groups/hooks.md)

### 03 Runner, report and docs (`tooling`)

`scripts/bench.sh` places the request, launches the target and collects `result.json`, and `js/scripts/bench-report.ts` prints it next to the baseline; the skill and AGENTS.md tell agents when to run it.

Depends on:
- 01 Benchmark Mode state machine and clocks (`core`)

[groups/tooling.md](groups/tooling.md)

### 04 Recorded baselines (`baselines`)

Generated `result.json` and `meta.json` from one Vita3K run on an i9 MacBook Pro and one device run; skim the meta commit and machine, not the numbers.

Depends on:
- 03 Runner, report and docs (`tooling`)

[groups/baselines.md](groups/baselines.md)

### 05 Drop always-on reconciler timing (`metrics-removal`)

`reconciler-metrics.ts` is deleted, the reconciler passes `hostConfig` straight to `Reconciler`, and `main.tsx` stops logging and exporting the metrics; the `hostConfig` hunk also declares the `benchActive` constant the hooks group uses.

[groups/metrics-removal.md](groups/metrics-removal.md)