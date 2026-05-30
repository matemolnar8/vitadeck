/*
	Minimal fetch() backed by libcurl. Each request runs on its own worker
	thread so the JS event loop never blocks; completed requests are polled by
	run_fetch() on the JS thread, which resolves/rejects the JS Promise.
*/
#include <ctype.h>
#include <curl/curl.h>
#include "platform/thread.h"

static char *fetch_strdup(const char *s) {
	if (!s) s = "";
	size_t len = strlen(s);
	char *copy = malloc(len + 1);
	if (copy) memcpy(copy, s, len + 1);
	return copy;
}

typedef struct {
	char *data;
	size_t len;
	size_t cap;
} FetchBuffer;

typedef struct {
	char *url;
	char *method;
	char *body;
	size_t body_len;
	struct curl_slist *headers;

	FetchBuffer resp_body;
	FetchBuffer resp_headers;
	char *status_line;
	long status;

	bool failed;
	char error[CURL_ERROR_SIZE];

	bool done;
	vd_thread *thread;

	JSValue resolve;
	JSValue reject;
} FetchRequest;

static FetchRequest **fetch_pending = NULL;
static vd_mutex *fetch_mutex = NULL;

static void buffer_append(FetchBuffer *buf, const char *data, size_t len) {
	if (buf->len + len + 1 > buf->cap) {
		size_t new_cap = buf->cap ? buf->cap * 2 : 1024;
		while (new_cap < buf->len + len + 1) new_cap *= 2;
		char *grown = realloc(buf->data, new_cap);
		if (!grown) return;
		buf->data = grown;
		buf->cap = new_cap;
	}
	memcpy(buf->data + buf->len, data, len);
	buf->len += len;
	buf->data[buf->len] = '\0';
}

static size_t fetch_write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
	FetchRequest *req = userdata;
	size_t total = size * nmemb;
	buffer_append(&req->resp_body, ptr, total);
	return total;
}

static size_t fetch_header_cb(char *buffer, size_t size, size_t nitems, void *userdata) {
	FetchRequest *req = userdata;
	size_t total = size * nitems;

	if (total >= 5 && strncmp(buffer, "HTTP/", 5) == 0) {
		// New status line (could be a redirect): drop headers gathered so far
		// so we only keep the final response's headers.
		req->resp_headers.len = 0;
		if (req->resp_headers.data) req->resp_headers.data[0] = '\0';

		size_t n = total;
		while (n > 0 && (buffer[n - 1] == '\r' || buffer[n - 1] == '\n')) n--;
		free(req->status_line);
		req->status_line = malloc(n + 1);
		if (req->status_line) {
			memcpy(req->status_line, buffer, n);
			req->status_line[n] = '\0';
		}
	} else {
		buffer_append(&req->resp_headers, buffer, total);
	}
	return total;
}

static void *fetch_worker(void *arg) {
	FetchRequest *req = arg;
	CURL *curl = curl_easy_init();
	if (!curl) {
		req->failed = true;
		snprintf(req->error, sizeof(req->error), "Failed to initialize curl");
	} else {
		curl_easy_setopt(curl, CURLOPT_URL, req->url);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req->method);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "vitadeck/1.0");
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, req->error);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fetch_write_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, req);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, fetch_header_cb);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, req);
		if (req->headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, req->headers);
		if (req->body) {
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->body);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)req->body_len);
		}

		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			req->failed = true;
			if (req->error[0] == '\0') {
				snprintf(req->error, sizeof(req->error), "%s", curl_easy_strerror(res));
			}
		} else {
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &req->status);
		}
		curl_easy_cleanup(curl);
	}

	vd_mutex_lock(fetch_mutex);
	req->done = true;
	vd_mutex_unlock(fetch_mutex);
	return NULL;
}

static JSValue build_headers_object(JSContext *ctx, const FetchRequest *req) {
	JSValue headers = JS_NewObject(ctx);
	const char *p = req->resp_headers.data;
	if (!p) return headers;

	while (*p) {
		const char *line_end = strpbrk(p, "\r\n");
		if (!line_end) line_end = p + strlen(p);
		size_t line_len = (size_t)(line_end - p);

		const char *colon = memchr(p, ':', line_len);
		if (colon) {
			size_t name_len = (size_t)(colon - p);
			const char *value = colon + 1;
			while (value < line_end && (*value == ' ' || *value == '\t')) value++;
			size_t value_len = (size_t)(line_end - value);

			char *name = malloc(name_len + 1);
			if (name) {
				for (size_t i = 0; i < name_len; i++) name[i] = (char)tolower((unsigned char)p[i]);
				name[name_len] = '\0';
				JS_SetPropertyStr(ctx, headers, name, JS_NewStringLen(ctx, value, value_len));
				free(name);
			}
		}

		p = line_end;
		while (*p == '\r' || *p == '\n') p++;
	}
	return headers;
}

static const char *status_text_from_line(const char *status_line) {
	if (!status_line) return "";
	const char *p = strchr(status_line, ' ');
	if (!p) return "";
	p++;
	while (*p && *p != ' ') p++; // skip status code
	while (*p == ' ') p++;
	return p;
}

static void resolve_fetch(JSContext *ctx, FetchRequest *req) {
	JSValue arg;
	JSValue fn;

	if (req->failed) {
		arg = JS_NewError(ctx);
		JS_SetPropertyStr(ctx, arg, "message", JS_NewString(ctx, req->error[0] ? req->error : "fetch failed"));
		fn = req->reject;
	} else {
		JSValue resp = JS_NewObject(ctx);
		JS_SetPropertyStr(ctx, resp, "status", JS_NewInt32(ctx, (int)req->status));
		JS_SetPropertyStr(ctx, resp, "ok", JS_NewBool(ctx, req->status >= 200 && req->status < 300));
		JS_SetPropertyStr(ctx, resp, "statusText", JS_NewString(ctx, status_text_from_line(req->status_line)));
		JS_SetPropertyStr(ctx, resp, "headers", build_headers_object(ctx, req));
		if (req->resp_body.data) {
			JS_SetPropertyStr(ctx, resp, "body", JS_NewStringLen(ctx, req->resp_body.data, req->resp_body.len));
		} else {
			JS_SetPropertyStr(ctx, resp, "body", JS_NewString(ctx, ""));
		}
		arg = resp;
		fn = req->resolve;
	}

	JSValue result = JS_Call(ctx, fn, JS_UNDEFINED, 1, &arg);
	if (JS_IsException(result)) {
		JSValue exc = JS_GetException(ctx);
		const char *str = JS_ToCString(ctx, exc);
		TraceLog(LOG_ERROR, "[fetch] error settling promise: %s", str ? str : "unknown");
		JS_FreeCString(ctx, str);
		JS_FreeValue(ctx, exc);
	}
	JS_FreeValue(ctx, result);
	JS_FreeValue(ctx, arg);
}

static void free_fetch_request(JSContext *ctx, FetchRequest *req) {
	JS_FreeValue(ctx, req->resolve);
	JS_FreeValue(ctx, req->reject);
	free(req->url);
	free(req->method);
	free(req->body);
	free(req->status_line);
	free(req->resp_body.data);
	free(req->resp_headers.data);
	if (req->headers) curl_slist_free_all(req->headers);
	free(req);
}

static JSValue js_native_fetch(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	(void)this_val;
	if (argc < 2) return JS_ThrowTypeError(ctx, "nativeFetch requires a url and method");

	JSValue promise_funcs[2];
	JSValue promise = JS_NewPromiseCapability(ctx, promise_funcs);
	if (JS_IsException(promise)) return promise;

	FetchRequest *req = calloc(1, sizeof(FetchRequest));
	if (!req) {
		JS_FreeValue(ctx, promise_funcs[0]);
		JS_FreeValue(ctx, promise_funcs[1]);
		return JS_ThrowOutOfMemory(ctx);
	}
	req->status_line = NULL;
	req->resolve = promise_funcs[0];
	req->reject = promise_funcs[1];

	const char *url = JS_ToCString(ctx, argv[0]);
	const char *method = JS_ToCString(ctx, argv[1]);
	req->url = fetch_strdup(url ? url : "");
	req->method = fetch_strdup(method ? method : "GET");
	if (url) JS_FreeCString(ctx, url);
	if (method) JS_FreeCString(ctx, method);

	if (argc >= 3 && JS_IsArray(ctx, argv[2])) {
		uint32_t len = 0;
		JSValue len_val = JS_GetPropertyStr(ctx, argv[2], "length");
		JS_ToUint32(ctx, &len, len_val);
		JS_FreeValue(ctx, len_val);
		for (uint32_t i = 0; i < len; i++) {
			JSValue item = JS_GetPropertyUint32(ctx, argv[2], i);
			const char *header = JS_ToCString(ctx, item);
			if (header) {
				req->headers = curl_slist_append(req->headers, header);
				JS_FreeCString(ctx, header);
			}
			JS_FreeValue(ctx, item);
		}
	}

	if (argc >= 4 && JS_IsString(argv[3])) {
		size_t body_len = 0;
		const char *body = JS_ToCStringLen(ctx, &body_len, argv[3]);
		if (body) {
			req->body = malloc(body_len + 1);
			if (req->body) {
				memcpy(req->body, body, body_len);
				req->body[body_len] = '\0';
				req->body_len = body_len;
			}
			JS_FreeCString(ctx, body);
		}
	}

	req->thread = vd_thread_create(fetch_worker, req);
	if (!req->thread) {
		JSValue err = JS_NewError(ctx);
		JS_SetPropertyStr(ctx, err, "message", JS_NewString(ctx, "Failed to start fetch thread"));
		JSValue r = JS_Call(ctx, req->reject, JS_UNDEFINED, 1, &err);
		JS_FreeValue(ctx, r);
		JS_FreeValue(ctx, err);
		free_fetch_request(ctx, req);
		return promise;
	}

	vd_mutex_lock(fetch_mutex);
	arrput(fetch_pending, req);
	vd_mutex_unlock(fetch_mutex);

	return promise;
}

void run_fetch(JSContext *ctx) {
	if (!fetch_pending) return;

	FetchRequest **finished = NULL;
	vd_mutex_lock(fetch_mutex);
	for (int i = (int)arrlen(fetch_pending) - 1; i >= 0; i--) {
		if (fetch_pending[i]->done) {
			arrput(finished, fetch_pending[i]);
			arrdel(fetch_pending, i);
		}
	}
	vd_mutex_unlock(fetch_mutex);

	for (size_t i = 0; i < arrlen(finished); i++) {
		FetchRequest *req = finished[i];
		vd_thread_join(req->thread);
		vd_thread_destroy(req->thread);
		resolve_fetch(ctx, req);
		free_fetch_request(ctx, req);
	}
	arrfree(finished);
}

void fetch_shutdown(JSContext *ctx) {
	if (!fetch_pending) return;
	for (size_t i = 0; i < arrlen(fetch_pending); i++) {
		FetchRequest *req = fetch_pending[i];
		vd_thread_join(req->thread);
		vd_thread_destroy(req->thread);
		free_fetch_request(ctx, req);
	}
	arrfree(fetch_pending);
	fetch_pending = NULL;
}

void register_js_fetch(JSContext *ctx) {
	static bool curl_initialized = false;
	if (!curl_initialized) {
		curl_global_init(CURL_GLOBAL_DEFAULT);
		curl_initialized = true;
	}
	if (!fetch_mutex) fetch_mutex = vd_mutex_create();

	js_set_global_function(ctx, "nativeFetch", js_native_fetch, 4);
}
