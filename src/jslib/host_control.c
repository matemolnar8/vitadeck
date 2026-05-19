#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quickjs.h"
#include "platform/thread.h"
#include "jslib_internal.h"

#define VD_HOST_HTTP_MAX_REQUESTS 16
#define VD_HOST_HTTP_MAX_BODY 65536

typedef struct {
    char *data;
    size_t size;
} HostHttpBuffer;

typedef struct {
    int request_id;
    char *url;
    char *body;
    long timeout_ms;
} HostHttpWorkerArg;

typedef struct {
    int request_id;
    vd_thread *thread;
    bool completed;
    long status;
    char *body;
    char error[256];
} HostHttpRequest;

static HostHttpRequest g_host_http_requests[VD_HOST_HTTP_MAX_REQUESTS];
static int g_host_http_request_count = 0;
static vd_mutex *g_host_http_mutex = NULL;
static bool g_curl_initialized = false;

static void host_http_ensure_mutex(void)
{
    if (!g_host_http_mutex) g_host_http_mutex = vd_mutex_create();
}

static size_t host_http_write(void *contents, size_t size, size_t nmemb, void *userp)
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

static int host_http_find_request(int request_id)
{
    for (int i = 0; i < g_host_http_request_count; i++) {
        if (g_host_http_requests[i].request_id == request_id) return i;
    }
    return -1;
}

static void host_http_complete(int request_id, long status, char *body, const char *error)
{
    host_http_ensure_mutex();
    vd_mutex_lock(g_host_http_mutex);
    int index = host_http_find_request(request_id);
    if (index >= 0) {
        HostHttpRequest *request = &g_host_http_requests[index];
        request->status = status;
        request->body = body;
        snprintf(request->error, sizeof(request->error), "%s", error ? error : "");
        request->completed = true;
    } else {
        free(body);
    }
    vd_mutex_unlock(g_host_http_mutex);
}

static void *host_http_worker(void *raw)
{
    HostHttpWorkerArg *arg = raw;
    HostHttpBuffer response = {0};
    CURL *curl = curl_easy_init();
    if (!curl) {
        host_http_complete(arg->request_id, 0, NULL, "Could not initialize HTTP client.");
        goto done;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, arg->url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, arg->body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(arg->body));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, host_http_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, arg->timeout_ms);

    CURLcode code = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

    if (code != CURLE_OK) {
        host_http_complete(arg->request_id, status, response.data, curl_easy_strerror(code));
    } else {
        host_http_complete(arg->request_id, status, response.data ? response.data : strdup(""), NULL);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

done:
    free(arg->url);
    free(arg->body);
    free(arg);
    return NULL;
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

static JSValue host_http_post_json(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 4) return JS_ThrowTypeError(ctx, "nativeHostControlHttpPostJson requires requestId, url, body, timeoutMs");

    int request_id = 0;
    int timeout_ms = 0;
    if (JS_ToInt32(ctx, &request_id, argv[0]) != 0 || JS_ToInt32(ctx, &timeout_ms, argv[3]) != 0) return JS_EXCEPTION;
    const char *url = JS_ToCString(ctx, argv[1]);
    const char *body = JS_ToCString(ctx, argv[2]);
    if (!url || !body) {
        if (url) JS_FreeCString(ctx, url);
        if (body) JS_FreeCString(ctx, body);
        return JS_EXCEPTION;
    }

    host_http_ensure_mutex();
    if (!g_curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        g_curl_initialized = true;
    }

    vd_mutex_lock(g_host_http_mutex);
    if (g_host_http_request_count >= VD_HOST_HTTP_MAX_REQUESTS || host_http_find_request(request_id) >= 0) {
        vd_mutex_unlock(g_host_http_mutex);
        JS_FreeCString(ctx, url);
        JS_FreeCString(ctx, body);
        return JS_ThrowInternalError(ctx, "Host Control HTTP request queue is full");
    }

    HostHttpRequest *request = &g_host_http_requests[g_host_http_request_count++];
    memset(request, 0, sizeof(*request));
    request->request_id = request_id;
    vd_mutex_unlock(g_host_http_mutex);

    HostHttpWorkerArg *arg = calloc(1, sizeof(*arg));
    if (!arg) {
        host_http_complete(request_id, 0, NULL, "Could not allocate Host Control request.");
        JS_FreeCString(ctx, url);
        JS_FreeCString(ctx, body);
        return JS_UNDEFINED;
    }
    arg->request_id = request_id;
    arg->url = strdup(url);
    arg->body = strdup(body);
    arg->timeout_ms = timeout_ms > 0 ? timeout_ms : 5000;
    JS_FreeCString(ctx, url);
    JS_FreeCString(ctx, body);

    vd_thread *thread = vd_thread_create(host_http_worker, arg);
    if (!thread) {
        free(arg->url);
        free(arg->body);
        free(arg);
        host_http_complete(request_id, 0, NULL, "Could not start Host Control HTTP worker.");
        return JS_UNDEFINED;
    }

    vd_mutex_lock(g_host_http_mutex);
    int index = host_http_find_request(request_id);
    if (index >= 0) g_host_http_requests[index].thread = thread;
    vd_mutex_unlock(g_host_http_mutex);
    return JS_UNDEFINED;
}

static JSValue host_http_take_response(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    (void)this_val;
    if (argc < 1) return JS_ThrowTypeError(ctx, "nativeTakeHostControlHttpResponse requires requestId");
    int request_id = 0;
    if (JS_ToInt32(ctx, &request_id, argv[0]) != 0) return JS_EXCEPTION;

    host_http_ensure_mutex();
    vd_mutex_lock(g_host_http_mutex);
    int index = host_http_find_request(request_id);
    if (index < 0 || !g_host_http_requests[index].completed) {
        vd_mutex_unlock(g_host_http_mutex);
        return JS_NULL;
    }

    HostHttpRequest request = g_host_http_requests[index];
    g_host_http_requests[index] = g_host_http_requests[--g_host_http_request_count];
    vd_mutex_unlock(g_host_http_mutex);

    if (request.thread) {
        vd_thread_join(request.thread);
        vd_thread_destroy(request.thread);
    }

    char *body = host_http_json_escape(request.body);
    char *error = host_http_json_escape(request.error);
    free(request.body);
    if (!body || !error) {
        free(body);
        free(error);
        return JS_ThrowInternalError(ctx, "Could not encode Host Control HTTP response");
    }

    size_t json_size = strlen(body) + strlen(error) + 96;
    char *json = malloc(json_size);
    if (!json) {
        free(body);
        free(error);
        return JS_ThrowOutOfMemory(ctx);
    }
    snprintf(json, json_size, "{\"status\":%ld,\"body\":\"%s\",\"error\":\"%s\"}", request.status, body, error);
    JSValue result = JS_NewString(ctx, json);
    free(json);
    free(body);
    free(error);
    return result;
}

void register_js_host_control(JSContext *ctx)
{
    js_set_global_function(ctx, "nativeHostControlHttpPostJson", host_http_post_json, 4);
    js_set_global_function(ctx, "nativeTakeHostControlHttpResponse", host_http_take_response, 1);
}
