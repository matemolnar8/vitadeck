#include "host_control.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>

#include "platform/thread.h"
#include "quickjs.h"

#define HC_MAX_PENDING 16
#define HC_POLL_TIMEOUT_MS 45000
#define HC_POLL_SLEEP_MS 50
#define HC_COMMAND_MAX 96
#define HC_JSON_MAX 65536

typedef struct {
    bool in_use;
    int request_id;
    JSValue resolve;
    JSValue reject;
} HcPending;

typedef struct {
    vd_mutex *mutex;
    int next_request_id;
    bool poll_active;
    bool has_work;
    int work_request_id;
    char work_command[HC_COMMAND_MAX];
    char *work_payload_json;
    HcPending pending[HC_MAX_PENDING];
    struct {
        int request_id;
        char *result_json;
    } done_queue[HC_MAX_PENDING];
    int done_head;
    int done_tail;
    int done_count;
} HostControlState;

static HostControlState g_hc;

static int64_t hc_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void hc_send_simple(int fd, int status, const char *status_text, const char *content_type, const char *body)
{
    char header[512];
    int body_len = body ? (int)strlen(body) : 0;
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, status_text, content_type, body_len);
    send(fd, header, (size_t)header_len, 0);
    if (body_len > 0) send(fd, body, (size_t)body_len, 0);
}

static void hc_send_json(int fd, int status, const char *body)
{
    const char *status_text = status == 200 ? "OK" : status == 204 ? "No Content" : status == 400 ? "Bad Request" : "Error";
    hc_send_simple(fd, status, status_text, "application/json", body);
}

static void hc_push_done(int request_id, char *result_json)
{
    if (g_hc.done_count >= HC_MAX_PENDING) {
        free(result_json);
        return;
    }
    g_hc.done_queue[g_hc.done_tail].request_id = request_id;
    g_hc.done_queue[g_hc.done_tail].result_json = result_json;
    g_hc.done_tail = (g_hc.done_tail + 1) % HC_MAX_PENDING;
    g_hc.done_count++;
}

static bool hc_pop_done(int *request_id, char **result_json)
{
    if (g_hc.done_count == 0) return false;
    *request_id = g_hc.done_queue[g_hc.done_head].request_id;
    *result_json = g_hc.done_queue[g_hc.done_head].result_json;
    g_hc.done_queue[g_hc.done_head].result_json = NULL;
    g_hc.done_head = (g_hc.done_head + 1) % HC_MAX_PENDING;
    g_hc.done_count--;
    return true;
}

static int hc_pending_index(int request_id)
{
    for (int i = 0; i < HC_MAX_PENDING; i++) {
        if (g_hc.pending[i].in_use && g_hc.pending[i].request_id == request_id) return i;
    }
    return -1;
}

static void hc_free_pending(JSContext *ctx, int index)
{
    if (index < 0 || index >= HC_MAX_PENDING) return;
    HcPending *p = &g_hc.pending[index];
    if (!p->in_use) return;
    JS_FreeValue(ctx, p->resolve);
    JS_FreeValue(ctx, p->reject);
    memset(p, 0, sizeof(*p));
}

static void hc_reject_pending(JSContext *ctx, int index, const char *message)
{
    HcPending *p = &g_hc.pending[index];
    JSValue err = JS_NewString(ctx, message);
    JSValue ret = JS_Call(ctx, p->reject, JS_UNDEFINED, 1, &err);
    if (JS_IsException(ret)) JS_GetException(ctx);
    JS_FreeValue(ctx, ret);
    JS_FreeValue(ctx, err);
    hc_free_pending(ctx, index);
}

void host_control_init(void)
{
    memset(&g_hc, 0, sizeof(g_hc));
    g_hc.next_request_id = 1;
    if (!g_hc.mutex) g_hc.mutex = vd_mutex_create();
}

void host_control_shutdown(JSContext *ctx)
{
    if (!g_hc.mutex) return;
    vd_mutex_lock(g_hc.mutex);
    for (int i = 0; i < HC_MAX_PENDING; i++) {
        if (g_hc.pending[i].in_use) {
            vd_mutex_unlock(g_hc.mutex);
            hc_reject_pending(ctx, i, "Host Control shut down.");
            vd_mutex_lock(g_hc.mutex);
        }
    }
    free(g_hc.work_payload_json);
    g_hc.work_payload_json = NULL;
    g_hc.has_work = false;
    g_hc.poll_active = false;
    vd_mutex_unlock(g_hc.mutex);

    char *json = NULL;
    int rid = 0;
    while (hc_pop_done(&rid, &json)) free(json);
}

bool host_control_poll_waiting(void)
{
    bool waiting = false;
    if (!g_hc.mutex) return false;
    vd_mutex_lock(g_hc.mutex);
    waiting = g_hc.poll_active;
    vd_mutex_unlock(g_hc.mutex);
    return waiting;
}

void host_control_handle_poll(int client_fd)
{
    if (!g_hc.mutex) host_control_init();

    vd_mutex_lock(g_hc.mutex);
    if (g_hc.poll_active) {
        vd_mutex_unlock(g_hc.mutex);
        hc_send_json(client_fd, 409, "{\"ok\":false,\"code\":\"method_not_allowed\",\"message\":\"Host poll already active.\"}");
        return;
    }
    g_hc.poll_active = true;
    vd_mutex_unlock(g_hc.mutex);

    int64_t deadline = hc_now_ms() + HC_POLL_TIMEOUT_MS;
    bool got_work = false;

    while (hc_now_ms() < deadline) {
        vd_mutex_lock(g_hc.mutex);
        got_work = g_hc.has_work;
        if (got_work) {
            g_hc.has_work = false;
            vd_mutex_unlock(g_hc.mutex);
            break;
        }
        vd_mutex_unlock(g_hc.mutex);
        struct timespec ts = { .tv_sec = 0, .tv_nsec = HC_POLL_SLEEP_MS * 1000000L };
        nanosleep(&ts, NULL);
    }

    vd_mutex_lock(g_hc.mutex);
    g_hc.poll_active = false;

    if (!got_work) {
        vd_mutex_unlock(g_hc.mutex);
        hc_send_simple(client_fd, 204, "No Content", "application/json", "");
        return;
    }

    char body[HC_JSON_MAX];
    const char *payload = g_hc.work_payload_json ? g_hc.work_payload_json : "null";
    int n = snprintf(body, sizeof(body),
        "{\"ok\":true,\"requestId\":%d,\"command\":\"%s\",\"payload\":%s}",
        g_hc.work_request_id,
        g_hc.work_command,
        payload);
    free(g_hc.work_payload_json);
    g_hc.work_payload_json = NULL;
    vd_mutex_unlock(g_hc.mutex);

    if (n < 0 || (size_t)n >= sizeof(body)) {
        hc_send_json(client_fd, 500, "{\"ok\":false,\"code\":\"command_failed\",\"message\":\"Response too large.\"}");
        return;
    }
    hc_send_json(client_fd, 200, body);
}

static int hc_parse_request_id(const char *body, int *out_id)
{
    const char *key = strstr(body, "\"requestId\"");
    if (!key) return -1;
    const char *colon = strchr(key, ':');
    if (!colon) return -1;
    *out_id = atoi(colon + 1);
    return 0;
}

static char *hc_copy_span(const char *start, const char *end_inclusive)
{
    if (!start || !end_inclusive || end_inclusive < start) return NULL;
    size_t len = (size_t)(end_inclusive - start + 1);
    char *copy = malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, start, len);
    copy[len] = '\0';
    return copy;
}

static char *hc_extract_result_json(const char *body)
{
    const char *key = strstr(body, "\"result\"");
    if (!key) return NULL;
    const char *start = strchr(key, ':');
    if (!start) return NULL;
    start++;
    while (*start == ' ' || *start == '\t') start++;

    if (*start == '{') {
        int depth = 0;
        for (const char *p = start; *p; p++) {
            if (*p == '{') depth++;
            else if (*p == '}') {
                depth--;
                if (depth == 0) return hc_copy_span(start, p);
            }
        }
        return NULL;
    }

    if (*start == '[') {
        int depth = 0;
        for (const char *p = start; *p; p++) {
            if (*p == '[') depth++;
            else if (*p == ']') {
                depth--;
                if (depth == 0) return hc_copy_span(start, p);
            }
        }
        return NULL;
    }

    if (*start == '"') {
        for (const char *p = start + 1; *p; p++) {
            if (*p == '\\' && p[1]) {
                p++;
                continue;
            }
            if (*p == '"') return hc_copy_span(start, p);
        }
        return NULL;
    }

    const char *end = start;

    while (*end && *end != ',' && *end != '}' && *end != '\n' && *end != '\r') end++;
    if (end == start) return NULL;
    return hc_copy_span(start, end - 1);
}

void host_control_handle_result(int client_fd, const char *body, size_t body_len)
{
    (void)body_len;
    if (!body) {
        hc_send_json(client_fd, 400, "{\"ok\":false,\"code\":\"malformed_request\",\"message\":\"Missing body.\"}");
        return;
    }

    int request_id = 0;
    if (hc_parse_request_id(body, &request_id) != 0) {
        hc_send_json(client_fd, 400, "{\"ok\":false,\"code\":\"malformed_request\",\"message\":\"Missing requestId.\"}");
        return;
    }

    char *result_json = hc_extract_result_json(body);
    if (!result_json) {
        hc_send_json(client_fd, 400, "{\"ok\":false,\"code\":\"malformed_request\",\"message\":\"Missing result.\"}");
        return;
    }

    vd_mutex_lock(g_hc.mutex);
    hc_push_done(request_id, result_json);
    vd_mutex_unlock(g_hc.mutex);

    hc_send_json(client_fd, 200, "{\"ok\":true}");
}

void host_control_drain_completions(JSContext *ctx, JSRuntime *rt)
{
    int request_id = 0;
    char *result_json = NULL;
    while (hc_pop_done(&request_id, &result_json)) {
        int index = hc_pending_index(request_id);
        if (index < 0) {
            free(result_json);
            continue;
        }
        HcPending *pending = &g_hc.pending[index];
        JSValue result = JS_NewString(ctx, result_json);
        JSValue ret = JS_Call(ctx, pending->resolve, JS_UNDEFINED, 1, &result);
        if (JS_IsException(ret)) JS_GetException(ctx);
        JS_FreeValue(ctx, ret);
        JS_FreeValue(ctx, result);
        hc_free_pending(ctx, index);
        free(result_json);
    }
    while (JS_ExecutePendingJob(rt, NULL) > 0) {}
}

static bool hc_enqueue_command(const char *command, const char *payload_json, int timeout_ms, int *out_request_id)
{
    (void)timeout_ms;
    if (!g_hc.mutex) host_control_init();

    vd_mutex_lock(g_hc.mutex);
    if (g_hc.has_work) {
        vd_mutex_unlock(g_hc.mutex);
        return false;
    }

    int request_id = g_hc.next_request_id++;
    snprintf(g_hc.work_command, sizeof(g_hc.work_command), "%s", command);
    free(g_hc.work_payload_json);
    g_hc.work_payload_json = payload_json ? strdup(payload_json) : NULL;
    g_hc.work_request_id = request_id;
    g_hc.has_work = true;
    *out_request_id = request_id;
    vd_mutex_unlock(g_hc.mutex);
    return true;
}

static int hc_alloc_pending(JSContext *ctx, JSValue resolve, JSValue reject, int request_id)
{
    for (int i = 0; i < HC_MAX_PENDING; i++) {
        if (!g_hc.pending[i].in_use) {
            g_hc.pending[i].in_use = true;
            g_hc.pending[i].request_id = request_id;
            g_hc.pending[i].resolve = JS_DupValue(ctx, resolve);
            g_hc.pending[i].reject = JS_DupValue(ctx, reject);
            return i;
        }
    }
    return -1;
}

JSValue native_host_control_command(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 2) {
        return JS_ThrowTypeError(ctx, "nativeHostControlCommand requires command and payloadJson");
    }

    const char *command = JS_ToCString(ctx, argv[0]);
    const char *payload_json = JS_ToCString(ctx, argv[1]);
    int timeout_ms = 10000;
    if (argc >= 3) JS_ToInt32(ctx, &timeout_ms, argv[2]);
    if (!command || !payload_json) {
        if (command) JS_FreeCString(ctx, command);
        if (payload_json) JS_FreeCString(ctx, payload_json);
        return JS_EXCEPTION;
    }

    JSValue resolving_funcs[2];
    JSValue promise = JS_NewPromiseCapability(ctx, resolving_funcs);
    if (JS_IsException(promise)) {
        JS_FreeCString(ctx, command);
        JS_FreeCString(ctx, payload_json);
        return promise;
    }

    int request_id = 0;
    if (!hc_enqueue_command(command, payload_json, timeout_ms, &request_id)) {
        JS_FreeCString(ctx, command);
        JS_FreeCString(ctx, payload_json);
        JSValue err = JS_NewString(ctx, "Host Control Unavailable");
        JSValue ret = JS_Call(ctx, resolving_funcs[1], JS_UNDEFINED, 1, &err);
        if (JS_IsException(ret)) JS_GetException(ctx);
        JS_FreeValue(ctx, ret);
        JS_FreeValue(ctx, err);
        JS_FreeValue(ctx, resolving_funcs[0]);
        JS_FreeValue(ctx, resolving_funcs[1]);
        return promise;
    }

    if (hc_alloc_pending(ctx, resolving_funcs[0], resolving_funcs[1], request_id) < 0) {
        JS_FreeCString(ctx, command);
        JS_FreeCString(ctx, payload_json);
        JSValue err = JS_NewString(ctx, "Host Control queue full");
        JSValue ret = JS_Call(ctx, resolving_funcs[1], JS_UNDEFINED, 1, &err);
        if (JS_IsException(ret)) JS_GetException(ctx);
        JS_FreeValue(ctx, ret);
        JS_FreeValue(ctx, err);
        JS_FreeValue(ctx, resolving_funcs[0]);
        JS_FreeValue(ctx, resolving_funcs[1]);
        return promise;
    }

    (void)timeout_ms;
    JS_FreeCString(ctx, command);
    JS_FreeCString(ctx, payload_json);
    JS_FreeValue(ctx, resolving_funcs[0]);
    JS_FreeValue(ctx, resolving_funcs[1]);
    return promise;
}
