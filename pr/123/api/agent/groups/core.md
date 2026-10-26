Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 37890a509945b80e7dbaeab3a5a87c58234b5e14` and `git rev-parse --verify a724ceea7f72a91e7420366537b01c70afbb9126` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 37890a509945b80e7dbaeab3a5a87c58234b5e14 a724ceea7f72a91e7420366537b01c70afbb9126 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 01 of 05: Benchmark Mode state machine and clocks (`core`)

Part: Benchmark Mode

The why:

Measurements have to come from the binary that ships, on a device with no scriptable profiler ([request](source:s2)).

The what:

`src/core/bench.c` runs each scenario through boot, setup, warmup, measure and drain using the per-platform `vd_now_us` and `vd_thread_cpu_us` clocks, then writes `result.json`; ADR 0005 records why it lives in the shipping binary.

Look for:
- Subtle: on Vita3K `sceKernelGetThreadInfo` reports 0 `runClocks`, so `cpu` is written as null there and the report shows `-`.
- Race: the UI and JS threads hand off the window through `g_window` and `g_js_ack` with release/acquire atomics; both flags reset at BOOT to SETUP, after the previous JS thread has been joined.

Hunk refs for this concern:
- src/core/bench.c
- src/core/bench.h
- src/platform/thread.h
- src/platform/thread_posix.c
- src/platform/thread_vita.c
- CMakeLists.txt
- docs/adr/0005-benchmark-mode-in-the-shipping-binary.md