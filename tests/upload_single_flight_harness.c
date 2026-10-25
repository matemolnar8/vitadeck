// Runtime Upload Single-Flight: drives the static handle_upload over socketpairs, so no listener or network.
#include "upload/http_server.c"

#include <sys/stat.h>

static int failures = 0;

static void expect(bool condition, const char *message)
{
    if (condition) return;
    fprintf(stderr, "upload_single_flight_harness: %s\n", message);
    failures++;
}

typedef struct {
    int client_fd;
    HandlerArg arg;
} Client;

static bool client_open(Client *client, VdUploadServer *server, const char *request)
{
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) return false;
    client->client_fd = fds[0];
    client->arg = (HandlerArg){.server = server, .fd = fds[1]};
    size_t len = strlen(request);
    return send(fds[0], request, len, 0) == (ssize_t)len;
}

// Closes the handler side like handler_thread does, then reads the whole response.
static void client_finish(Client *client, char *response, size_t response_size)
{
    close(client->arg.fd);
    size_t used = 0;
    ssize_t n;
    while (used + 1 < response_size && (n = recv(client->client_fd, response + used, response_size - used - 1, 0)) > 0)
        used += (size_t)n;
    response[used] = '\0';
    close(client->client_fd);
}

static void *run_handle_upload(void *raw)
{
    handle_upload(raw, NULL);
    return NULL;
}

static bool is_ingesting(VdUploadServer *server)
{
    vd_mutex_lock(server->mutex);
    bool ingesting = server->ingesting;
    vd_mutex_unlock(server->mutex);
    return ingesting;
}

static bool staged_archive_exists(void)
{
    char path[VD_PATH_MAX];
    struct stat st;
    snprintf(path, sizeof(path), "%s/upload.zip", package_library_staging_root());
    return stat(path, &st) == 0;
}

static const char *const zip_headers = "POST /upload HTTP/1.1\r\n"
                                       "Content-Type: application/zip\r\n"
                                       "Content-Length: 4\r\n"
                                       "\r\n";

static void upload_once(VdUploadServer *server, const char *request, char *response, size_t response_size)
{
    Client client;
    if (!client_open(&client, server, request)) {
        expect(false, "could not open a socketpair client");
        response[0] = '\0';
        return;
    }
    handle_upload(&client.arg, NULL);
    client_finish(&client, response, response_size);
}

static void test_second_upload_is_rejected(VdUploadServer *server)
{
    // The first upload sends only its headers, so it holds the ingest lock while it waits for the body.
    Client first;
    if (!client_open(&first, server, zip_headers)) {
        expect(false, "could not open the first client");
        return;
    }
    vd_thread *thread = vd_thread_create(run_handle_upload, &first.arg);
    if (!thread) {
        expect(false, "could not start the first upload");
        return;
    }
    for (int i = 0; i < 500 && !is_ingesting(server); i++)
        vd_sleep_ms(10);
    expect(is_ingesting(server), "the first upload takes the ingest lock before reading its body");

    char response[4096];
    char second_request[256];
    snprintf(second_request, sizeof(second_request), "%sPK\x03\x04", zip_headers);
    upload_once(server, second_request, response, sizeof(response));
    expect(strncmp(response, "HTTP/1.1 409 Conflict\r\n", 23) == 0, "a second upload gets 409 Conflict");
    expect(strstr(response, "\"code\":\"upload_in_progress\"") != NULL, "the 409 body says upload_in_progress");
    expect(!staged_archive_exists(), "the rejected upload writes nothing to staging");
    expect(is_ingesting(server), "the rejected upload leaves the first upload's lock in place");

    send(first.client_fd, "junk", 4, 0);
    vd_thread_join(thread);
    vd_thread_destroy(thread);
    client_finish(&first, response, sizeof(response));
    expect(strstr(response, "HTTP/1.1 422 ") == response, "the first upload finishes on its own");
    expect(staged_archive_exists(), "the first upload staged its archive");
    expect(!is_ingesting(server), "a failed upload releases the ingest lock");
}

static void test_lock_released_on_every_exit(VdUploadServer *server)
{
    char response[4096];
    char request[256];
    snprintf(request, sizeof(request), "%sjunk", zip_headers);
    upload_once(server, request, response, sizeof(response));
    expect(strstr(response, "HTTP/1.1 422 ") == response, "an upload after a finished one is not rejected");
    expect(!is_ingesting(server), "an invalid archive releases the ingest lock");

    upload_once(server,
                "POST /upload HTTP/1.1\r\n"
                "Content-Type: application/zip\r\n"
                "Content-Length: 99999999\r\n"
                "\r\n",
                response, sizeof(response));
    expect(strstr(response, "HTTP/1.1 413 ") == response, "an oversized upload gets 413");
    expect(!is_ingesting(server), "an oversized upload releases the ingest lock");

    upload_once(server,
                "POST /upload HTTP/1.1\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 4\r\n"
                "\r\n"
                "junk",
                response, sizeof(response));
    expect(strstr(response, "HTTP/1.1 415 ") == response, "a wrong content type gets 415");
    expect(!is_ingesting(server), "a wrong content type releases the ingest lock");
}

// Writing a reply to a client that already hung up must fail the send, not raise SIGPIPE and kill the process.
static void test_client_gone_before_reply(VdUploadServer *server)
{
    Client client;
    if (!client_open(&client, server,
                     "POST /upload HTTP/1.1\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 4\r\n"
                     "\r\n"
                     "junk")) {
        expect(false, "could not open a socketpair client");
        return;
    }
    close(client.client_fd);
    handle_upload(&client.arg, NULL);
    close(client.arg.fd);
    expect(!is_ingesting(server), "a reply to a gone client releases the ingest lock");
}

int main(void)
{
    char root[] = "/tmp/vitadeck-single-flight-XXXXXX";
    char error[256] = "";
    if (!mkdtemp(root) || setenv("VITADECK_DATA_ROOT", root, 1) != 0 || !package_library_init(error, sizeof(error))) {
        fprintf(stderr, "upload_single_flight_harness: could not set up a data root: %s\n", error);
        return 1;
    }

    VdUploadServer server;
    upload_server_init(&server);
    test_second_upload_is_rejected(&server);
    test_lock_released_on_every_exit(&server);
    test_client_gone_before_reply(&server);
    vd_mutex_destroy(server.mutex);
    vd_remove_tree(root);

    if (failures) {
        fprintf(stderr, "upload_single_flight_harness: %d failure(s)\n", failures);
        return 1;
    }
    printf("upload_single_flight_harness: ok\n");
    return 0;
}
