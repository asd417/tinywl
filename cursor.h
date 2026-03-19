#ifndef TINYWL_CURSOR_H
#define TINYWL_CURSOR_H

struct tinywl_server;
void reset_cursor_mode(struct tinywl_server *server);
void server_cursor_motion(struct wl_listener *listener, void *data);
void server_cursor_motion_absolute(struct wl_listener *listener, void *data);
void server_cursor_button(struct wl_listener *listener, void *data);
void server_cursor_axis(struct wl_listener *listener, void *data);
void server_cursor_frame(struct wl_listener *listener, void *data);

#endif