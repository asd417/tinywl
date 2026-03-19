#include "tinywl.h"

void process_cursor_move(struct tinywl_server *server)
{
    /* Move the grabbed toplevel to the new position. */
    struct tinywl_toplevel *toplevel = server->grabbed_toplevel;
    wlr_scene_node_set_position(&toplevel->scene_tree->node,
                                server->cursor->x - server->grab_x,
                                server->cursor->y - server->grab_y);
}

void process_cursor_resize(struct tinywl_server *server)
{
    /*
     * Resizing the grabbed toplevel can be a little bit complicated, because we
     * could be resizing from any corner or edge. This not only resizes the
     * toplevel on one or two axes, but can also move the toplevel if you resize
     * from the top or left edges (or top-left corner).
     *
     * Note that some shortcuts are taken here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at the new
     * size, then commit any movement that was prepared.
     */
    struct tinywl_toplevel *toplevel = server->grabbed_toplevel;
    double border_x = server->cursor->x - server->grab_x;
    double border_y = server->cursor->y - server->grab_y;
    int new_left = server->grab_geobox.x;
    int new_right = server->grab_geobox.x + server->grab_geobox.width;
    int new_top = server->grab_geobox.y;
    int new_bottom = server->grab_geobox.y + server->grab_geobox.height;

    if (server->resize_edges & WLR_EDGE_TOP)
    {
        new_top = border_y;
        if (new_top >= new_bottom)
        {
            new_top = new_bottom - 1;
        }
    }
    else if (server->resize_edges & WLR_EDGE_BOTTOM)
    {
        new_bottom = border_y;
        if (new_bottom <= new_top)
        {
            new_bottom = new_top + 1;
        }
    }
    if (server->resize_edges & WLR_EDGE_LEFT)
    {
        new_left = border_x;
        if (new_left >= new_right)
        {
            new_left = new_right - 1;
        }
    }
    else if (server->resize_edges & WLR_EDGE_RIGHT)
    {
        new_right = border_x;
        if (new_right <= new_left)
        {
            new_right = new_left + 1;
        }
    }

    struct wlr_box *geo_box = &toplevel->xdg_toplevel->base->geometry;
    wlr_scene_node_set_position(&toplevel->scene_tree->node,
                                new_left - geo_box->x, new_top - geo_box->y);

    int new_width = new_right - new_left;
    int new_height = new_bottom - new_top;
    wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, new_width, new_height);
}

void process_cursor_motion(struct tinywl_server *server, uint32_t time)
{
    /* If the mode is non-passthrough, delegate to those functions. */
    if (server->cursor_mode == TINYWL_CURSOR_MOVE)
    {
        process_cursor_move(server);
        return;
    }
    else if (server->cursor_mode == TINYWL_CURSOR_RESIZE)
    {
        process_cursor_resize(server);
        return;
    }

    /* Otherwise, find the toplevel under the pointer and send the event along. */
    double sx, sy;
    struct wlr_seat *seat = server->seat;
    struct wlr_surface *surface = NULL;
    struct tinywl_toplevel *toplevel = desktop_toplevel_at(server,
                                                           server->cursor->x, server->cursor->y, &surface, &sx, &sy);
    if (!toplevel)
    {
        /* If there's no toplevel under the cursor, set the cursor image to a
         * default. This is what makes the cursor image appear when you move it
         * around the screen, not over any toplevels. */
        wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
    }
    if (surface)
    {
        /*
         * Send pointer enter and motion events.
         *
         * The enter event gives the surface "pointer focus", which is distinct
         * from keyboard focus. You get pointer focus by moving the pointer over
         * a window.
         *
         * Note that wlroots will avoid sending duplicate enter/motion events if
         * the surface has already has pointer focus or if the client is already
         * aware of the coordinates passed.
         */
        wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(seat, time, sx, sy);
    }
    else
    {
        /* Clear pointer focus so future button events and such are not sent to
         * the last client to have the cursor over it. */
        wlr_seat_pointer_clear_focus(seat);
    }
}