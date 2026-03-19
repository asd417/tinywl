#ifndef TINYWL_DECORATION_H
#define TINYWL_DECORATION_H

void server_new_xdg_decoration(struct wl_listener *listener, void *data);
void decoration_handle_request_mode(struct wl_listener *listener, void *data);
void decoration_handle_destroy(struct wl_listener *listener, void *data);

#endif