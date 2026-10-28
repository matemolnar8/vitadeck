// Host Control Link: the wire fixtures through the parser, the link store on disk, and the listener over loopback.
#include "core/json.h"
#include "core/package_library.h"
#include "core/util.h"
#include "host_control/link.h"
#include "host_control/listener.h"
#include "platform/socket.h"
#include "platform/thread.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures = 0;
static char g_root[] = "/tmp/vitadeck-host-control-XXXXXX";

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "host_control_link_harness: %s\n", message);
    failures++;
}

#define FIXTURE_TEXT_MAX 256

typedef struct {
    char *request;
    int status;
    char code[FIXTURE_TEXT_MAX];
    char message[FIXTURE_TEXT_MAX];
} Fixture;

// Reads a `{request, status, response}` triple, keeping the request as its raw JSON text.
static bool load_fixture(const char *name, Fixture *fixture)
{
    memset(fixture, 0, sizeof(*fixture));
    char path[VD_PATH_MAX];
    snprintf(path, sizeof(path), "%s/tests/fixtures/host-control/%s", VD_SOURCE_DIR, name);
    size_t len = 0;
    char *json = vd_read_file(path, &len);
    if (!json) return false;

    Jimp jimp = {0};
    jimp_begin(&jimp, path, json, len);
    bool ok = jimp_object_begin(&jimp);
    while (ok && jimp_object_member(&jimp)) {
        if (strcmp(jimp.string, "request") == 0) {
            const char *start = jimp.point;
            ok = json_skip_value(&jimp);
            size_t request_len = (size_t)(jimp.point - start);
            fixture->request = malloc(request_len + 1);
            memcpy(fixture->request, start, request_len);
            fixture->request[request_len] = '\0';
        } else if (strcmp(jimp.string, "status") == 0) {
            ok = jimp_number(&jimp);
            fixture->status = (int)jimp.number;
        } else if (strcmp(jimp.string, "response") == 0) {
            ok = jimp_object_begin(&jimp);
            while (ok && jimp_object_member(&jimp)) {
                char *out = strcmp(jimp.string, "code") == 0      ? fixture->code
                            : strcmp(jimp.string, "message") == 0 ? fixture->message
                                                                  : NULL;
                if (!out) {
                    ok = json_skip_value(&jimp);
                    continue;
                }
                ok = jimp_string(&jimp);
                if (ok) snprintf(out, FIXTURE_TEXT_MAX, "%s", jimp.string);
            }
            if (ok) ok = jimp_object_end(&jimp);
        } else {
            ok = json_skip_value(&jimp);
        }
    }
    if (ok) ok = jimp_object_end(&jimp);
    free(jimp.string);
    free(json);
    return ok && fixture->request;
}

static void expect_reply(const char *case_name, VdHostControlLinkReply reply, int status, const char *code,
                         const char *message)
{
    char line[512];
    if (reply.status != status) {
        snprintf(line, sizeof(line), "%s: status %d, expected %d", case_name, reply.status, status);
        expect(false, line);
        return;
    }
    if (!code) {
        snprintf(line, sizeof(line), "%s: a success should carry no code", case_name);
        expect(reply.code == NULL, line);
        return;
    }
    snprintf(line, sizeof(line), "%s: code %s, expected %s", case_name, reply.code ? reply.code : "(null)", code);
    expect(reply.code && strcmp(reply.code, code) == 0, line);
    if (message) {
        snprintf(line, sizeof(line), "%s: message \"%s\"", case_name, reply.message ? reply.message : "(null)");
        expect(reply.message && strcmp(reply.message, message) == 0, line);
    }
}

static void test_fixtures(void)
{
    const char *names[] = {"link-ok.json", "link-invalid-url.json", "link-version-mismatch.json"};
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        Fixture fixture;
        if (!load_fixture(names[i], &fixture)) {
            expect(false, names[i]);
            continue;
        }
        VdHostControlLink link;
        VdHostControlLinkReply reply = host_control_link_parse(fixture.request, &link);
        expect_reply(names[i], reply, fixture.status, fixture.status == 200 ? NULL : fixture.code,
                     fixture.status == 200 ? NULL : fixture.message);
        if (fixture.status == 200) {
            expect(strcmp(link.id, "3f0c9a52-6f1e-4c55-9d0b-0a8f4c1e7b21") == 0, "link-ok.json: id");
            expect(strcmp(link.name, "Studio Mac") == 0, "link-ok.json: name");
            expect(strcmp(link.url, "http://192.168.1.20:8797/") == 0, "link-ok.json: url");
        }
        free(fixture.request);
    }
}

static void test_parse_edges(void)
{
    VdHostControlLink link;
    expect_reply("not JSON", host_control_link_parse("{\"protocolVersion\":1,", &link), 400, "malformed_request", NULL);
    expect_reply("an array", host_control_link_parse("[]", &link), 400, "malformed_request", NULL);
    expect_reply(
        "a number name",
        host_control_link_parse("{\"protocolVersion\":1,\"id\":\"a\",\"name\":5,\"url\":\"http://h/\"}", &link), 400,
        "malformed_request", NULL);
    expect_reply(
        "a string protocolVersion",
        host_control_link_parse("{\"protocolVersion\":\"1\",\"id\":\"a\",\"name\":\"n\",\"url\":\"http://h/\"}", &link),
        400, "malformed_request", NULL);
    expect_reply("the version comes before the fields",
                 host_control_link_parse("{\"url\":\"ftp://h/\",\"protocolVersion\":2}", &link), 400,
                 "unsupported_protocol_version", NULL);
    expect_reply("a missing protocolVersion",
                 host_control_link_parse("{\"id\":\"a\",\"name\":\"n\",\"url\":\"http://h/\"}", &link), 400,
                 "unsupported_protocol_version", NULL);
    expect_reply("unknown members are ignored",
                 host_control_link_parse("{\"protocolVersion\":1,\"extra\":{\"a\":[null,true]},\"id\":\"a\","
                                         "\"name\":\"Den PC\",\"url\":\"http://h/\"}",
                                         &link),
                 200, NULL, NULL);
    expect_reply("a missing name",
                 host_control_link_parse("{\"protocolVersion\":1,\"id\":\"a\",\"url\":\"http://h/\"}", &link), 422,
                 "invalid_payload", NULL);
    expect_reply(
        "an id with a space",
        host_control_link_parse("{\"protocolVersion\":1,\"id\":\"a b\",\"name\":\"n\",\"url\":\"http://h/\"}", &link),
        422, "invalid_payload", NULL);
    expect_reply(
        "a url with a space",
        host_control_link_parse("{\"protocolVersion\":1,\"id\":\"a\",\"name\":\"n\",\"url\":\"http://h /\"}", &link),
        422, "invalid_payload", NULL);
    expect_reply(
        "a name with a control byte",
        host_control_link_parse("{\"protocolVersion\":1,\"id\":\"a\",\"name\":\"a\\tb\",\"url\":\"http://h/\"}", &link),
        422, "invalid_payload", NULL);
    expect_reply(
        "a bare scheme",
        host_control_link_parse("{\"protocolVersion\":1,\"id\":\"a\",\"name\":\"n\",\"url\":\"http://\"}", &link), 422,
        "invalid_payload", NULL);

    char json[1024];
    char name[VD_HOST_CONTROL_NAME_MAX + 2];
    memset(name, 'n', sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    snprintf(json, sizeof(json), "{\"protocolVersion\":1,\"id\":\"a\",\"name\":\"%s\",\"url\":\"http://h/\"}", name);
    expect_reply("a 65-byte name", host_control_link_parse(json, &link), 422, "invalid_payload", NULL);
    name[VD_HOST_CONTROL_NAME_MAX] = '\0';
    snprintf(json, sizeof(json), "{\"protocolVersion\":1,\"id\":\"a\",\"name\":\"%s\",\"url\":\"http://h/\"}", name);
    expect_reply("a 64-byte name", host_control_link_parse(json, &link), 200, NULL, NULL);

    char url[VD_HOST_CONTROL_URL_MAX + 2];
    memset(url, 'u', sizeof(url) - 1);
    memcpy(url, "http://", 7);
    url[sizeof(url) - 1] = '\0';
    snprintf(json, sizeof(json), "{\"protocolVersion\":1,\"id\":\"a\",\"name\":\"n\",\"url\":\"%s\"}", url);
    expect_reply("a 256-byte url", host_control_link_parse(json, &link), 422, "invalid_payload", NULL);
    url[VD_HOST_CONTROL_URL_MAX] = '\0';
    snprintf(json, sizeof(json), "{\"protocolVersion\":1,\"id\":\"a\",\"name\":\"n\",\"url\":\"%s\"}", url);
    expect_reply("a 255-byte url", host_control_link_parse(json, &link), 200, NULL, NULL);
}

static VdHostControlLink make_link(const char *id, const char *name)
{
    VdHostControlLink link = {0};
    snprintf(link.id, sizeof(link.id), "%s", id);
    snprintf(link.name, sizeof(link.name), "%s", name);
    snprintf(link.url, sizeof(link.url), "http://192.168.1.20:8797/%s", id);
    return link;
}

// Whether the store lists exactly these ids, newest first.
static bool links_are(const char *const *ids, int count)
{
    VdHostControlLink links[VD_HOST_CONTROL_MAX_LINKS];
    if (host_control_links_list(links, VD_HOST_CONTROL_MAX_LINKS) != count) return false;
    for (int i = 0; i < count; i++) {
        if (strcmp(links[i].id, ids[i]) != 0) return false;
    }
    return true;
}

static bool links_file_exists(void)
{
    char path[VD_PATH_MAX];
    snprintf(path, sizeof(path), "%s/host-control-links.json", g_root);
    struct stat st;
    return stat(path, &st) == 0;
}

static void test_store(void)
{
    expect(host_control_links_init(g_root), "the store should init on an empty data root");
    expect(links_are(NULL, 0), "an empty data root should have no links");

    VdHostControlLink a = make_link("a", "Studio Mac");
    VdHostControlLink b = make_link("b", "Den PC");
    expect(host_control_links_save(&a) && host_control_links_save(&b), "saving links failed");
    expect(links_are((const char *[]){"b", "a"}, 2), "the newest link should come first");

    expect(host_control_links_init(g_root), "reloading the store failed");
    expect(links_are((const char *[]){"b", "a"}, 2), "links should survive a reload from disk");
    VdHostControlLink reloaded[VD_HOST_CONTROL_MAX_LINKS];
    host_control_links_list(reloaded, VD_HOST_CONTROL_MAX_LINKS);
    expect(strcmp(reloaded[1].name, "Studio Mac") == 0 && strcmp(reloaded[1].url, a.url) == 0,
           "a reloaded link should keep its name and url");

    VdHostControlLink renamed = make_link("a", "Studio Mac Pro");
    expect(host_control_links_save(&renamed), "relinking failed");
    expect(links_are((const char *[]){"a", "b"}, 2), "relinking an id should replace it and move it first");
    host_control_links_list(reloaded, VD_HOST_CONTROL_MAX_LINKS);
    expect(strcmp(reloaded[0].name, "Studio Mac Pro") == 0, "relinking should take the new name");

    VdHostControlLink c = make_link("c", "C"), d = make_link("d", "D"), e = make_link("e", "E");
    host_control_links_save(&c);
    host_control_links_save(&d);
    host_control_links_save(&e);
    expect(links_are((const char *[]){"e", "d", "c", "a"}, 4), "a fifth id should drop the oldest link");

    char *json = host_control_links_json();
    const char *newest = "[{\"id\":\"e\",\"name\":\"E\",\"url\":\"http://192.168.1.20:8797/e\"},";
    expect(json && strncmp(json, newest, strlen(newest)) == 0, "the getter JSON should list the newest link first");
    free(json);

    expect(host_control_links_forget("d"), "forgetting a link failed");
    expect(!host_control_links_forget("d"), "forgetting an unknown id should fail");
    expect(links_are((const char *[]){"e", "c", "a"}, 3), "forgetting should remove only that link");
    expect(host_control_links_init(g_root) && links_are((const char *[]){"e", "c", "a"}, 3),
           "forgetting should persist");

    host_control_links_forget("e");
    host_control_links_forget("c");
    expect(links_file_exists(), "the file should stay while a link is left");
    host_control_links_forget("a");
    expect(!links_file_exists(), "forgetting the last link should delete the file");
    json = host_control_links_json();
    expect(json && strcmp(json, "[]") == 0, "the getter JSON should be [] without links");
    free(json);
}

static int listener_port(const VdHostControlListener *listener)
{
    const char *colon = strrchr(host_control_listener_address(listener), ':');
    return colon ? atoi(colon + 1) : 0;
}

// Sends one request over loopback and reads the whole response.
static void exchange(int port, const char *request, char *response, size_t response_size)
{
    response[0] = '\0';
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons((uint16_t)port)};
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (fd < 0 || connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        expect(false, "could not connect to the listener");
        if (fd >= 0) close(fd);
        return;
    }
    size_t len = strlen(request);
    expect(send(fd, request, len, VD_MSG_NOSIGNAL) == (ssize_t)len, "could not send the request");
    size_t used = 0;
    ssize_t n;
    while (used + 1 < response_size && (n = recv(fd, response + used, response_size - used - 1, 0)) > 0)
        used += (size_t)n;
    response[used] = '\0';
    close(fd);
}

static void post_link(int port, const char *body, char *response, size_t response_size)
{
    char request[2048];
    snprintf(request, sizeof(request),
             "POST /link HTTP/1.1\r\nhost: 127.0.0.1:%d\r\ncontent-type: application/json\r\ncontent-length: "
             "%zu\r\nconnection: close\r\n\r\n%s",
             port, strlen(body), body);
    exchange(port, request, response, response_size);
}

static const char *response_body(const char *response)
{
    const char *end = strstr(response, "\r\n\r\n");
    return end ? end + 4 : "";
}

static void test_loopback(void)
{
    expect(host_control_links_init(g_root), "the store should init for the loopback test");
    VdHostControlListener listener;
    host_control_listener_init(&listener);
    char error[256] = "";
    if (!host_control_listener_start(&listener, error, sizeof(error))) {
        expect(false, error);
        host_control_listener_destroy(&listener);
        return;
    }
    int port = listener_port(&listener);
    expect(port >= 8798 && port <= 8807, "the listener should bind in 8798-8807");

    char response[4096];
    exchange(port, "GET /nope HTTP/1.1\r\nhost: 127.0.0.1\r\n\r\n", response, sizeof(response));
    expect(strncmp(response, "HTTP/1.1 404 ", 13) == 0, "an unknown path should get 404");
    exchange(port, "GET /link HTTP/1.1\r\nhost: 127.0.0.1\r\n\r\n", response, sizeof(response));
    expect(strncmp(response, "HTTP/1.1 405 ", 13) == 0, "a GET on /link should get 405");

    Fixture mismatch;
    if (load_fixture("link-version-mismatch.json", &mismatch)) {
        post_link(port, mismatch.request, response, sizeof(response));
        expect(strncmp(response, "HTTP/1.1 400 ", 13) == 0, "a version mismatch should get 400");
        char expected[512];
        snprintf(expected, sizeof(expected), "{\"ok\":false,\"code\":\"%s\",\"message\":\"%s\"}", mismatch.code,
                 mismatch.message);
        expect(strcmp(response_body(response), expected) == 0, "a version mismatch should answer the fixture body");
        free(mismatch.request);
    } else {
        expect(false, "could not load link-version-mismatch.json");
    }

    char text_plain[512];
    snprintf(text_plain, sizeof(text_plain),
             "POST /link HTTP/1.1\r\ncontent-type: text/plain\r\ncontent-length: 2\r\n\r\n{}");
    exchange(port, text_plain, response, sizeof(response));
    expect(strstr(response, "\"code\":\"malformed_request\"") != NULL, "a non-JSON content type should be malformed");

    exchange(port, "POST /link HTTP/1.1\r\ncontent-type: application/json\r\ncontent-length: 70000\r\n\r\n", response,
             sizeof(response));
    expect(strncmp(response, "HTTP/1.1 413 ", 13) == 0 && strstr(response, "\"code\":\"payload_too_large\""),
           "a body over 64 KiB should get 413 payload_too_large");

    expect(host_control_listener_is_running(&listener), "rejected requests should keep the listener running");
    expect(links_are(NULL, 0), "rejected requests should store nothing");

    Fixture ok;
    if (load_fixture("link-ok.json", &ok)) {
        post_link(port, ok.request, response, sizeof(response));
        expect(strncmp(response, "HTTP/1.1 200 ", 13) == 0, "a valid link should get 200");
        expect(strcmp(response_body(response), "{\"ok\":true}") == 0, "a valid link should answer {\"ok\":true}");
        free(ok.request);
    } else {
        expect(false, "could not load link-ok.json");
    }

    for (int i = 0; i < 200 && host_control_listener_is_running(&listener); i++)
        vd_sleep_ms(10);
    expect(!host_control_listener_is_running(&listener), "the listener should stop after a link");
    expect(links_are((const char *[]){"3f0c9a52-6f1e-4c55-9d0b-0a8f4c1e7b21"}, 1), "the link should be stored");
    VdHostControlLink taken;
    expect(host_control_listener_take_link(&listener, &taken) && strcmp(taken.name, "Studio Mac") == 0,
           "the Shell should be able to take the new link");
    expect(!host_control_listener_take_link(&listener, &taken), "a link should be taken once");

    host_control_listener_destroy(&listener);
}

int main(void)
{
    if (!mkdtemp(g_root)) {
        fprintf(stderr, "host_control_link_harness: could not create a temp data root\n");
        return 1;
    }

    test_fixtures();
    test_parse_edges();
    test_store();
    test_loopback();

    vd_remove_tree(g_root);
    if (failures > 0) {
        fprintf(stderr, "host_control_link_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("host_control_link_harness: ok\n");
    return 0;
}
