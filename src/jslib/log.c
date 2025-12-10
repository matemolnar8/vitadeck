/*
	Log helpers wired to raylib TraceLog. 
	Joins all args with spaces, converts them to string representation.
*/
static JSValue log_with_level(JSContext *ctx, int level, int argc, JSValueConst *argv)
{
	char buffer[2048];
	int pos = 0;
	buffer[0] = '\0';

	for (int i = 0; i < argc; i++) {
		const char *s = JS_ToCString(ctx, argv[i]);
		if (s == NULL) s = "";

		if (i > 0) {
			if (pos < (int)sizeof(buffer) - 2) {
				buffer[pos++] = ' ';
				buffer[pos] = '\0';
			}
		}

		int written = snprintf(buffer + pos, sizeof(buffer) - pos, "%s", s);
		JS_FreeCString(ctx, s);
		
		if (written < 0) break;
		pos += written;
		if (pos >= (int)sizeof(buffer) - 1) {
			pos = sizeof(buffer) - 1;
			break;
		}
	}

	buffer[pos] = '\0';
	TraceLog(level, "%s", buffer);
	return JS_UNDEFINED;
}

static JSValue js_logInfo(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	(void)this_val;
	return log_with_level(ctx, LOG_INFO, argc, argv);
}

static JSValue js_logDebug(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	(void)this_val;
	return log_with_level(ctx, LOG_DEBUG, argc, argv);
}

static JSValue js_logWarn(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	(void)this_val;
	return log_with_level(ctx, LOG_WARNING, argc, argv);
}

static JSValue js_logError(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	(void)this_val;
	return log_with_level(ctx, LOG_ERROR, argc, argv);
}

static const char *console_js =
	"var console = { log: logInfo, info: logInfo, debug: logDebug, warn: logWarn, error: logError };";

void register_js_log(JSContext *ctx) {
	js_set_global_function(ctx, "logInfo", js_logInfo, 0);
	js_set_global_function(ctx, "logDebug", js_logDebug, 0);
	js_set_global_function(ctx, "logWarn", js_logWarn, 0);
	js_set_global_function(ctx, "logError", js_logError, 0);

	JSValue result = JS_Eval(ctx, console_js, strlen(console_js), "<console>", JS_EVAL_TYPE_GLOBAL);
	JS_FreeValue(ctx, result);
}