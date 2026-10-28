#include "host_control/link.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/json.h"
#include "core/package_library.h"
#include "core/util.h"
#include "platform/thread.h"

static const VdHostControlLinkReply LINK_OK = {200, NULL, NULL};
static const VdHostControlLinkReply LINK_MALFORMED = {
    400, "malformed_request",
    "Link request must be a JSON object with a number protocolVersion and string id, name and url."};
static const VdHostControlLinkReply LINK_UNSUPPORTED_VERSION = {400, "unsupported_protocol_version",
                                                                "VitaDeck speaks Host Control protocol version 1."};
static const VdHostControlLinkReply LINK_INVALID = {
    422, "invalid_payload", "Link needs an id, a name of 1 to 64 bytes, and an http:// url of at most 255 bytes."};

// jimp keeps the string's terminator in string_count, so an embedded NUL shows up as a shorter strlen.
static bool valid_text(const Jimp *jimp, size_t max, bool allow_space)
{
    size_t len = strlen(jimp->string);
    if (len == 0 || len > max || len + 1 != jimp->string_count) return false;
    for (const unsigned char *p = (const unsigned char *)jimp->string; *p; p++) {
        if (*p < 0x20 || *p == 0x7f || (*p == ' ' && !allow_space)) return false;
    }
    return true;
}

// Reads a string member into out when it is valid; false only when the member is not a string.
static bool read_field(Jimp *jimp, char *out, size_t max, bool allow_space, bool *valid)
{
    if (!jimp_string(jimp)) return false;
    *valid = valid_text(jimp, max, allow_space);
    if (*valid) memcpy(out, jimp->string, strlen(jimp->string) + 1);
    return true;
}

typedef struct {
    bool version_seen;
    double version;
    bool id_valid, name_valid, url_valid;
} LinkFields;

// Parses one link object. False when it is not an object of the right member types.
static bool parse_link_object(Jimp *jimp, VdHostControlLink *out, LinkFields *fields)
{
    memset(out, 0, sizeof(*out));
    memset(fields, 0, sizeof(*fields));
    if (!jimp_object_begin(jimp)) return false;
    while (jimp_object_member(jimp)) {
        bool ok;
        if (strcmp(jimp->string, "protocolVersion") == 0) {
            ok = jimp_number(jimp);
            fields->version_seen = ok;
            fields->version = jimp->number;
        } else if (strcmp(jimp->string, "id") == 0) {
            ok = read_field(jimp, out->id, VD_HOST_CONTROL_ID_MAX, false, &fields->id_valid);
        } else if (strcmp(jimp->string, "name") == 0) {
            ok = read_field(jimp, out->name, VD_HOST_CONTROL_NAME_MAX, true, &fields->name_valid);
        } else if (strcmp(jimp->string, "url") == 0) {
            ok = read_field(jimp, out->url, VD_HOST_CONTROL_URL_MAX, false, &fields->url_valid);
            if (ok && fields->url_valid) {
                fields->url_valid = strncmp(out->url, "http://", 7) == 0 && out->url[7] != '\0';
            }
        } else {
            ok = json_skip_value(jimp);
        }
        if (!ok) return false;
    }
    return jimp_object_end(jimp);
}

static bool link_fields_valid(const LinkFields *fields)
{
    return fields->id_valid && fields->name_valid && fields->url_valid;
}

VdHostControlLinkReply host_control_link_parse(const char *json, VdHostControlLink *out)
{
    Jimp jimp = {0};
    jimp_begin(&jimp, "link request", json, strlen(json));
    LinkFields fields;
    bool parsed = parse_link_object(&jimp, out, &fields);
    free(jimp.string);

    if (!parsed) return LINK_MALFORMED;
    if (!fields.version_seen || fields.version != VD_HOST_CONTROL_PROTOCOL_VERSION) return LINK_UNSUPPORTED_VERSION;
    if (!link_fields_valid(&fields)) return LINK_INVALID;
    return LINK_OK;
}

static vd_mutex *g_mutex;
static char g_path[VD_PATH_MAX];
static VdHostControlLink g_links[VD_HOST_CONTROL_MAX_LINKS];
static int g_count;

static char *links_to_json(const VdHostControlLink *links, int count)
{
    // Every escaped byte takes at most 6; the rest is punctuation and member names.
    size_t size = 3 + (size_t)count * (sizeof(VdHostControlLink) * 6 + 64);
    char *json = malloc(size);
    if (!json) return NULL;
    size_t used = 0;
    json[used++] = '[';
    for (int i = 0; i < count; i++) {
        char id[sizeof(links[i].id) * 6];
        char name[sizeof(links[i].name) * 6];
        char url[sizeof(links[i].url) * 6];
        json_escape(id, sizeof(id), links[i].id);
        json_escape(name, sizeof(name), links[i].name);
        json_escape(url, sizeof(url), links[i].url);
        used += (size_t)snprintf(json + used, size - used, "%s{\"id\":\"%s\",\"name\":\"%s\",\"url\":\"%s\"}",
                                 i > 0 ? "," : "", id, name, url);
    }
    json[used++] = ']';
    json[used] = '\0';
    return json;
}

static bool persist(const VdHostControlLink *links, int count)
{
    if (count == 0) return vd_remove_replaced_file(g_path);
    char *json = links_to_json(links, count);
    bool ok = json && vd_replace_file(g_path, json, strlen(json));
    free(json);
    return ok;
}

static void load(void)
{
    g_count = 0;
    size_t len = 0;
    bool found = false;
    char *json = vd_read_replaced_file(g_path, &len, &found);
    if (!json) return;

    Jimp jimp = {0};
    jimp_begin(&jimp, g_path, json, len);
    if (jimp_array_begin(&jimp)) {
        while (g_count < VD_HOST_CONTROL_MAX_LINKS && jimp_array_item(&jimp)) {
            LinkFields fields;
            if (!parse_link_object(&jimp, &g_links[g_count], &fields)) break;
            if (link_fields_valid(&fields)) g_count++;
        }
    }
    free(jimp.string);
    free(json);
}

bool host_control_links_init(const char *data_root)
{
    if (!g_mutex) g_mutex = vd_mutex_create();
    if (!g_mutex || !vd_join_path(g_path, sizeof(g_path), data_root, "host-control-links.json")) return false;
    vd_mutex_lock(g_mutex);
    load();
    vd_mutex_unlock(g_mutex);
    return true;
}

bool host_control_links_save(const VdHostControlLink *link)
{
    VdHostControlLink next[VD_HOST_CONTROL_MAX_LINKS];
    int count = 0;
    next[count++] = *link;
    vd_mutex_lock(g_mutex);
    for (int i = 0; i < g_count && count < VD_HOST_CONTROL_MAX_LINKS; i++) {
        if (strcmp(g_links[i].id, link->id) != 0) next[count++] = g_links[i];
    }
    bool ok = persist(next, count);
    if (ok) {
        memcpy(g_links, next, sizeof(next[0]) * (size_t)count);
        g_count = count;
    }
    vd_mutex_unlock(g_mutex);
    return ok;
}

bool host_control_links_forget(const char *id)
{
    VdHostControlLink next[VD_HOST_CONTROL_MAX_LINKS];
    int count = 0;
    vd_mutex_lock(g_mutex);
    for (int i = 0; i < g_count; i++) {
        if (strcmp(g_links[i].id, id) != 0) next[count++] = g_links[i];
    }
    bool ok = count < g_count && persist(next, count);
    if (ok) {
        memcpy(g_links, next, sizeof(next[0]) * (size_t)count);
        g_count = count;
    }
    vd_mutex_unlock(g_mutex);
    return ok;
}

int host_control_links_list(VdHostControlLink *out, int max)
{
    vd_mutex_lock(g_mutex);
    int count = g_count < max ? g_count : max;
    memcpy(out, g_links, sizeof(out[0]) * (size_t)count);
    vd_mutex_unlock(g_mutex);
    return count;
}

char *host_control_links_json(void)
{
    vd_mutex_lock(g_mutex);
    char *json = links_to_json(g_links, g_count);
    vd_mutex_unlock(g_mutex);
    return json;
}
