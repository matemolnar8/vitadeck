#include "host_control.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../core/host_control_link.h"
#include "platform/thread.h"

#ifdef __vita__
#include <psp2/sysmodule.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>
#else
#include <curl/curl.h>
#endif

#define VD_HOST_HTTP_QUEUE_CAP 16
#define VD_HOST_HTTP_MAX_BODY 65536

typedef struct {
    int request_id;
    char *url;
    char *body;
    int timeout_ms;
} HostHttpJob;

typedef struct {
    int request_id;
    long status;
    char *body;
    char error[256];
} HostHttpDone;

typedef struct {
    bool in_use;
    int request_id;
    JSValue resolve;
    JSValue reject;
} HostHttpPending;

typedef struct {
    HostHttpJob jobs[VD_HOST_HTTP_QUEUE_CAP];
    int job_head;
    int job_tail;
    int job_count;

    HostHttpDone dones[VD_HOST_HTTP_QUEUE_CAP];
    int done_head;
    int done_tail;
    int done_count;

    HostHttpPending pending[VD_HOST_HTTP_QUEUE_CAP];
    int next_request_id;

    vd_mutex *mutex;
    vd_thread *worker;
    bool worker_started;
    bool shutdown;
} HostHttpState;

static HostHttpState g_host_http;

static void host_http_ensure_mutex(void)
{
    if (!g_host_http.mutex) g_host_http.mutex = vd_mutex_create();
}

static int host_http_pending_index(int request_id)
{
    for (int i = 0; i < VD_HOST_HTTP_QUEUE_CAP; i++) {
        if (g_host_http.pending[i].in_use && g_host_http.pending[i].request_id == request_id) return i;
    }
    return -1;
}

static int host_http_alloc_pending(JSContext *ctx, JSValue resolve, JSValue reject, int *out_request_id)
{
    for (int i = 0; i < VD_HOST_HTTP_QUEUE_CAP; i++) {
        if (!g_host_http.pending[i].in_use) {
            g_host_http.pending[i].in_use = true;
            g_host_http.pending[i].request_id = g_host_http.next_request_id++;
            g_host_http.pending[i].resolve = JS_DupValue(ctx, resolve);
            g_host_http.pending[i].reject = JS_DupValue(ctx, reject);
            *out_request_id = g_host_http.pending[i].request_id;
            return i;
        }
    }
    return -1;
}

static void host_http_free_pending(JSContext *ctx, int index)
{
    if (index < 0 || index >= VD_HOST_HTTP_QUEUE_CAP) return;
    HostHttpPending *pending = &g_host_http.pending[index];
    if (!pending->in_use) return;
    JS_FreeValue(ctx, pending->resolve);
    JS_FreeValue(ctx, pending->reject);
    memset(pending, 0, sizeof(*pending));
}

static bool host_http_push_job(const HostHttpJob *job)
{
    host_http_ensure_mutex();
    vd_mutex_lock(g_host_http.mutex);
    if (g_host_http.job_count >= VD_HOST_HTTP_QUEUE_CAP) {
        vd_mutex_unlock(g_host_http.mutex);
        return false;
    }
    g_host_http.jobs[g_host_http.job_tail] = *job;
    g_host_http.job_tail = (g_host_http.job_tail + 1) % VD_HOST_HTTP_QUEUE_CAP;
    g_host_http.job_count++;
    vd_mutex_unlock(g_host_http.mutex);
    return true;
}

static bool host_http_pop_job(HostHttpJob *job)
{
    host_http_ensure_mutex();
    vd_mutex_lock(g_host_http.mutex);
    if (g_host_http.job_count == 0) {
        vd_mutex_unlock(g_host_http.mutex);
        return false;
    }
    *job = g_host_http.jobs[g_host_http.job_head];
    g_host_http.job_head = (g_host_http.job_head + 1) % VD_HOST_HTTP_QUEUE_CAP;
    g_host_http.job_count--;
    vd_mutex_unlock(g_host_http.mutex);
    return true;
}

static void host_http_push_done(const HostHttpDone *done)
{
    host_http_ensure_mutex();
    vd_mutex_lock(g_host_http.mutex);
    if (g_host_http.done_count < VD_HOST_HTTP_QUEUE_CAP) {
        g_host_http.dones[g_host_http.done_tail] = *done;
        g_host_http.done_tail = (g_host_http.done_tail + 1) % VD_HOST_HTTP_QUEUE_CAP;
        g_host_http.done_count++;
    } else {
        free(done->body);
    }
    vd_mutex_unlock(g_host_http.mutex);
}

static bool host_http_pop_done(HostHttpDone *done)
{
    host_http_ensure_mutex();
    vd_mutex_lock(g_host_http.mutex);
    if (g_host_http.done_count == 0) {
        vd_mutex_unlock(g_host_http.mutex);
        return false;
    }
    *done = g_host_http.dones[g_host_http.done_head];
    g_host_http.done_head = (g_host_http.done_head + 1) % VD_HOST_HTTP_QUEUE_CAP;
    g_host_http.done_count--;
    vd_mutex_unlock(g_host_http.mutex);
    return true;
}

static char *host_http_json_escape(const char *value)
{
    if (!value) value = "";
    size_t len = strlen(value);
    char *out = malloc(len * 6 + 1);
    if (!out) return NULL;

    char *p = out;
    for (const unsigned char *s = (const unsigned char *)value; *s; s++) {
        switch (*s) {
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '"': *p++ = '\\'; *p++ = '"'; break;
            case '\n': *p++ = '\\'; *p++ = 'n'; break;
            case '\r': *p++ = '\\'; *p++ = 'r'; break;
            case '\t': *p++ = '\\'; *p++ = 't'; break;
            default:
                if (*s < 0x20) {
                    snprintf(p, 7, "\\u%04x", *s);
                    p += 6;
                } else {
                    *p++ = (char)*s;
                }
                break;
        }
    }
    *p = '\0';
    return out;
}

#ifndef __vita__

typedef struct {
    char *data;
    size_t size;
} HostHttpBuffer;

static size_t host_http_curl_write(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total = size * nmemb;
    HostHttpBuffer *buffer = userp;
    if (buffer->size + total > VD_HOST_HTTP_MAX_BODY) return 0;

    char *next = realloc(buffer->data, buffer->size + total + 1);
    if (!next) return 0;
    buffer->data = next;
    memcpy(buffer->data + buffer->size, contents, total);
    buffer->size += total;
    buffer->data[buffer->size] = '\0';
    return total;
}

static bool g_curl_initialized;

static void host_http_curl_init(void)
{
    if (!g_curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        g_curl_initialized = true;
    }
}

static void host_http_perform_curl(const HostHttpJob *job, HostHttpDone *done)
{
    done->request_id = job->request_id;
    done->status = 0;
    done->body = NULL;
    done->error[0] = '\0';

    host_http_curl_init();

    HostHttpBuffer response = {0};
    CURL *curl = curl_easy_init();
    if (!curl) {
        snprintf(done->error, sizeof(done->error), "Could not initialize HTTP client.");
        return;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, job->url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, job->body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(job->body));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, host_http_curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, job->timeout_ms > 0 ? (long)job->timeout_ms : 5000L);

    CURLcode code = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &done->status);

    if (code != CURLE_OK) {
        snprintf(done->error, sizeof(done->error), "%s", curl_easy_strerror(code));
        free(response.data);
    } else {
        done->body = response.data ? response.data : strdup("");
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

#else

static void *g_net_mem;
static bool g_sce_http_ready;

static int host_http_sce_init(void)
{
    if (g_sce_http_ready) return 0;

    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET_CTL);
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);

    g_net_mem = malloc(256 * 1024);
    if (!g_net_mem) return -1;

    SceNetInitParam param = { .memory = g_net_mem, .size = 256 * 1024, .flags = 0 };
    if (sceNetInit(&param) < 0) return -1;
    if (sceNetCtlInit() < 0) return -1;
    if (sceHttpInit(256 * 1024) < 0) return -1;

    g_sce_http_ready = true;
    return 0;
}

static void host_http_perform_sce(const HostHttpJob *job, HostHttpDone *done)
{
    done->request_id = job->request_id;
    done->status = 0;
    done->body = NULL;
    done->error[0] = '\0';

    if (host_http_sce_init() < 0) {
        snprintf(done->error, sizeof(done->error), "HTTP stack init failed.");
        return;
    }

    int tmpl = sceHttpCreateTemplate("vitadeck", SCE_HTTP_VERSION_1_1, SCE_HTTP_HEADER_OVERWRITE);
    if (tmpl < 0) {
        snprintf(done->error, sizeof(done->error), "sceHttpCreateTemplate failed (0x%08X).", tmpl);
        return;
    }

    unsigned int timeout_usec = (unsigned int)(job->timeout_ms > 0 ? job->timeout_ms : 5000) * 1000U;
    sceHttpSetConnectTimeOut(tmpl, timeout_usec);
    sceHttpSetRecvTimeOut(tmpl, timeout_usec);
    sceHttpSetSendTimeOut(tmpl, timeout_usec);

    int conn = sceHttpCreateConnectionWithURL(tmpl, job->url, 0);
    if (conn < 0) {
        snprintf(done->error, sizeof(done->error), "sceHttpCreateConnectionWithURL failed (0x%08X).", conn);
        sceHttpDeleteTemplate(tmpl);
        return;
    }

    unsigned long long content_length = (unsigned long long)strlen(job->body);
    int req = -1;
    req = sceHttpCreateRequestWithURL(conn, SCE_HTTP_METHOD_POST, job->url, content_length);
    if (req < 0) {
        snprintf(done->error, sizeof(done->error), "sceHttpCreateRequestWithURL failed (0x%08X).", req);
        sceHttpDeleteConnection(conn);
        sceHttpDeleteTemplate(tmpl);
        return;
    }

    sceHttpAddRequestHeader(req, "Content-Type", "application/json", SCE_HTTP_HEADER_OVERWRITE);
    sceHttpAddRequestHeader(req, "Accept", "application/json", SCE_HTTP_HEADER_OVERWRITE);

    int send = sceHttpSendRequest(req, job->body, (unsigned int)content_length);
    if (send < 0) {
        snprintf(done->error, sizeof(done->error), "sceHttpSendRequest failed (0x%08X).", send);
        goto cleanup;
    }

    size_t total = 0;
    char chunk[4096];
    char *buffer = NULL;
    for (;;) {
        int read = sceHttpReadData(req, chunk, sizeof(chunk));
        if (read < 0) {
            snprintf(done->error, sizeof(done->error), "sceHttpReadData failed (0x%08X).", read);
            free(buffer);
            buffer = NULL;
            break;
        }
        if (read == 0) break;
        if (total + (size_t)read > VD_HOST_HTTP_MAX_BODY) {
            snprintf(done->error, sizeof(done->error), "Response body exceeds 64KB.");
            free(buffer);
            buffer = NULL;
            break;
        }
        char *next = realloc(buffer, total + (size_t)read + 1);
        if (!next) {
            snprintf(done->error, sizeof(done->error), "Out of memory reading HTTP response.");
            free(buffer);
            buffer = NULL;
            break;
        }
        buffer = next;
        memcpy(buffer + total, chunk, (size_t)read);
        total += (size_t)read;
        buffer[total] = '\0';
    }

    if (!done->error[0]) {
        int status = 0;
        if (sceHttpGetStatusCode(req, &status) < 0) status = 0;
        done->status = status;
        done->body = buffer ? buffer : strdup("");
        buffer = NULL;
    }

cleanup:
    if (req >= 0) sceHttpDeleteRequest(req);
    sceHttpDeleteConnection(conn);
    sceHttpDeleteTemplate(tmpl);
    free(buffer);
}

#endif

static void host_http_perform(const HostHttpJob *job, HostHttpDone *done)
{
#ifdef __vita__
    host_http_perform_sce(job, done);
#else
    host_http_perform_curl(job, done);
#endif
}

static void *host_http_worker_main(void *arg)
{
    (void)arg;
    while (true) {
        host_http_ensure_mutex();
        vd_mutex_lock(g_host_http.mutex);
        bool stopping = g_host_http.shutdown;
        vd_mutex_unlock(g_host_http.mutex);

        HostHttpJob job;
        if (!host_http_pop_job(&job)) {
            if (stopping) break;
            vd_thread_yield();
            continue;
        }

        HostHttpDone done = {0};
        host_http_perform(&job, &done);
        free(job.url);
        free(job.body);
        host_http_push_done(&done);
    }
    return NULL;
}

static bool host_http_ensure_worker(void)
{
    if (g_host_http.worker_started) return true;
    g_host_http.worker = vd_thread_create(host_http_worker_main, NULL);
    if (!g_host_http.worker) return false;
    g_host_http.worker_started = true;
    return true;
}

static char *host_http_build_transport_json(const HostHttpDone *done)
{
    char *body = host_http_json_escape(done->body ? done->body : "");
    char *error = host_http_json_escape(done->error);
    if (!body || !error) {
        free(body);
        free(error);
        return NULL;
    }

    size_t json_size = strlen(body) + strlen(error) + 96;
    char *json = malloc(json_size);
    if (!json) {
        free(body);
        free(error);
        return NULL;
    }
    snprintf(json, json_size, "{\"status\":%ld,\"body\":\"%s\",\"error\":\"%s\"}", done->status, body, error);
    free(body);
    free(error);
    return json;
}

static void host_http_reject_pending(JSContext *ctx, int index, const char *message)
{
    HostHttpPending *pending = &g_host_http.pending[index];
    JSValue err = JS_NewString(ctx, message);
    JSValue ret = JS_Call(ctx, pending->reject, JS_UNDEFINED, 1, &err);
    if (JS_IsException(ret)) JS_GetException(ctx);
    JS_FreeValue(ctx, ret);
    JS_FreeValue(ctx, err);
    host_http_free_pending(ctx, index);
}

void host_control_drain_completions(JSContext *ctx, JSRuntime *rt)
{
    HostHttpDone done;
    while (host_http_pop_done(&done)) {
        int index = host_http_pending_index(done.request_id);
        if (index < 0) {
            free(done.body);
            continue;
        }

        HostHttpPending *pending = &g_host_http.pending[index];
        if (done.error[0]) {
            TraceLog(LOG_ERROR, "Host Control HTTP failed: %s", done.error);
            host_http_reject_pending(ctx, index, done.error);
        } else {
            char *json = host_http_build_transport_json(&done);
            if (!json) {
                host_http_reject_pending(ctx, index, "Could not encode Host Control HTTP response.");
            } else {
                JSValue result = JS_NewString(ctx, json);
                JSValue ret = JS_Call(ctx, pending->resolve, JS_UNDEFINED, 1, &result);
                if (JS_IsException(ret)) JS_GetException(ctx);
                JS_FreeValue(ctx, ret);
                JS_FreeValue(ctx, result);
                host_http_free_pending(ctx, index);
            }
            free(json);
        }
        free(done.body);
    }

    while (JS_ExecutePendingJob(rt, NULL) > 0) {}
}

void host_control_shutdown(JSContext *ctx)
{
    if (!g_host_http.worker_started) return;

    host_http_ensure_mutex();
    vd_mutex_lock(g_host_http.mutex);
    g_host_http.shutdown = true;
    vd_mutex_unlock(g_host_http.mutex);

    vd_thread_join(g_host_http.worker);
    vd_thread_destroy(g_host_http.worker);
    g_host_http.worker = NULL;
    g_host_http.worker_started = false;

    HostHttpDone done;
    while (host_http_pop_done(&done)) free(done.body);

    for (int i = 0; i < VD_HOST_HTTP_QUEUE_CAP; i++) {
        if (g_host_http.pending[i].in_use) {
            host_http_reject_pending(ctx, i, "Host Control HTTP shut down.");
        }
    }

    host_http_ensure_mutex();
    vd_mutex_lock(g_host_http.mutex);
    for (int i = 0; i < g_host_http.job_count; i++) {
        int idx = (g_host_http.job_head + i) % VD_HOST_HTTP_QUEUE_CAP;
        free(g_host_http.jobs[idx].url);
        free(g_host_http.jobs[idx].body);
    }
    g_host_http.job_count = 0;
    g_host_http.job_head = 0;
    g_host_http.job_tail = 0;
    vd_mutex_unlock(g_host_http.mutex);
}

JSValue native_host_control_fetch(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 3) {
        return JS_ThrowTypeError(ctx, "nativeHostControlFetch requires url, body, timeoutMs");
    }

    const char *url = JS_ToCString(ctx, argv[0]);
    const char *body = JS_ToCString(ctx, argv[1]);
    int timeout_ms = 0;
    if (!url || !body || JS_ToInt32(ctx, &timeout_ms, argv[2]) != 0) {
        if (url) JS_FreeCString(ctx, url);
        if (body) JS_FreeCString(ctx, body);
        return JS_EXCEPTION;
    }

    JSValue resolving_funcs = JS_UNDEFINED;
    JSValue promise = JS_NewPromiseCapability(ctx, &resolving_funcs);
    if (JS_IsException(promise)) {
        JS_FreeCString(ctx, url);
        JS_FreeCString(ctx, body);
        return promise;
    }

    JSValue resolve = JS_GetPropertyStr(ctx, resolving_funcs, "resolve");
    JSValue reject = JS_GetPropertyStr(ctx, resolving_funcs, "reject");
    JS_FreeValue(ctx, resolving_funcs);

    if (!host_http_ensure_worker()) {
        JSValue err = JS_NewString(ctx, "Could not start Host Control HTTP worker.");
        JS_Call(ctx, reject, JS_UNDEFINED, 1, &err);
        JS_FreeValue(ctx, err);
        JS_FreeValue(ctx, resolve);
        JS_FreeValue(ctx, reject);
        JS_FreeValue(ctx, promise);
        JS_FreeCString(ctx, url);
        JS_FreeCString(ctx, body);
        return promise;
    }

    int request_id = 0;
    int pending_index = host_http_alloc_pending(ctx, resolve, reject, &request_id);

    if (pending_index < 0) {
        JSValue err = JS_NewString(ctx, "Host Control HTTP request queue is full.");
        JSValue ret = JS_Call(ctx, reject, JS_UNDEFINED, 1, &err);
        if (JS_IsException(ret)) JS_GetException(ctx);
        JS_FreeValue(ctx, ret);
        JS_FreeValue(ctx, err);
        JS_FreeValue(ctx, resolve);
        JS_FreeValue(ctx, reject);
        JS_FreeValue(ctx, promise);
        JS_FreeCString(ctx, url);
        JS_FreeCString(ctx, body);
        return promise;
    }

    JS_FreeValue(ctx, resolve);
    JS_FreeValue(ctx, reject);

    HostHttpJob job = {
        .request_id = request_id,
        .url = strdup(url),
        .body = strdup(body),
        .timeout_ms = timeout_ms > 0 ? timeout_ms : 5000,
    };
    JS_FreeCString(ctx, url);
    JS_FreeCString(ctx, body);

    if (!job.url || !job.body || !host_http_push_job(&job)) {
        if (job.url) free(job.url);
        if (job.body) free(job.body);
        host_http_reject_pending(ctx, pending_index, "Host Control HTTP request queue is full.");
        return promise;
    }

    TraceLog(LOG_INFO, "Host Control HTTP: %s", job.url);
    return promise;
}

void host_control_init(void)
{
    memset(&g_host_http, 0, sizeof(g_host_http));
    g_host_http.next_request_id = 1;
    host_control_link_load();
}

JSValue native_get_host_control_base_url(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    (void)argc;
    (void)argv;
    char url[512];
    if (!host_control_link_get_callback_url(url, sizeof(url))) {
        return JS_NewString(ctx, "");
    }
    return JS_NewString(ctx, url);
}
