#include "core/package_library.h"
#include "core/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "package_library_harness: %s\n", message);
    failures++;
}

static void write_package(const char *package_path, const char *display_name)
{
    char manifest[256];
    snprintf(manifest, sizeof(manifest),
             "{\"schemaVersion\":1,\"name\":\"%s\",\"version\":\"1.0.0\",\"entry\":\"app.js\"}", display_name);
    char path[VD_PATH_MAX];
    vd_join_path(path, sizeof(path), package_path, "manifest.json");
    if (!vd_mkdir_p(package_path) || !vd_write_text_file(path, manifest)) expect(false, "could not write manifest");
    vd_join_path(path, sizeof(path), package_path, "app.js");
    if (!vd_write_text_file(path, "globalThis.vitadeckPackage.register(() => null);\n")) {
        expect(false, "could not write app.js");
    }
}

static void publish(const char *package_name, const char *display_name)
{
    char staged[VD_PATH_MAX];
    vd_join_path(staged, sizeof(staged), package_library_staging_root(), package_name);
    write_package(staged, display_name);
    char error[256] = "";
    if (!package_library_publish_package(staged, package_name, NULL, NULL, error, sizeof(error))) expect(false, error);
}

static int list(VdPackageInfo *items)
{
    return package_library_list(items, VD_PACKAGE_LIST_MAX);
}

int main(void)
{
    char root[] = "/tmp/vitadeck-package-library-XXXXXX";
    if (!mkdtemp(root) || setenv("VITADECK_DATA_ROOT", root, 1) != 0) {
        fprintf(stderr, "package_library_harness: could not create temp data root\n");
        return 1;
    }
    char error[256] = "";
    if (!package_library_init(error, sizeof(error))) {
        fprintf(stderr, "package_library_harness: %s\n", error);
        return 1;
    }

    VdPackageInfo items[VD_PACKAGE_LIST_MAX];
    expect(list(items) == 0, "empty library should list nothing");

    publish("a.vdapp", "Alpha");
    expect(list(items) == 1 && items[0].is_active, "publish should refresh the list and activate the first package");

    publish("b.vdapp", "Beta");
    expect(list(items) == 2, "second publish should refresh the list");

    expect(package_library_set_active("b.vdapp", error, sizeof(error)), "set active failed");
    expect(list(items) == 2 && !items[0].is_active && items[1].is_active, "set active should move is_active");

    expect(package_library_set_active("b.vdapp", error, sizeof(error)), "re-activating the active package failed");
    expect(list(items) == 2 && items[1].is_active, "re-activating should keep the package active");

    expect(package_library_remove("a.vdapp", error, sizeof(error)), "remove failed");
    expect(list(items) == 1 && strcmp(items[0].package_name, "b.vdapp") == 0, "remove should refresh the list");

    char outside[VD_PATH_MAX];
    char installed[VD_PATH_MAX];
    vd_join_path(installed, sizeof(installed), root, "installed-deck-apps");
    vd_join_path(outside, sizeof(outside), installed, "c.vdapp");
    write_package(outside, "Gamma");
    expect(list(items) == 1, "changes outside the library should wait for a refresh");
    package_library_refresh();
    expect(list(items) == 2, "refresh should rescan the installed packages");

    vd_remove_tree(root);

    if (failures > 0) {
        fprintf(stderr, "package_library_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("package_library_harness: ok\n");
    return 0;
}
