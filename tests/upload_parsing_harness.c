#include "upload/archive.h"
#include "core/json.h"
#include "net/http.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static int failures = 0;
static char g_root[] = "/tmp/vitadeck-upload-harness-XXXXXX";

static void failf(const char *case_name, const char *message)
{
    fprintf(stderr, "upload_parsing_harness: %s: %s\n", case_name, message);
    failures++;
}

static void expect_header(const char *case_name, const char *headers, const char *name, const char *expected)
{
    char value[256];
    bool found = http_header_value(headers, name, value, sizeof(value));
    if (!expected) {
        if (found) failf(case_name, "header should not be found");
        return;
    }
    if (!found) {
        failf(case_name, "header not found");
    } else if (strcmp(value, expected) != 0) {
        failf(case_name, "header value mismatch");
    }
}

static void test_headers(void)
{
    const char *lowercase = "POST /upload HTTP/1.1\r\n"
                            "host: 127.0.0.1:8787\r\n"
                            "content-type: application/zip\r\n"
                            "content-length: 42\r\n"
                            "\r\n";
    expect_header("lowercase content-type", lowercase, "Content-Type", "application/zip");
    if (http_content_length(lowercase) != 42) failf("lowercase content-length", "expected 42");

    const char *nested = "POST /upload HTTP/1.1\r\n"
                         "X-Note: Content-Length: 999, content-type: application/zip\r\n"
                         "Content-Length: 5\r\n"
                         "\r\n";
    if (http_content_length(nested) != 5) failf("header name inside value", "matched inside another value");
    expect_header("header name inside value", nested, "Content-Type", NULL);

    if (http_content_length("POST / HTTP/1.1\r\nContent-Length: 12abc\r\n\r\n") != -1) {
        failf("non-numeric content-length", "expected -1");
    }
}

static void expect_escape(const char *case_name, const char *in, size_t out_size, const char *expected)
{
    char out[256];
    json_escape(out, out_size, in);
    if (strcmp(out, expected) != 0) failf(case_name, out);
}

static void test_json_escape(void)
{
    expect_escape("quote and backslash", "evil\"name\\.vdapp", 256, "evil\\\"name\\\\.vdapp");
    expect_escape("control character", "a\nb", 256, "a\\u000ab");
    expect_escape("truncation keeps escapes whole", "ab\"", 4, "ab");
}

typedef struct {
    unsigned char data[4096];
    size_t len;
} Buf;

static void put(Buf *b, const void *data, size_t len)
{
    memcpy(b->data + b->len, data, len);
    b->len += len;
}

static void put16(Buf *b, uint16_t v)
{
    unsigned char p[2] = {(unsigned char)v, (unsigned char)(v >> 8)};
    put(b, p, 2);
}

static void put32(Buf *b, uint32_t v)
{
    unsigned char p[4] = {(unsigned char)v, (unsigned char)(v >> 8), (unsigned char)(v >> 16),
                          (unsigned char)(v >> 24)};
    put(b, p, 4);
}

typedef struct {
    bool deflate;
    uint32_t uncompressed_size_override;
    bool override_local_offset;
    uint32_t local_offset;
    bool override_central;
    uint32_t central_offset;
    uint32_t central_size;
} ZipTweaks;

/* Builds a single-entry zip; tweaks forge the offsets an attacker controls. */
static void build_zip(Buf *zip, const char *name, const char *contents, ZipTweaks tweaks)
{
    size_t name_len = strlen(name);
    size_t raw_len = strlen(contents);
    unsigned char payload[1024];
    size_t payload_len = raw_len;
    if (tweaks.deflate) {
        z_stream stream = {0};
        deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
        stream.next_in = (Bytef *)contents;
        stream.avail_in = (uInt)raw_len;
        stream.next_out = payload;
        stream.avail_out = sizeof(payload);
        deflate(&stream, Z_FINISH);
        payload_len = stream.total_out;
        deflateEnd(&stream);
    } else {
        memcpy(payload, contents, raw_len);
    }
    uint16_t method = tweaks.deflate ? 8 : 0;
    uint32_t uncompressed_size =
        tweaks.uncompressed_size_override ? tweaks.uncompressed_size_override : (uint32_t)raw_len;

    zip->len = 0;
    put32(zip, 0x04034b50);
    put16(zip, 20);
    put16(zip, 0);
    put16(zip, method);
    put32(zip, 0);
    put32(zip, 0);
    put32(zip, (uint32_t)payload_len);
    put32(zip, uncompressed_size);
    put16(zip, (uint16_t)name_len);
    put16(zip, 0);
    put(zip, name, name_len);
    put(zip, payload, payload_len);

    size_t central_offset = zip->len;
    put32(zip, 0x02014b50);
    put16(zip, 20);
    put16(zip, 20);
    put16(zip, 0);
    put16(zip, method);
    put32(zip, 0);
    put32(zip, 0);
    put32(zip, (uint32_t)payload_len);
    put32(zip, uncompressed_size);
    put16(zip, (uint16_t)name_len);
    put16(zip, 0);
    put16(zip, 0);
    put16(zip, 0);
    put16(zip, 0);
    put32(zip, 0);
    put32(zip, tweaks.override_local_offset ? tweaks.local_offset : 0);
    put(zip, name, name_len);
    size_t central_size = zip->len - central_offset;

    put32(zip, 0x06054b50);
    put16(zip, 0);
    put16(zip, 0);
    put16(zip, 1);
    put16(zip, 1);
    put32(zip, tweaks.override_central ? tweaks.central_size : (uint32_t)central_size);
    put32(zip, tweaks.override_central ? tweaks.central_offset : (uint32_t)central_offset);
    put16(zip, 0);
}

static bool extract(const char *case_name, const Buf *zip, char *error, size_t error_size)
{
    char zip_path[VD_PATH_MAX];
    snprintf(zip_path, sizeof(zip_path), "%s/upload.zip", g_root);
    FILE *f = fopen(zip_path, "wb");
    if (!f || fwrite(zip->data, 1, zip->len, f) != zip->len) {
        if (f) fclose(f);
        failf(case_name, "could not write fixture zip");
        return false;
    }
    fclose(f);

    Arena arena = {0};
    VdArchiveExtractResult result;
    error[0] = '\0';
    bool ok = upload_archive_extract(&arena, zip_path, &result, error, error_size);
    arena_free(&arena);
    return ok;
}

static void expect_extract_failure(const char *case_name, const Buf *zip, const char *expected_error)
{
    char error[256];
    if (extract(case_name, zip, error, sizeof(error))) {
        failf(case_name, "extract should fail");
    } else if (strcmp(error, expected_error) != 0) {
        failf(case_name, error);
    }
}

static void expect_extracted_file(const char *case_name, const Buf *zip, const char *rel_path, const char *contents)
{
    char error[256];
    if (!extract(case_name, zip, error, sizeof(error))) {
        failf(case_name, error);
        return;
    }
    char path[VD_PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s", package_library_staging_root(), rel_path);
    char actual[256] = "";
    FILE *f = fopen(path, "rb");
    if (!f) {
        failf(case_name, "extracted file is missing");
        return;
    }
    size_t len = fread(actual, 1, sizeof(actual) - 1, f);
    fclose(f);
    actual[len] = '\0';
    if (strcmp(actual, contents) != 0) failf(case_name, "extracted contents mismatch");
}

static void test_archives(void)
{
    const char *app = "export default function App() { return null; }";
    Buf zip;

    build_zip(&zip, "sample.vdapp/app.js", app, (ZipTweaks){0});
    expect_extracted_file("stored entry", &zip, "sample.vdapp/app.js", app);

    build_zip(&zip, "sample.vdapp/app.js", app, (ZipTweaks){.deflate = true});
    expect_extracted_file("deflated entry", &zip, "sample.vdapp/app.js", app);

    build_zip(&zip, "sample.vdapp/app.js", app, (ZipTweaks){.deflate = true, .uncompressed_size_override = 8});
    expect_extract_failure("deflated entry larger than declared", &zip,
                           "Archive contains an unsupported or invalid compressed entry.");

    build_zip(&zip, "sample.vdapp/app.js", app,
              (ZipTweaks){.override_central = true, .central_offset = 0xFFFFFFF0u, .central_size = 0x20});
    expect_extract_failure("central directory offset wraps", &zip, "Archive entry count or directory is invalid.");

    build_zip(&zip, "sample.vdapp/app.js", app,
              (ZipTweaks){.override_local_offset = true, .local_offset = 0xFFFFFFF0u});
    expect_extract_failure("local header offset wraps", &zip,
                           "Archive exceeds unpacked size limit or has invalid local data.");

    build_zip(&zip, "sample.vdapp/app.js", app, (ZipTweaks){.uncompressed_size_override = 0xFFFFFFFFu});
    expect_extract_failure("entry over unpacked limit", &zip,
                           "Archive exceeds unpacked size limit or has invalid local data.");

    build_zip(&zip, "../evil.vdapp/app.js", app, (ZipTweaks){0});
    expect_extract_failure("path traversal", &zip, "Archive contains an unsafe path.");
}

int main(void)
{
    if (!mkdtemp(g_root) || setenv("VITADECK_DATA_ROOT", g_root, 1) != 0) {
        fprintf(stderr, "upload_parsing_harness: could not create temp data root\n");
        return 1;
    }
    char error[256];
    if (!package_library_init(error, sizeof(error))) {
        fprintf(stderr, "upload_parsing_harness: %s\n", error);
        return 1;
    }

    test_headers();
    test_json_escape();
    test_archives();

    char cmd[VD_PATH_MAX + 16];
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", g_root);
    if (system(cmd) != 0) fprintf(stderr, "upload_parsing_harness: could not remove %s\n", g_root);

    if (failures > 0) {
        fprintf(stderr, "upload_parsing_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("upload_parsing_harness: ok\n");
    return 0;
}
