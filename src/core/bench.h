#ifndef VD_BENCH_H
#define VD_BENCH_H

// Benchmark Mode: plays scripted scenarios against installed Deck Apps, times the frame loop and the JS thread, writes
// <data root>/bench/result.json and exits. A <data root>/bench/request file turns it on for one launch;
// scripts/bench.sh writes it. Samples stay in memory until a scenario ends, and when Benchmark Mode is off every hook
// returns on its first branch.

#include <stdbool.h>
#include <stdint.h>
#include "quickjs.h"
#include "core/js_runtime.h"

// Consecutive spans of a UI frame; each mark ends the span the previous one started.
typedef enum {
    VD_BENCH_FRAME_INPUT,   // Shell update and input
    VD_BENCH_FRAME_BEGIN,   // BeginDrawing and the clear, where vitaGL may wait for the GPU
    VD_BENCH_FRAME_LAYOUT,  // the render lock, and layout of a new snapshot
    VD_BENCH_FRAME_GLYPHS,  // caching the frame's glyphs and uploading atlas pages
    VD_BENCH_FRAME_DRAW,    // walking the instance tree and issuing draw calls
    VD_BENCH_FRAME_SHELL,   // Shell paint
    VD_BENCH_FRAME_PRESENT, // EndDrawing: buffer swap, vsync and raylib's frame limiter
    VD_BENCH_FRAME_SPAN_COUNT,
} VdBenchFrameSpan;

// Consecutive spans of a JS thread loop iteration.
typedef enum {
    VD_BENCH_JS_INPUT,
    VD_BENCH_JS_TIMERS,
    VD_BENCH_JS_FETCH,
    VD_BENCH_JS_MICROTASKS,
    VD_BENCH_JS_SWAP,
    VD_BENCH_JS_SPAN_COUNT,
} VdBenchJsSpan;

// UI thread. Call after package_library_init and before the Active Deck App starts; makes the first scenario's
// Deck App active when Benchmark Mode is on.
void bench_init(void);
bool bench_active(void);
// UI thread, first thing every frame. Advances the scenario and may ask for a Deck App Runtime Restart. Returns
// false once every scenario ran and the result is written.
bool bench_update(const VdJsRuntime *runtime, bool *request_runtime_restart);
// UI thread, in place of the input polls: applies this frame's scripted pad input.
void bench_apply_input(void);
void bench_frame_mark(VdBenchFrameSpan span);

// JS thread.
void bench_js_thread_start(void);
void bench_js_ready(void);
void bench_js_iteration_begin(void);
void bench_js_mark(VdBenchJsSpan span);
void bench_js_iteration_end(JSRuntime *rt, bool busy, bool swapped);
// After each input event's handler returns. A Deck App that cannot keep up handles a backlog in one iteration, so
// this also opens and closes the window mid-iteration.
void bench_js_input_handled(JSRuntime *rt, uint64_t queued_us, uint64_t started_us);
// Brackets a React commit: prepareForCommit to resetAfterCommit.
void bench_commit(bool ended);

#endif /* VD_BENCH_H */
