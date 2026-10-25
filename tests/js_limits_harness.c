// Runs runtime.js stand-ins on the real JS thread and checks that hitting the QuickJS stack and memory
// limits or the fetch body cap reaches the Deck App Failure Screen instead of crashing, and that only
// promise rejections still unhandled after the microtask queue drains do.
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "core/event_queue.h"
#include "core/js_runtime.h"
#include "platform/thread.h"

#define WAIT_TIMEOUT_MS 10000
#define FETCH_LIMIT_BYTES (4 * 1024 * 1024)

#define DECK_APP_PRELUDE "globalThis.vitadeck = { updateContainer() {} };\n"

static char work_dir[] = "/tmp/vitadeck-js-limits-XXXXXX";

static bool write_file(const char *path, const char *contents, size_t len)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = fwrite(contents, 1, len, f) == len;
    return fclose(f) == 0 && ok;
}

static bool write_body_file(const char *name, size_t len)
{
    char *body = malloc(len);
    if (!body) return false;
    memset(body, 'x', len);
    bool ok = write_file(name, body, len);
    free(body);
    return ok;
}

// Starts the JS thread on `source` as js/runtime.js and expects a failure whose message contains `expected`.
static int expect_failure(const char *name, const char *source, const char *expected)
{
    if (!write_file("js/runtime.js", source, strlen(source))) {
        fprintf(stderr, "%s: could not write js/runtime.js\n", name);
        return 1;
    }

    VdJsRuntime runtime;
    js_runtime_init(&runtime);
    if (!js_runtime_start(&runtime)) {
        fprintf(stderr, "%s: could not start the JS thread\n", name);
        return 1;
    }
    for (int waited = 0; !js_runtime_failed(&runtime) && waited < WAIT_TIMEOUT_MS; waited++)
        vd_sleep_ms(1);

    int ret = 0;
    if (!js_runtime_failed(&runtime)) {
        fprintf(stderr, "%s: no failure reported\n", name);
        ret = 1;
    } else if (!strstr(runtime.failure_message, expected)) {
        fprintf(stderr, "%s: expected \"%s\", got \"%s\"\n", name, expected, runtime.failure_message);
        ret = 1;
    }
    js_runtime_stop(&runtime);
    return ret;
}

static int expect_fetch(const char *name, const char *file, const char *expected)
{
    char source[1024];
    int n = snprintf(source, sizeof(source),
                     DECK_APP_PRELUDE "nativeFetch('file://%s/%s', 'GET').then("
                                      "(r) => nativeReportDeckAppError('resolved ' + r.body.length),"
                                      "(e) => nativeReportDeckAppError('rejected: ' + e.message));\n",
                     work_dir, file);
    if (n < 0 || (size_t)n >= sizeof(source)) {
        fprintf(stderr, "%s: fetch source truncated\n", name);
        return 1;
    }
    return expect_failure(name, source, expected);
}

int main(void)
{
    SetTraceLogLevel(LOG_NONE);
    if (!mkdtemp(work_dir) || chdir(work_dir) != 0 || mkdir("js", 0755) != 0) {
        perror("js_limits_harness: work dir");
        return 2;
    }
    if (!write_body_file("at-limit.txt", FETCH_LIMIT_BYTES) ||
        !write_body_file("over-limit.txt", FETCH_LIMIT_BYTES + 1)) {
        fprintf(stderr, "js_limits_harness: could not write fetch bodies\n");
        return 2;
    }
    if (!event_queue_init()) return 2;

    int failures = 0;
    failures += expect_failure("stack", DECK_APP_PRELUDE "(function recurse() { return recurse() + 1; })();\n",
                               "stack overflow");
    failures += expect_failure("memory",
                               DECK_APP_PRELUDE "(() => {"
                                                "  const chunks = [];"
                                                "  for (;;) chunks.push(new Array(100000).fill(0));"
                                                "})();\n",
                               "out of memory");
    failures += expect_fetch("fetch at limit", "at-limit.txt", "resolved 4194304");
    failures += expect_fetch("fetch over limit", "over-limit.txt", "rejected: fetch response body exceeds the 4 MiB limit");
    failures += expect_failure("unhandled rejection", DECK_APP_PRELUDE "Promise.reject(new Error('boom'));\n", "boom");
    failures += expect_failure("unhandled rejection in timer",
                               DECK_APP_PRELUDE "setTimeout(() => { Promise.reject(new Error('late boom')); }, 0);\n",
                               "late boom");
    // nativeReportDeckAppError only shows if nothing failed first, since the first failure is kept.
    failures += expect_failure("rejection handled later",
                               DECK_APP_PRELUDE "const p = Promise.reject(new Error('boom'));"
                                                "Promise.resolve().then(() => p.catch(() => {}));"
                                                "setTimeout(() => nativeReportDeckAppError('still running'), 0);\n",
                               "still running");

    event_queue_destroy();
    unlink("at-limit.txt");
    unlink("over-limit.txt");
    unlink("js/runtime.js");
    rmdir("js");
    if (chdir("/") == 0) rmdir(work_dir);

    if (failures == 0) printf("js_limits_harness (stack %d KiB): ok\n", VD_THREAD_STACK_SIZE / 1024);
    return failures == 0 ? 0 : 1;
}
