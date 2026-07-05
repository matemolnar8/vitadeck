#include "core/package_manifest.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures = 0;

static void failf(const char *case_name, const char *message)
{
    fprintf(stderr, "package_manifest_harness: %s: %s\n", case_name, message);
    failures++;
}

static void join_path(char *out, size_t out_size, const char *a, const char *b)
{
    snprintf(out, out_size, "%s/%s", a, b);
}

static bool mkdir_p(const char *path)
{
    char tmp[VD_PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len == 0) return false;
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; p++) {
        if (*p != '/') continue;
        *p = '\0';
        if (mkdir(tmp, 0777) != 0 && errno != EEXIST) return false;
        *p = '/';
    }
    return mkdir(tmp, 0777) == 0 || errno == EEXIST;
}

static bool write_text_file(const char *path, const char *contents)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = fwrite(contents, 1, strlen(contents), f) == strlen(contents);
    fclose(f);
    return ok;
}

static bool write_package_file(const char *package_path, const char *rel_path, const char *contents)
{
    char path[VD_PATH_MAX];
    join_path(path, sizeof(path), package_path, rel_path);
    char parent[VD_PATH_MAX];
    snprintf(parent, sizeof(parent), "%s", path);
    char *slash = strrchr(parent, '/');
    if (slash) {
        *slash = '\0';
        if (!mkdir_p(parent)) return false;
    }
    return write_text_file(path, contents);
}

static bool remove_tree(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir) return unlink(path) == 0 || errno == ENOENT;

    struct dirent *entry;
    bool ok = true;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char child[VD_PATH_MAX];
        join_path(child, sizeof(child), path, entry->d_name);
        if (!remove_tree(child)) {
            ok = false;
            break;
        }
    }
    closedir(dir);
    return ok && (rmdir(path) == 0 || errno == ENOENT);
}

static bool make_package_root(char *root, size_t root_size, char *package_path, size_t package_path_size)
{
    char template_path[] = "/tmp/vitadeck-package-manifest-XXXXXX";
    char *created = mkdtemp(template_path);
    if (!created) return false;
    snprintf(root, root_size, "%s", created);
    join_path(package_path, package_path_size, root, "sample.vdapp");
    return mkdir_p(package_path);
}

static void write_minimal_valid_package(const char *package_path)
{
    const char *manifest = "{"
                           "\"schemaVersion\":1,"
                           "\"name\":\"Sample\","
                           "\"version\":\"1.2.3\","
                           "\"entry\":\"app.js\""
                           "}";
    write_package_file(package_path, "manifest.json", manifest);
    write_package_file(package_path, "app.js", "globalThis.vitadeckPackage.register(() => null);\n");
}

static void expect_success_minimal(void)
{
    const char *case_name = "valid minimal manifest";
    char root[VD_PATH_MAX];
    char package_path[VD_PATH_MAX];
    if (!make_package_root(root, sizeof(root), package_path, sizeof(package_path))) {
        failf(case_name, "could not create temp package");
        return;
    }
    write_minimal_valid_package(package_path);

    char error[256] = "";
    VdPackageManifest manifest;
    if (!package_manifest_read(package_path, "sample.vdapp", &manifest, error, sizeof(error))) {
        failf(case_name, error);
    } else {
        if (strcmp(manifest.package_name, "sample.vdapp") != 0) failf(case_name, "package name mismatch");
        if (strcmp(manifest.display_name, "Sample") != 0) failf(case_name, "display name mismatch");
        if (strcmp(manifest.version, "1.2.3") != 0) failf(case_name, "version mismatch");
        if (strcmp(manifest.entry, "app.js") != 0) failf(case_name, "entry mismatch");
        if (manifest.font_count != 0) failf(case_name, "unexpected fonts");
        if (manifest.image_count != 0) failf(case_name, "unexpected images");
    }

    remove_tree(root);
}

static void expect_success_assets(void)
{
    const char *case_name = "valid manifest with fonts and images";
    char root[VD_PATH_MAX];
    char package_path[VD_PATH_MAX];
    if (!make_package_root(root, sizeof(root), package_path, sizeof(package_path))) {
        failf(case_name, "could not create temp package");
        return;
    }
    const char *manifest = "{"
                           "\"schemaVersion\":1,"
                           "\"name\":\"Assets\","
                           "\"version\":\"2.0.0\","
                           "\"entry\":\"app.js\","
                           "\"fonts\":{\"body\":\"fonts/body.ttf\"},"
                           "\"images\":{\"logo\":\"images/logo.png\"}"
                           "}";
    write_package_file(package_path, "manifest.json", manifest);
    write_package_file(package_path, "app.js", "app\n");
    write_package_file(package_path, "fonts/body.ttf", "font\n");
    write_package_file(package_path, "images/logo.png", "image\n");

    char error[256] = "";
    VdPackageManifest parsed;
    if (!package_manifest_read(package_path, "sample.vdapp", &parsed, error, sizeof(error))) {
        failf(case_name, error);
    } else {
        if (parsed.font_count != 1 || strcmp(parsed.fonts[0].name, "body") != 0 ||
            strcmp(parsed.fonts[0].path, "fonts/body.ttf") != 0) {
            failf(case_name, "font declaration mismatch");
        }
        if (parsed.image_count != 1 || strcmp(parsed.images[0].name, "logo") != 0 ||
            strcmp(parsed.images[0].path, "images/logo.png") != 0) {
            failf(case_name, "image declaration mismatch");
        }
    }

    remove_tree(root);
}

static void expect_failure(const char *case_name, const char *manifest, bool write_entry, const char *expected_error)
{
    char root[VD_PATH_MAX];
    char package_path[VD_PATH_MAX];
    if (!make_package_root(root, sizeof(root), package_path, sizeof(package_path))) {
        failf(case_name, "could not create temp package");
        return;
    }
    if (manifest) write_package_file(package_path, "manifest.json", manifest);
    if (write_entry) write_package_file(package_path, "app.js", "app\n");

    char error[256] = "";
    VdPackageManifest parsed;
    bool ok = package_manifest_read(package_path, "sample.vdapp", &parsed, error, sizeof(error));
    if (ok) {
        failf(case_name, "expected failure");
    } else if (strcmp(error, expected_error) != 0) {
        char message[512];
        snprintf(message, sizeof(message), "expected '%s', got '%s'", expected_error, error);
        failf(case_name, message);
    }

    remove_tree(root);
}

static void expect_missing_file_failure(const char *case_name, const char *manifest, const char *expected_error)
{
    expect_failure(case_name, manifest, true, expected_error);
}

int main(void)
{
    expect_success_minimal();
    expect_success_assets();

    expect_failure("missing manifest", NULL, true, "Deck App Package Manifest is missing.");
    expect_failure("wrong schema version",
                   "{\"schemaVersion\":2,\"name\":\"Sample\",\"version\":\"1.2.3\",\"entry\":\"app.js\"}", true,
                   "Deck App Package Manifest is invalid.");
    expect_failure("non app entry",
                   "{\"schemaVersion\":1,\"name\":\"Sample\",\"version\":\"1.2.3\",\"entry\":\"main.js\"}", true,
                   "Deck App Package Manifest is invalid.");
    expect_failure("invalid version",
                   "{\"schemaVersion\":1,\"name\":\"Sample\",\"version\":\"1.2\",\"entry\":\"app.js\"}", true,
                   "Deck App Package Manifest is invalid.");
    expect_failure("missing entry",
                   "{\"schemaVersion\":1,\"name\":\"Sample\",\"version\":\"1.2.3\",\"entry\":\"app.js\"}", false,
                   "Deck App Package Entry is missing.");
    expect_missing_file_failure("reserved font name",
                                "{\"schemaVersion\":1,\"name\":\"Sample\",\"version\":\"1.2.3\",\"entry\":\"app.js\","
                                "\"fonts\":{\"default\":\"fonts/body.ttf\"}}",
                                "Deck App Package declares an invalid font name.");
    expect_missing_file_failure("invalid image name",
                                "{\"schemaVersion\":1,\"name\":\"Sample\",\"version\":\"1.2.3\",\"entry\":\"app.js\","
                                "\"images\":{\"1logo\":\"images/logo.png\"}}",
                                "Deck App Package declares an invalid image name.");
    expect_missing_file_failure("unsafe asset path",
                                "{\"schemaVersion\":1,\"name\":\"Sample\",\"version\":\"1.2.3\",\"entry\":\"app.js\","
                                "\"fonts\":{\"body\":\"../body.ttf\"}}",
                                "Deck App Package declares an invalid font path.");
    expect_missing_file_failure("unsupported font extension",
                                "{\"schemaVersion\":1,\"name\":\"Sample\",\"version\":\"1.2.3\",\"entry\":\"app.js\","
                                "\"fonts\":{\"body\":\"fonts/body.txt\"}}",
                                "Deck App Package declares an invalid font path.");
    expect_missing_file_failure("missing declared image",
                                "{\"schemaVersion\":1,\"name\":\"Sample\",\"version\":\"1.2.3\",\"entry\":\"app.js\","
                                "\"images\":{\"logo\":\"images/logo.png\"}}",
                                "Deck App Package Image is missing.");

    return failures == 0 ? 0 : 1;
}
