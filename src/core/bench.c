#include "bench.h"

#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/package_library.h"
#include "core/util.h"
#include "platform/thread.h"
#include "ui/input.h"
#include "ui/instance_tree.h"

#define WARMUP_S 3
#define MEASURE_S 10
#define WARMUP_US (WARMUP_S * 1000000ull)
#define MEASURE_US (MEASURE_S * 1000000ull)
#define BOOT_TIMEOUT_US (30 * 1000000ull)
#define DRAIN_TIMEOUT_US (10 * 1000000ull)
// A frame longer than one and a half vsync intervals of the Vita's 60 Hz display missed one.
#define DROPPED_FRAME_US 25000
// Scripts advance one step every FRAMES_PER_STEP frames, ten steps a second at 60 FPS.
#define FRAMES_PER_STEP 6
#define FRAME_CAPACITY (MEASURE_S * 240)
#define JS_ITERATION_CAPACITY (MEASURE_S * 2000)
#define COMMIT_CAPACITY (MEASURE_S * 240)
#define INPUT_CAPACITY (MEASURE_S * 240)
#define SCRATCH_CAPACITY JS_ITERATION_CAPACITY
#define MAX_RUNS 32
#define SCENARIO_NAME_MAX 64

// A script is one character per step:
//   ^ v < >   D-pad press          x          Cross tap
//   o t s     Circle, Triangle, Square tap   l r   L, R tap
//   + -       left stick held down / up      .     nothing
typedef struct {
    const char *name;
    const char *package;
    // Played once after the Deck App boots, e.g. to reach a demo section.
    const char *setup;
    // Played over and over through warmup and measurement.
    const char *loop;
} BenchScenario;

static const BenchScenario SCENARIOS[] = {
    // 145 Buttons and a header that never change: the steady cost of a dense screen.
    {"minesweeper-idle", "minesweeper.vdapp", NULL, NULL},
    // Focus walks the border of the 16x9 grid; every move re-renders the board through onFocus.
    {"minesweeper-navigate", "minesweeper.vdapp", NULL, ">>>>>>>>>>>>>>>vvvvvvvv<<<<<<<<<<<<<<<^^^^^^^^"},
    // Wrapped Latin, CJK and emoji text.
    {"demo-text", "vitadeck-demo.vdapp", "r", NULL},
    // Focus moves from the section list into the Scroll section's list, which then glides down and up under the
    // left stick: native scrolling with no React work.
    {"demo-scroll", "vitadeck-demo.vdapp", "rrrrrrv>", "++++++++++----------"},
};
#define SCENARIO_COUNT ((int)(sizeof(SCENARIOS) / sizeof(SCENARIOS[0])))

typedef enum { STATE_START, STATE_BOOT, STATE_SETUP, STATE_WARMUP, STATE_MEASURE, STATE_DRAIN, STATE_DONE } State;

// The measurement window, published by the UI thread to the JS thread.
enum { WINDOW_IDLE, WINDOW_MEASURE, WINDOW_DONE };

#define FRAME_COLUMNS (VD_BENCH_FRAME_SPAN_COUNT + 1)
#define FRAME_INTERVAL VD_BENCH_FRAME_SPAN_COUNT

static bool g_active;
static int g_window;
// Set by the JS thread once its samples for the window are final.
static int g_js_ack;
// Loop iterations in which the JS thread found nothing to do.
static unsigned g_js_idle_iterations;

// UI thread only.
static struct {
    char runs[MAX_RUNS][SCENARIO_NAME_MAX];
    int run_count;
    int run;
    const BenchScenario *scenario;
    State state;
    uint64_t state_start_us;

    const char *script;
    bool script_loops;
    int script_pos;
    int script_frame;
    unsigned setup_idle_mark;
    GamepadSample sample;

    uint64_t frame_start_us;
    uint64_t last_mark_us;
    uint32_t spans[VD_BENCH_FRAME_SPAN_COUNT];
    uint32_t (*frames)[FRAME_COLUMNS];
    int frame_count;
    int frame_overflow;
    uint64_t measure_start_us;
    uint64_t measured_us;
    uint64_t cpu_start_us;
    uint64_t cpu_us;
    // Vita3K does not implement the Vita's per-thread CPU time and reports 0.
    bool cpu_known;

    uint32_t *scratch;
    FILE *out;
    int written;
} ui;

// JS thread only, until it sets g_js_ack.
static struct {
    bool recording;
    uint64_t thread_start_us;
    uint64_t startup_us;
    uint64_t last_mark_us;
    uint32_t spans[VD_BENCH_JS_SPAN_COUNT];
    // Every span's time in the window, including iterations the window cuts through.
    uint64_t span_totals[VD_BENCH_JS_SPAN_COUNT];
    uint32_t (*iterations)[VD_BENCH_JS_SPAN_COUNT];
    int iteration_count;
    int iteration_overflow;
    uint32_t *swaps;
    int swap_count;
    uint32_t *commits;
    int commit_count;
    uint64_t commit_start_us;
    uint32_t *inputs;
    uint32_t *handlers;
    int input_count;
    uint32_t mutations_start;
    uint32_t mutations;
    uint64_t cpu_start_us;
    uint64_t cpu_us;
    int64_t heap_used;
    int64_t heap_malloc;
} js;

static const char *FRAME_SPAN_NAMES[VD_BENCH_FRAME_SPAN_COUNT] = {"input", "begin", "layout", "glyphs",
                                                                  "draw",  "shell", "present"};
static const char *JS_SPAN_NAMES[VD_BENCH_JS_SPAN_COUNT] = {"input", "timers", "fetch", "microtasks", "swap"};

static bool bench_path(char *out, size_t out_size, const char *name)
{
    char dir[VD_PATH_MAX];
    return vd_join_path(dir, sizeof(dir), package_library_data_root(), "bench") &&
           vd_join_path(out, out_size, dir, name);
}

static const BenchScenario *find_scenario(const char *name)
{
    for (int i = 0; i < SCENARIO_COUNT; i++) {
        if (strcmp(SCENARIOS[i].name, name) == 0) return &SCENARIOS[i];
    }
    return NULL;
}

static void parse_request(const char *request)
{
    const char *at = request;
    while (*at && ui.run_count < MAX_RUNS) {
        size_t skip = strspn(at, " \t\r\n");
        at += skip;
        size_t len = strcspn(at, " \t\r\n");
        if (len == 0) break;
        if (len < SCENARIO_NAME_MAX) {
            memcpy(ui.runs[ui.run_count], at, len);
            ui.runs[ui.run_count][len] = '\0';
            ui.run_count++;
        }
        at += len;
    }
    if (ui.run_count > 0) return;
    for (int i = 0; i < SCENARIO_COUNT && i < MAX_RUNS; i++) {
        snprintf(ui.runs[i], SCENARIO_NAME_MAX, "%s", SCENARIOS[i].name);
        ui.run_count++;
    }
}

static double measure_clock_overhead_ns(void)
{
    const int reads = 10000;
    volatile uint64_t sink = 0;
    uint64_t start = vd_now_us();
    for (int i = 0; i < reads; i++)
        sink += vd_now_us();
    (void)sink;
    return (double)(vd_now_us() - start) * 1000.0 / reads;
}

static void write_json_string(const char *value)
{
    fputc('"', ui.out);
    for (const unsigned char *c = (const unsigned char *)value; *c; c++) {
        if (*c == '"' || *c == '\\') {
            fprintf(ui.out, "\\%c", *c);
        } else if (*c < 0x20) {
            fprintf(ui.out, "\\u%04x", *c);
        } else {
            fputc(*c, ui.out);
        }
    }
    fputc('"', ui.out);
}

void bench_init(void)
{
    char request_path[VD_PATH_MAX];
    char stale_path[VD_PATH_MAX];
    char result_path[VD_PATH_MAX];
    if (!bench_path(request_path, sizeof(request_path), "request") ||
        !bench_path(stale_path, sizeof(stale_path), "result.json") ||
        !bench_path(result_path, sizeof(result_path), "result.json.tmp")) {
        return;
    }
    char *request = vd_read_file(request_path, NULL);
    if (!request) return;
    // Once the request is gone a runner can wait for result.json without mistaking the last one for it.
    remove(stale_path);
    remove(request_path);
    parse_request(request);
    free(request);

    ui.frames = malloc(sizeof(*ui.frames) * FRAME_CAPACITY);
    ui.scratch = malloc(sizeof(*ui.scratch) * SCRATCH_CAPACITY);
    js.iterations = malloc(sizeof(*js.iterations) * JS_ITERATION_CAPACITY);
    js.swaps = malloc(sizeof(*js.swaps) * JS_ITERATION_CAPACITY);
    js.commits = malloc(sizeof(*js.commits) * COMMIT_CAPACITY);
    js.inputs = malloc(sizeof(*js.inputs) * INPUT_CAPACITY);
    js.handlers = malloc(sizeof(*js.handlers) * INPUT_CAPACITY);
    ui.out = fopen(result_path, "w");
    if (!ui.frames || !ui.scratch || !js.iterations || !js.swaps || !js.commits || !js.inputs || !js.handlers ||
        !ui.out) {
        TraceLog(LOG_ERROR, "Benchmark Mode: could not allocate samples or open %s", result_path);
        if (ui.out) fclose(ui.out);
        free(ui.frames);
        free(ui.scratch);
        free(js.iterations);
        free(js.swaps);
        free(js.commits);
        free(js.inputs);
        free(js.handlers);
        memset(&ui, 0, sizeof(ui));
        memset(&js, 0, sizeof(js));
        return;
    }

    fprintf(ui.out, "{\n  \"schema\": 1,\n  \"platform\": \"%s\",\n  \"clock_overhead_ns\": %.1f,\n",
#ifdef __vita__
            "vita",
#else
            "host",
#endif
            measure_clock_overhead_ns());
    fprintf(ui.out, "  \"warmup_s\": %d,\n  \"measure_s\": %d,\n  \"scenarios\": [", WARMUP_S, MEASURE_S);

    const BenchScenario *first = find_scenario(ui.runs[0]);
    if (first) package_library_set_active_transient(first->package, NULL, 0);
    g_active = true;
    TraceLog(LOG_INFO, "Benchmark Mode: %d scenario(s)", ui.run_count);
}

bool bench_active(void)
{
    return g_active;
}

static void script_start(const char *script, bool loops)
{
    ui.script = script;
    ui.script_loops = loops;
    ui.script_pos = 0;
    ui.script_frame = 0;
}

// Fills this frame's sample from the script. Returns false once a one-shot script has played.
static bool script_play(GamepadSample *sample)
{
    if (!ui.script || ui.script[0] == '\0') return false;
    if (ui.script[ui.script_pos] == '\0') {
        if (!ui.script_loops) return false;
        ui.script_pos = 0;
    }
    const bool first = ui.script_frame == 0;
    switch (ui.script[ui.script_pos]) {
    case '^':
        sample->up = first;
        break;
    case 'v':
        sample->down = first;
        break;
    case '<':
        sample->left = first;
        break;
    case '>':
        sample->right = first;
        break;
    case 'x':
        sample->confirm = first;
        break;
    case 'o':
        sample->pad_buttons[PAD_CIRCLE] = first;
        break;
    case 't':
        sample->pad_buttons[PAD_TRIANGLE] = first;
        break;
    case 's':
        sample->pad_buttons[PAD_SQUARE] = first;
        break;
    case 'l':
        sample->pad_buttons[PAD_L] = first;
        break;
    case 'r':
        sample->pad_buttons[PAD_R] = first;
        break;
    case '+':
        sample->stick_y = 1.0f;
        break;
    case '-':
        sample->stick_y = -1.0f;
        break;
    default:
        break;
    }
    if (++ui.script_frame == FRAMES_PER_STEP) {
        ui.script_frame = 0;
        ui.script_pos++;
    }
    return true;
}

static void enter(State state, uint64_t now)
{
    ui.state = state;
    ui.state_start_us = now;
}

static void begin_scenario_entry(const char *name)
{
    fprintf(ui.out, "%s\n    {\n      \"name\": ", ui.written++ > 0 ? "," : "");
    write_json_string(name);
}

static void write_error(const char *name, const char *error)
{
    TraceLog(LOG_WARNING, "Benchmark Mode: %s: %s", name, error);
    begin_scenario_entry(name);
    fprintf(ui.out, ",\n      \"error\": ");
    write_json_string(error);
    fprintf(ui.out, "\n    }");
}

static int compare_u32(const void *a, const void *b)
{
    uint32_t x = *(const uint32_t *)a;
    uint32_t y = *(const uint32_t *)b;
    return (x > y) - (x < y);
}

// Writes "key": {mean, p50, p95, p99, max} in milliseconds, and sorts values.
static void write_stats(const char *indent, const char *key, uint32_t *values, int count, bool last)
{
    fprintf(ui.out, "%s\"%s\": ", indent, key);
    if (count == 0) {
        fprintf(ui.out, "null%s\n", last ? "" : ",");
        return;
    }
    qsort(values, (size_t)count, sizeof(*values), compare_u32);
    uint64_t sum = 0;
    for (int i = 0; i < count; i++)
        sum += values[i];
#define PERCENTILE(p) (values[(int)((count - 1) * (p) + 0.5)] / 1000.0)
    fprintf(ui.out, "{\"mean\": %.3f, \"p50\": %.3f, \"p95\": %.3f, \"p99\": %.3f, \"max\": %.3f}%s\n",
            (double)sum / count / 1000.0, PERCENTILE(0.50), PERCENTILE(0.95), PERCENTILE(0.99),
            values[count - 1] / 1000.0, last ? "" : ",");
#undef PERCENTILE
}

static int frame_column(int column)
{
    for (int i = 0; i < ui.frame_count; i++)
        ui.scratch[i] = ui.frames[i][column];
    return ui.frame_count;
}

// The CPU work of a frame: every span but the GPU and vsync waits in BeginDrawing and EndDrawing.
static int frame_work(void)
{
    for (int i = 0; i < ui.frame_count; i++) {
        uint32_t work = 0;
        for (int span = 0; span < VD_BENCH_FRAME_SPAN_COUNT; span++) {
            if (span != VD_BENCH_FRAME_BEGIN && span != VD_BENCH_FRAME_PRESENT) work += ui.frames[i][span];
        }
        ui.scratch[i] = work;
    }
    return ui.frame_count;
}

static int js_iteration_totals(void)
{
    for (int i = 0; i < js.iteration_count; i++) {
        uint32_t total = 0;
        for (int span = 0; span < VD_BENCH_JS_SPAN_COUNT; span++)
            total += js.iterations[i][span];
        ui.scratch[i] = total;
    }
    return js.iteration_count;
}

static int instance_count(void)
{
    int count = 0;
    for (NodeType type = NT_RECT; type <= NT_STACK; type++)
        count += instance_tree_count_nodes(type);
    return count;
}

static void write_scenario(bool js_done)
{
    const double seconds = ui.measured_us / 1e6;
    int dropped = 0;
    for (int i = 0; i < ui.frame_count; i++)
        dropped += ui.frames[i][FRAME_INTERVAL] > DROPPED_FRAME_US;

    begin_scenario_entry(ui.scenario->name);
    fprintf(ui.out, ",\n      \"package\": ");
    write_json_string(ui.scenario->package);
    fprintf(ui.out, ",\n      \"measured_s\": %.3f,\n      \"instances\": %d,\n", seconds, instance_count());
    if (ui.cpu_known) {
        fprintf(ui.out, "      \"cpu\": {\"ui_pct\": %.1f, \"js_pct\": ", ui.cpu_us * 100.0 / ui.measured_us);
        if (js_done) {
            fprintf(ui.out, "%.1f},\n", js.cpu_us * 100.0 / ui.measured_us);
        } else {
            fprintf(ui.out, "null},\n");
        }
    } else {
        fprintf(ui.out, "      \"cpu\": null,\n");
    }

    fprintf(ui.out, "      \"frames\": {\n");
    fprintf(ui.out, "        \"count\": %d,\n        \"fps\": %.2f,\n        \"dropped\": %d,\n", ui.frame_count,
            ui.frame_count / seconds, dropped);
    fprintf(ui.out, "        \"overflow\": %d,\n", ui.frame_overflow);
    write_stats("        ", "interval", ui.scratch, frame_column(FRAME_INTERVAL), false);
    write_stats("        ", "work", ui.scratch, frame_work(), false);
    for (int span = 0; span < VD_BENCH_FRAME_SPAN_COUNT; span++) {
        write_stats("        ", FRAME_SPAN_NAMES[span], ui.scratch, frame_column(span),
                    span == VD_BENCH_FRAME_SPAN_COUNT - 1);
    }
    fprintf(ui.out, "      },\n");

    fprintf(ui.out, "      \"js\": ");
    if (!js_done) {
        fprintf(ui.out, "null\n    }");
        return;
    }
    fprintf(ui.out, "{\n        \"startup_ms\": %.3f,\n", js.startup_us / 1000.0);
    fprintf(ui.out, "        \"heap_used_kb\": %lld,\n        \"heap_malloc_kb\": %lld,\n",
            (long long)(js.heap_used / 1024), (long long)(js.heap_malloc / 1024));
    fprintf(ui.out, "        \"busy_iterations\": %d,\n        \"overflow\": %d,\n", js.iteration_count,
            js.iteration_overflow);
    fprintf(ui.out, "        \"ms_per_s\": {");
    uint64_t busy_total = 0;
    for (int span = 0; span < VD_BENCH_JS_SPAN_COUNT; span++) {
        busy_total += js.span_totals[span];
        fprintf(ui.out, "\"%s\": %.3f, ", JS_SPAN_NAMES[span], js.span_totals[span] / 1000.0 / seconds);
    }
    fprintf(ui.out, "\"busy\": %.3f},\n", busy_total / 1000.0 / seconds);
    write_stats("        ", "busy_iteration", ui.scratch, js_iteration_totals(), false);
    fprintf(ui.out, "        \"inputs\": %d,\n", js.input_count);
    const int inputs = js.input_count < INPUT_CAPACITY ? js.input_count : INPUT_CAPACITY;
    write_stats("        ", "input_latency", js.inputs, inputs, false);
    write_stats("        ", "input_handler", js.handlers, inputs, false);
    fprintf(ui.out, "        \"commits\": %d,\n", js.commit_count);
    fprintf(ui.out, "        \"mutations\": %u,\n", js.mutations);
    write_stats("        ", "commit", js.commits, js.commit_count < COMMIT_CAPACITY ? js.commit_count : COMMIT_CAPACITY,
                false);
    fprintf(ui.out, "        \"swaps\": %d,\n", js.swap_count);
    write_stats("        ", "swap", js.swaps, js.swap_count, true);
    fprintf(ui.out, "      }\n    }");

    TraceLog(LOG_INFO, "Benchmark Mode: %s: %.1f fps, %d commits, %d inputs", ui.scenario->name,
             ui.frame_count / seconds, js.commit_count, js.input_count);
}

static void finish(void)
{
    fprintf(ui.out, "\n  ]\n}\n");
    fclose(ui.out);
    ui.out = NULL;
    char tmp_path[VD_PATH_MAX];
    char result_path[VD_PATH_MAX];
    bench_path(tmp_path, sizeof(tmp_path), "result.json.tmp");
    bench_path(result_path, sizeof(result_path), "result.json");
    if (rename(tmp_path, result_path) != 0) {
        TraceLog(LOG_ERROR, "Benchmark Mode: could not write %s", result_path);
    } else {
        TraceLog(LOG_INFO, "Benchmark Mode: wrote %s", result_path);
    }
    ui.state = STATE_DONE;
}

static void next_scenario(uint64_t now)
{
    ui.run++;
    enter(STATE_START, now);
    if (ui.run == ui.run_count) finish();
}

static void fail_scenario(const char *error, uint64_t now)
{
    write_error(ui.runs[ui.run], error);
    next_scenario(now);
}

// Records the frame that ends now if all of it fell in the measurement window.
static void finish_frame(uint64_t now)
{
    const uint32_t interval = ui.frame_start_us ? (uint32_t)(now - ui.frame_start_us) : 0;
    if (ui.state == STATE_MEASURE && ui.frame_start_us >= ui.measure_start_us) {
        if (ui.frame_count < FRAME_CAPACITY) {
            memcpy(ui.frames[ui.frame_count], ui.spans, sizeof(ui.spans));
            ui.frames[ui.frame_count][FRAME_INTERVAL] = interval;
            ui.frame_count++;
        } else {
            ui.frame_overflow++;
        }
    }
    memset(ui.spans, 0, sizeof(ui.spans));
    ui.frame_start_us = now;
    ui.last_mark_us = now;
    ui.sample = (GamepadSample){.dt = interval > 0 && interval < 100000 ? interval / 1e6f : 1.0f / 60.0f};
}

bool bench_update(const VdJsRuntime *runtime, bool *request_runtime_restart)
{
    if (!g_active) return true;
    const uint64_t now = vd_now_us();
    finish_frame(now);

    if (ui.state != STATE_START && ui.state != STATE_DONE && js_runtime_failed(runtime)) {
        char error[VD_FAILURE_MESSAGE_MAX + 32];
        snprintf(error, sizeof(error), "Deck App failed: %s", runtime->failure_message);
        fail_scenario(error, now);
    }

    switch (ui.state) {
    case STATE_START: {
        ui.scenario = find_scenario(ui.runs[ui.run]);
        char error[256] = "";
        if (!ui.scenario) {
            fail_scenario("unknown scenario", now);
        } else if (!package_library_set_active_transient(ui.scenario->package, error, sizeof(error))) {
            fail_scenario(error, now);
        } else {
            __atomic_store_n(&g_window, WINDOW_IDLE, __ATOMIC_RELEASE);
            __atomic_store_n(&g_js_ack, 0, __ATOMIC_RELEASE);
            *request_runtime_restart = true;
            enter(STATE_BOOT, now);
        }
        break;
    }
    case STATE_BOOT:
        if (js_runtime_is_ready(runtime)) {
            // The previous JS thread is joined by now, so a late ack from it cannot land after this.
            __atomic_store_n(&g_window, WINDOW_IDLE, __ATOMIC_RELEASE);
            __atomic_store_n(&g_js_ack, 0, __ATOMIC_RELEASE);
            ui.setup_idle_mark = __atomic_load_n(&g_js_idle_iterations, __ATOMIC_ACQUIRE);
            script_start(ui.scenario->setup, false);
            enter(STATE_SETUP, now);
        } else if (now - ui.state_start_us > BOOT_TIMEOUT_US) {
            fail_scenario("Deck App did not start", now);
        }
        break;
    case STATE_SETUP:
        // Each setup step waits for the JS thread to handle the last one and go idle twice: the second idle
        // iteration would have found the step's input, so the Deck App has rendered it and the next step sees the
        // new tree.
        if (ui.script_frame == 0 && __atomic_load_n(&g_js_idle_iterations, __ATOMIC_ACQUIRE) < ui.setup_idle_mark + 2)
            break;
        if (ui.script_frame == FRAMES_PER_STEP - 1) {
            ui.setup_idle_mark = __atomic_load_n(&g_js_idle_iterations, __ATOMIC_ACQUIRE);
        }
        if (!script_play(&ui.sample)) {
            script_start(ui.scenario->loop, true);
            enter(STATE_WARMUP, now);
        }
        break;
    case STATE_WARMUP:
        script_play(&ui.sample);
        if (now - ui.state_start_us >= WARMUP_US) {
            enter(STATE_MEASURE, now);
            ui.measure_start_us = now;
            ui.frame_count = 0;
            ui.frame_overflow = 0;
            ui.cpu_start_us = vd_thread_cpu_us();
            __atomic_store_n(&g_window, WINDOW_MEASURE, __ATOMIC_RELEASE);
        }
        break;
    case STATE_MEASURE:
        script_play(&ui.sample);
        if (now - ui.state_start_us >= MEASURE_US) {
            const uint64_t cpu_end_us = vd_thread_cpu_us();
            ui.cpu_known = cpu_end_us != 0;
            ui.cpu_us = cpu_end_us - ui.cpu_start_us;
            ui.measured_us = now - ui.measure_start_us;
            __atomic_store_n(&g_window, WINDOW_DONE, __ATOMIC_RELEASE);
            enter(STATE_DRAIN, now);
        }
        break;
    case STATE_DRAIN: {
        const bool js_done = __atomic_load_n(&g_js_ack, __ATOMIC_ACQUIRE) != 0;
        if (js_done || now - ui.state_start_us > DRAIN_TIMEOUT_US) {
            write_scenario(js_done);
            next_scenario(now);
        }
        break;
    }
    case STATE_DONE:
        break;
    }
    return ui.state != STATE_DONE;
}

void bench_apply_input(void)
{
    input_apply_gamepad(&ui.sample);
}

void bench_frame_mark(VdBenchFrameSpan span)
{
    if (!g_active) return;
    const uint64_t now = vd_now_us();
    ui.spans[span] += (uint32_t)(now - ui.last_mark_us);
    ui.last_mark_us = now;
}

void bench_js_thread_start(void)
{
    if (!g_active) return;
    js.recording = false;
    js.thread_start_us = vd_now_us();
    js.startup_us = 0;
}

void bench_js_ready(void)
{
    if (!g_active) return;
    js.startup_us = vd_now_us() - js.thread_start_us;
}

void bench_js_iteration_begin(void)
{
    if (!g_active) return;
    js.last_mark_us = vd_now_us();
    memset(js.spans, 0, sizeof(js.spans));
}

void bench_js_mark(VdBenchJsSpan span)
{
    if (!g_active) return;
    const uint64_t now = vd_now_us();
    js.spans[span] += (uint32_t)(now - js.last_mark_us);
    if (js.recording) js.span_totals[span] += now - js.last_mark_us;
    js.last_mark_us = now;
}

// Starts recording when the UI thread opens the window, and hands the samples over once it closes. open_span is
// the span in progress, which the window may cut through; -1 between iterations.
static void poll_window(JSRuntime *rt, int open_span)
{
    const int window = __atomic_load_n(&g_window, __ATOMIC_ACQUIRE);
    if (window == WINDOW_MEASURE && !js.recording) {
        js.recording = true;
        js.last_mark_us = vd_now_us();
        memset(js.span_totals, 0, sizeof(js.span_totals));
        js.iteration_count = 0;
        js.iteration_overflow = 0;
        js.swap_count = 0;
        js.commit_count = 0;
        js.commit_start_us = 0;
        js.input_count = 0;
        js.mutations_start = instance_back_mutation_count();
        js.cpu_start_us = vd_thread_cpu_us();
    } else if (window == WINDOW_DONE && js.recording) {
        if (open_span >= 0) js.span_totals[open_span] += vd_now_us() - js.last_mark_us;
        js.recording = false;
        js.cpu_us = vd_thread_cpu_us() - js.cpu_start_us;
        js.mutations = instance_back_mutation_count() - js.mutations_start;
        JSMemoryUsage usage;
        JS_ComputeMemoryUsage(rt, &usage);
        js.heap_used = usage.memory_used_size;
        js.heap_malloc = usage.malloc_size;
        __atomic_store_n(&g_js_ack, 1, __ATOMIC_RELEASE);
    }
}

void bench_js_iteration_end(JSRuntime *rt, bool busy, bool swapped)
{
    if (!g_active) return;
    if (js.recording && busy) {
        if (js.iteration_count < JS_ITERATION_CAPACITY) {
            memcpy(js.iterations[js.iteration_count++], js.spans, sizeof(js.spans));
            if (swapped) js.swaps[js.swap_count++] = js.spans[VD_BENCH_JS_SWAP];
        } else {
            js.iteration_overflow++;
        }
    }

    if (!busy) __atomic_add_fetch(&g_js_idle_iterations, 1, __ATOMIC_RELEASE);
    poll_window(rt, -1);
}

void bench_js_input_handled(JSRuntime *rt, uint64_t queued_us, uint64_t started_us)
{
    if (!g_active) return;
    if (js.recording) {
        const uint64_t now = vd_now_us();
        if (js.input_count < INPUT_CAPACITY) {
            js.inputs[js.input_count] = (uint32_t)(now - queued_us);
            js.handlers[js.input_count] = (uint32_t)(now - started_us);
        }
        js.input_count++;
    }
    poll_window(rt, VD_BENCH_JS_INPUT);
}

void bench_commit(bool ended)
{
    if (!g_active || !js.recording) return;
    const uint64_t now = vd_now_us();
    if (!ended) {
        js.commit_start_us = now;
    } else if (js.commit_start_us) {
        if (js.commit_count < COMMIT_CAPACITY) js.commits[js.commit_count] = (uint32_t)(now - js.commit_start_us);
        js.commit_count++;
        js.commit_start_us = 0;
    }
}
