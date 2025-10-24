/*
	Log helpers wired to raylib TraceLog. 
	Joins all args with spaces, converts them to "repr" (string representation).
*/
static void log_with_level(js_State *J, int level)
{
	int top = js_gettop(J);
	char buffer[2048];
	int pos = 0;

	buffer[0] = '\0';

	for (int i = 1; i < top; i++) {
		const char *s;

		if(js_isstring(J, i)) {
			s = js_tostring(J, i);
		} else {
			s = js_torepr(J, i);
		}
		
		if (s == NULL) s = "";

		if (i > 1) {
			if (pos < sizeof(buffer) - 2) {
				buffer[pos++] = ' ';
				buffer[pos] = '\0';
			}
		}

		int written = snprintf(buffer + pos, sizeof(buffer) - pos, "%s", s);
		if (written < 0) {
			break;
		}
		pos += written;
		if (pos >= sizeof(buffer) - 1) {
			pos = sizeof(buffer) - 1;
			break;
		}
	}

	buffer[pos] = '\0';
	TraceLog(level, "%s", buffer);
	js_pushundefined(J);
}

static void logInfo(js_State *J) { log_with_level(J, LOG_INFO); }
static void logDebug(js_State *J) { log_with_level(J, LOG_DEBUG); }
static void logWarn(js_State *J) { log_with_level(J, LOG_WARNING); }
static void logError(js_State *J) { log_with_level(J, LOG_ERROR); }

static const char *console_js =
	"var console = { log: logInfo, info: logInfo, debug: logDebug, warn: logWarn, error: logError };";

void register_js_log(js_State *J) {
    js_newcfunction(J, logInfo, "logInfo", 0);
	js_setglobal(J, "logInfo");

	js_newcfunction(J, logDebug, "logDebug", 0);
	js_setglobal(J, "logDebug");

	js_newcfunction(J, logWarn, "logWarn", 0);
	js_setglobal(J, "logWarn");

	js_newcfunction(J, logError, "logError", 0);
	js_setglobal(J, "logError");

    js_dostring(J, console_js);
}