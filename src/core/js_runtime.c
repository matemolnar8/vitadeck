#include "js_runtime.h"

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include "quickjs.h"
#include "core/event_queue.h"
#include "jslib/jslib.h"
#include "net/host_control.h"
#include "platform/thread.h"
#include "ui/input.h"
#include "ui/instance_tree.h"

static int run_function(JSContext *ctx, const char *func_name)
{
	JSValue global = JS_GetGlobalObject(ctx);
	JSValue vitadeck = JS_GetPropertyStr(ctx, global, "vitadeck");
	JSValue func = JS_GetPropertyStr(ctx, vitadeck, func_name);

	JSValue result = JS_Call(ctx, func, vitadeck, 0, NULL);

	int ret = 0;
	if (JS_IsException(result)) {
		JSValue exc = JS_GetException(ctx);
		const char *str = JS_ToCString(ctx, exc);
		TraceLog(LOG_ERROR, "%s: an exception occurred in the javascript function", func_name);
		TraceLog(LOG_ERROR, "%s", str ? str : "unknown error");
		JS_FreeCString(ctx, str);
		JS_FreeValue(ctx, exc);
		ret = -1;
	}

	JS_FreeValue(ctx, result);
	JS_FreeValue(ctx, func);
	JS_FreeValue(ctx, vitadeck);
	JS_FreeValue(ctx, global);

	return ret;
}

static char *read_file(const char *filename, size_t *out_len) {
	FILE *f = fopen(filename, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc((size_t)len + 1);
	if (!buf) { fclose(f); return NULL; }
	fread(buf, 1, (size_t)len, f);
	buf[len] = '\0';
	fclose(f);
	if (out_len) *out_len = (size_t)len;
	return buf;
}

static void *js_thread_func(void *arg)
{
	VdJsRuntime *runtime = arg;
	JSRuntime *rt = NULL;
	JSContext *ctx = NULL;
	void *result = NULL;

	rt = JS_NewRuntime();
	if (!rt) {
		TraceLog(LOG_ERROR, "Could not initialize QuickJS runtime.");
		runtime->failed = true;
		return NULL;
	}

	ctx = JS_NewContext(rt);
	if (!ctx) {
		TraceLog(LOG_ERROR, "Could not initialize QuickJS context.");
		runtime->failed = true;
		goto defer;
	}

	register_js_lib(ctx);

	size_t len = 0;
	char *code = read_file("js/runtime.js", &len);
	if (!code) {
		TraceLog(LOG_ERROR, "Could not load runtime.js.");
		runtime->failed = true;
		goto defer;
	}

	JSValue eval_result = JS_Eval(ctx, code, len, "runtime.js", JS_EVAL_TYPE_GLOBAL);
	free(code);

	if (JS_IsException(eval_result)) {
		JSValue exc = JS_GetException(ctx);
		const char *str = JS_ToCString(ctx, exc);
		TraceLog(LOG_ERROR, "Error evaluating runtime.js: %s", str ? str : "unknown error");
		JS_FreeCString(ctx, str);
		JS_FreeValue(ctx, exc);
		JS_FreeValue(ctx, eval_result);
		runtime->failed = true;
		goto defer;
	}
	JS_FreeValue(ctx, eval_result);

	if (run_function(ctx, "updateContainer") != 0) {
		runtime->failed = true;
		goto defer;
	}

	instance_tree_swap();
	runtime->ready = true;

	while (!runtime->stop_requested && !event_queue_is_shutdown()) {
		process_input_events(ctx);
		host_control_drain_completions(ctx, rt);
		run_timeouts(ctx);
		instance_tree_swap();
		vd_thread_yield();
	}

defer:
	if (ctx) {
		host_control_shutdown(ctx);
		JS_FreeContext(ctx);
	}
	if (rt) JS_FreeRuntime(rt);
	return result;
}

void js_runtime_init(VdJsRuntime *runtime)
{
	runtime->thread = NULL;
	runtime->stop_requested = false;
	runtime->ready = false;
	runtime->failed = false;
}

bool js_runtime_start(VdJsRuntime *runtime)
{
	if (runtime->thread) return true;
	runtime->stop_requested = false;
	runtime->ready = false;
	runtime->failed = false;
	event_queue_clear();
	input_clear_focus();
	runtime->thread = vd_thread_create(js_thread_func, runtime);
	return runtime->thread != NULL;
}

void js_runtime_stop(VdJsRuntime *runtime)
{
	if (!runtime->thread) return;
	runtime->stop_requested = true;
	vd_thread_join((vd_thread *)runtime->thread);
	vd_thread_destroy((vd_thread *)runtime->thread);
	runtime->thread = NULL;
	runtime->ready = false;
	input_clear_focus();
	instance_tree_clear();
	event_queue_clear();
}

bool js_runtime_restart(VdJsRuntime *runtime)
{
	js_runtime_stop(runtime);
	return js_runtime_start(runtime);
}

bool js_runtime_is_ready(const VdJsRuntime *runtime)
{
	return runtime->ready;
}

bool js_runtime_failed(const VdJsRuntime *runtime)
{
	return runtime->failed;
}
