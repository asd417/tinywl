#ifndef TINYWL_STRUCTS_H
#define TINYWL_STRUCTS_H
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <wayland-server-core.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/box.h>

#include <xkbcommon/xkbcommon.h>

#include "decoration.h"
#include "cursor.h"
#include "shell.h"
/* For brevity's sake, struct members are annotated where they are used. */
enum tinywl_cursor_mode
{
    TINYWL_CURSOR_PASSTHROUGH,
    TINYWL_CURSOR_MOVE,
    TINYWL_CURSOR_RESIZE,
};

struct tinywl_server
{
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;
    struct wlr_scene_rect *background_rect;

    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_toplevel;
    struct wl_listener new_xdg_popup;
    struct wl_list toplevels;

    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;
    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;

    struct wlr_seat *seat;
    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener pointer_focus_change;
    struct wl_listener request_set_selection;
    struct wl_list keyboards;
    enum tinywl_cursor_mode cursor_mode;
    struct tinywl_toplevel *grabbed_toplevel;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;
    struct wl_listener new_output;

    struct wlr_xdg_decoration_manager_v1 *xdg_decoration_manager;
    struct wl_listener new_xdg_decoration;
};

struct tinywl_output
{
    struct wl_list link;
    struct tinywl_server *server;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;
};

struct tinywl_toplevel
{
    struct wl_list link;
    struct tinywl_server *server;
    struct wlr_xdg_toplevel *xdg_toplevel;
    struct wlr_scene_tree *scene_tree;
    struct wlr_scene_rect *title_bar;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;

    struct wlr_xdg_toplevel_decoration_v1 *decoration;
    struct wl_listener decoration_request_mode;
    struct wl_listener decoration_destroy;
};

struct tinywl_popup
{
    struct wlr_xdg_popup *xdg_popup;
    struct wl_listener commit;
    struct wl_listener destroy;
};

struct tinywl_keyboard
{
    struct wl_list link;
    struct tinywl_server *server;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

void focus_toplevel(struct tinywl_toplevel *toplevel);
void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
bool handle_keybinding(struct tinywl_server *server, xkb_keysym_t sym);
void keyboard_handle_key(struct wl_listener *listener, void *data);
void keyboard_handle_destroy(struct wl_listener *listener, void *data);
void server_new_keyboard(struct tinywl_server *server, struct wlr_input_device *device);
void server_new_pointer(struct tinywl_server *server, struct wlr_input_device *device);
void server_new_input(struct wl_listener *listener, void *data);
void seat_request_cursor(struct wl_listener *listener, void *data);
void seat_pointer_focus_change(struct wl_listener *listener, void *data);
void seat_request_set_selection(struct wl_listener *listener, void *data);
struct tinywl_toplevel *desktop_toplevel_at(
    struct tinywl_server *server, double lx, double ly,
    struct wlr_surface **surface, double *sx, double *sy);

void output_frame(struct wl_listener *listener, void *data);
void output_request_state(struct wl_listener *listener, void *data);
void output_destroy(struct wl_listener *listener, void *data);
void server_new_output(struct wl_listener *listener, void *data);
void begin_interactive(struct tinywl_toplevel *toplevel,
                       enum tinywl_cursor_mode mode, uint32_t edges);



#endif