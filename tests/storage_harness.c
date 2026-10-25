#include "core/deck_app_storage.h"
#include "core/package_library.h"
#include "core/util.h"
#include "jslib/jslib_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures = 0;
static char root[] = "/tmp/vitadeck-storage-XXXXXX";

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "storage_harness: %s\n", message);
    failures++;
}

static bool exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static void storage_path(char *out, size_t out_size, const char *package_name, const char *suffix)
{
    snprintf(out, out_size, "%s/deck-app-data/%s/storage.json%s", root, package_name, suffix);
}

static bool stored_equals(const char *package_name, const char *expected)
{
    size_t len = 0;
    char *json = deck_app_storage_read(root, package_name, &len);
    bool ok = json && len == strlen(expected) && strcmp(json, expected) == 0;
    free(json);
    return ok;
}

static bool write_json(const char *package_name, const char *json)
{
    char error[128] = "";
    return deck_app_storage_write(root, package_name, json, strlen(json), error, sizeof(error));
}

static void test_round_trip(void)
{
    expect(stored_equals("a.vdapp", "{}"), "an unwritten store should read as {}");
    expect(write_json("a.vdapp", "{\"best\":\"3\"}"), "first write failed");
    expect(stored_equals("a.vdapp", "{\"best\":\"3\"}"), "first write should read back");
    expect(write_json("a.vdapp", "{\"best\":\"7\"}"), "replacing write failed");
    expect(stored_equals("a.vdapp", "{\"best\":\"7\"}"), "replacing write should read back");

    char path[VD_PATH_MAX];
    storage_path(path, sizeof(path), "a.vdapp", ".bak");
    expect(!exists(path), "a finished write should not leave a backup");
    storage_path(path, sizeof(path), "a.vdapp", ".tmp");
    expect(!exists(path), "a finished write should not leave a temp file");

    expect(stored_equals("b.vdapp", "{}"), "stores should be scoped per package");
    expect(!write_json("../a.vdapp", "{}"), "an unsafe package name should be rejected");
    expect(!deck_app_storage_read(root, "../a.vdapp", NULL), "an unsafe package name should not be read");
}

static void test_cap(void)
{
    char *big = malloc(VD_DECK_APP_STORAGE_MAX + 2);
    if (!big) {
        expect(false, "could not allocate the cap fixture");
        return;
    }
    memset(big, 'x', VD_DECK_APP_STORAGE_MAX + 1);
    big[VD_DECK_APP_STORAGE_MAX + 1] = '\0';

    char error[128] = "";
    expect(!deck_app_storage_write(root, "a.vdapp", big, VD_DECK_APP_STORAGE_MAX + 1, error, sizeof(error)),
           "a write past the cap should fail");
    expect(strstr(error, "1 MiB") != NULL, "the cap failure should name the limit");
    expect(stored_equals("a.vdapp", "{\"best\":\"7\"}"), "a rejected write should keep the previous store");

    expect(deck_app_storage_write(root, "c.vdapp", big, VD_DECK_APP_STORAGE_MAX, error, sizeof(error)),
           "a write at the cap should succeed");
    free(big);
}

static void test_crash_recovery(void)
{
    char file[VD_PATH_MAX];
    char temp[VD_PATH_MAX];
    char backup[VD_PATH_MAX];
    storage_path(file, sizeof(file), "a.vdapp", "");
    storage_path(temp, sizeof(temp), "a.vdapp", ".tmp");
    storage_path(backup, sizeof(backup), "a.vdapp", ".bak");

    // Crashed while writing the temp file.
    expect(vd_write_text_file(temp, "{\"best\":"), "could not write a partial temp file");
    expect(stored_equals("a.vdapp", "{\"best\":\"7\"}"), "a partial temp file should not replace the store");
    expect(write_json("a.vdapp", "{\"best\":\"8\"}"), "a write after a partial temp file failed");
    expect(stored_equals("a.vdapp", "{\"best\":\"8\"}"), "a write should replace a partial temp file");

    // Crashed after moving the old store aside, before the new one was in place.
    expect(rename(file, backup) == 0, "could not simulate a crash between renames");
    expect(vd_write_text_file(temp, "{\"best\":\"9\"}"), "could not write the pending temp file");
    expect(stored_equals("a.vdapp", "{\"best\":\"8\"}"), "the backup should be read when the store is missing");
    expect(write_json("a.vdapp", "{\"best\":\"10\"}"), "a write after a crash between renames failed");
    expect(stored_equals("a.vdapp", "{\"best\":\"10\"}"), "a write should recover from a crash between renames");
    expect(!exists(backup), "a recovered write should drop the backup");

    // Crashed after the new store was in place, before the backup was removed.
    expect(vd_write_text_file(backup, "{\"best\":\"old\"}"), "could not write a stale backup");
    expect(stored_equals("a.vdapp", "{\"best\":\"10\"}"), "a stale backup should not shadow the store");
    expect(write_json("a.vdapp", "{\"best\":\"11\"}"), "a write over a stale backup failed");
    expect(stored_equals("a.vdapp", "{\"best\":\"11\"}"), "a write over a stale backup should read back");
}

static bool eval_bool(JSContext *ctx, const char *source)
{
    JSValue result = JS_Eval(ctx, source, strlen(source), "storage-harness.js", JS_EVAL_TYPE_GLOBAL);
    bool ok = JS_ToBool(ctx, result) > 0;
    if (JS_IsException(result)) JS_FreeValue(ctx, JS_GetException(ctx));
    JS_FreeValue(ctx, result);
    return ok;
}

static bool eval_in_runtime(const char *source)
{
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = rt ? JS_NewContext(rt) : NULL;
    if (!ctx) {
        if (rt) JS_FreeRuntime(rt);
        return false;
    }
    register_js_storage(ctx);
    bool ok = eval_bool(ctx, source);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return ok;
}

static void publish(const char *package_name)
{
    char staged[VD_PATH_MAX];
    char path[VD_PATH_MAX];
    vd_join_path(staged, sizeof(staged), package_library_staging_root(), package_name);
    vd_join_path(path, sizeof(path), staged, "manifest.json");
    bool written = vd_mkdir_p(staged) &&
                   vd_write_text_file(path, "{\"schemaVersion\":1,\"name\":\"App\",\"version\":\"1.0.0\",\"entry\":\"app.js\"}");
    vd_join_path(path, sizeof(path), staged, "app.js");
    written = written && vd_write_text_file(path, "globalThis.vitadeckPackage.register(() => null);\n");
    char error[256] = "";
    expect(written && package_library_publish_package(staged, package_name, NULL, NULL, error, sizeof(error)),
           "could not publish a fixture package");
}

static void test_natives_and_library(void)
{
    char error[256] = "";
    if (setenv("VITADECK_DATA_ROOT", root, 1) != 0 || !package_library_init(error, sizeof(error))) {
        expect(false, "could not initialize the Installed Deck App Library");
        return;
    }
    publish("game.vdapp");

    expect(eval_in_runtime("nativeReadDeckAppStorage() === '{}'"), "the Active Deck App should start with {}");
    expect(eval_in_runtime("nativeWriteDeckAppStorage(JSON.stringify({ best: '42' })); true"), "native write failed");
    expect(eval_in_runtime("JSON.parse(nativeReadDeckAppStorage()).best === '42'"),
           "a write should be read back after a Deck App Runtime Restart");
    expect(eval_in_runtime("try { nativeWriteDeckAppStorage('x'.repeat(1024 * 1024 + 1)); false }"
                           " catch (e) { e instanceof RangeError }"),
           "a write past the cap should throw a RangeError");
    expect(eval_in_runtime("JSON.parse(nativeReadDeckAppStorage()).best === '42'"),
           "a write past the cap should keep the store");

    publish("game.vdapp");
    expect(stored_equals("game.vdapp", "{\"best\":\"42\"}"), "Runtime Upload Replacement should keep the store");

    publish("other.vdapp");
    expect(write_json("other.vdapp", "{\"a\":\"1\"}"), "could not write the removable store");
    expect(package_library_remove("other.vdapp", error, sizeof(error)), "could not remove a Deck App Package");
    char dir[VD_PATH_MAX];
    snprintf(dir, sizeof(dir), "%s/deck-app-data/other.vdapp", root);
    expect(!vd_is_dir(dir), "Installed Deck App Removal should delete the store");
    expect(stored_equals("game.vdapp", "{\"best\":\"42\"}"), "removing one package should keep the others");
}

int main(void)
{
    if (!mkdtemp(root)) {
        fprintf(stderr, "storage_harness: could not create temp data root\n");
        return 1;
    }
    test_round_trip();
    test_cap();
    test_crash_recovery();
    test_natives_and_library();
    vd_remove_tree(root);

    if (failures > 0) return 1;
    printf("storage_harness: ok\n");
    return 0;
}
