#ifndef VD_HOST_CONTROL_LINK_H
#define VD_HOST_CONTROL_LINK_H

#include <stdbool.h>
#include <stddef.h>

#define VD_HOST_CONTROL_PROTOCOL_VERSION 1
#define VD_HOST_CONTROL_MAX_LINKS 4
#define VD_HOST_CONTROL_ID_MAX 64
#define VD_HOST_CONTROL_NAME_MAX 64
#define VD_HOST_CONTROL_URL_MAX 255

// One computer whose Host Companion linked with this VitaDeck.
typedef struct {
    char id[VD_HOST_CONTROL_ID_MAX + 1];
    char name[VD_HOST_CONTROL_NAME_MAX + 1];
    char url[VD_HOST_CONTROL_URL_MAX + 1];
} VdHostControlLink;

// How the link endpoint answers a request: 200 with a NULL code, or a wire error code and message.
typedef struct {
    int status;
    const char *code;
    const char *message;
} VdHostControlLinkReply;

// Validates a NUL-terminated `POST /link` body. Pure; `out` and `relink` are only meaningful when the status is 200.
VdHostControlLinkReply host_control_link_parse(const char *json, VdHostControlLink *out, bool *relink);

// The Host Control Links, newest first, kept in <data root>/host-control-links.json. Safe from any thread.
bool host_control_links_init(const char *data_root);
// Puts the link first, replacing the entry with its id or else dropping the oldest past the cap, and persists.
bool host_control_links_save(const VdHostControlLink *link);
// Updates the link with this id in place, persisting only a change: 200, 404 unknown_link, or 500 command_failed.
VdHostControlLinkReply host_control_links_relink(const VdHostControlLink *link, bool *changed);
bool host_control_links_forget(const char *id);
int host_control_links_list(VdHostControlLink *out, int max);
// The list as a malloc'd JSON array of {id,name,url}, newest first, or NULL when out of memory.
char *host_control_links_json(void);

#endif /* VD_HOST_CONTROL_LINK_H */
