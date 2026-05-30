#ifndef VD_HOST_CONTROL_LINK_H
#define VD_HOST_CONTROL_LINK_H

#include <stdbool.h>
#include <stddef.h>

void host_control_link_load(void);
bool host_control_link_is_linked(void);
bool host_control_link_get_callback_url(char *out, size_t out_size);
bool host_control_link_get_host_name(char *out, size_t out_size);
void host_control_link_format_status_line(char *out, size_t out_size);
void host_control_link_handle_post(int client_fd, const char *body, size_t body_len);
void host_control_link_handle_status_get(int client_fd);

#endif /* VD_HOST_CONTROL_LINK_H */
