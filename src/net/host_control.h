#ifndef VD_HOST_CONTROL_H
#define VD_HOST_CONTROL_H

#include <stdbool.h>
#include <stddef.h>

struct JSContext;
struct JSRuntime;

void host_control_init(void);
void host_control_shutdown(struct JSContext *ctx);
void host_control_drain_completions(struct JSContext *ctx, struct JSRuntime *rt);

bool host_control_poll_waiting(void);

void host_control_handle_poll(int client_fd);
void host_control_handle_result(int client_fd, const char *body, size_t body_len);

#endif /* VD_HOST_CONTROL_H */
