#include "tinywl.h"

void server_new_xdg_toplevel(struct wl_listener *listener, void *data)
{
  /* This event is raised when a client creates a new toplevel (application window). */
  struct tinywl_server *server = wl_container_of(listener, server, new_xdg_toplevel);
  struct wlr_xdg_toplevel *xdg_toplevel = data;

  /* Allocate a tinywl_toplevel for this surface */
  struct tinywl_toplevel *toplevel = calloc(1, sizeof(*toplevel));
  toplevel->server = server;
  toplevel->xdg_toplevel = xdg_toplevel;
  xdg_toplevel->base->data = toplevel;

  toplevel->scene_tree = wlr_scene_tree_create(&server->scene->tree);
  float color[4] = {0.2, 0.2, 0.8, 1.0};
  toplevel->title_bar = wlr_scene_rect_create(toplevel->scene_tree, 20, 20, color);

  struct wlr_scene_tree *xdg_tree = wlr_scene_xdg_surface_create(toplevel->scene_tree, xdg_toplevel->base);
  wlr_scene_node_set_position(&xdg_tree->node, 0, 20);

  // toplevel->scene_tree =
  // 	wlr_scene_xdg_surface_create(&toplevel->server->scene->tree, xdg_toplevel->base);
  toplevel->scene_tree->node.data = toplevel;

  /* Listen to the various events it can emit */
  toplevel->map.notify = xdg_toplevel_map;
  wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);
  toplevel->unmap.notify = xdg_toplevel_unmap;
  wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);
  toplevel->commit.notify = xdg_toplevel_commit;
  wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);

  toplevel->destroy.notify = xdg_toplevel_destroy;
  wl_signal_add(&xdg_toplevel->events.destroy, &toplevel->destroy);

  /* cotd */
  toplevel->request_move.notify = xdg_toplevel_request_move;
  wl_signal_add(&xdg_toplevel->events.request_move, &toplevel->request_move);
  toplevel->request_resize.notify = xdg_toplevel_request_resize;
  wl_signal_add(&xdg_toplevel->events.request_resize, &toplevel->request_resize);
  toplevel->request_maximize.notify = xdg_toplevel_request_maximize;
  wl_signal_add(&xdg_toplevel->events.request_maximize, &toplevel->request_maximize);
  toplevel->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
  wl_signal_add(&xdg_toplevel->events.request_fullscreen, &toplevel->request_fullscreen);
}

void xdg_toplevel_map(struct wl_listener *listener, void *data)
{
  /* Called when the surface is mapped, or ready to display on-screen. */
  struct tinywl_toplevel *toplevel = wl_container_of(listener, toplevel, map);

  struct wlr_box geom;
  geom = toplevel->xdg_toplevel->base->current.geometry;
  wlr_scene_rect_set_size(toplevel->title_bar, geom.width, 20);

  wl_list_insert(&toplevel->server->toplevels, &toplevel->link);

  focus_toplevel(toplevel);
}

void xdg_toplevel_unmap(struct wl_listener *listener, void *data)
{
  /* Called when the surface is unmapped, and should no longer be shown. */
  struct tinywl_toplevel *toplevel = wl_container_of(listener, toplevel, unmap);

  /* Reset the cursor mode if the grabbed toplevel was unmapped. */
  if (toplevel == toplevel->server->grabbed_toplevel)
  {
    reset_cursor_mode(toplevel->server);
  }
  wlr_scene_node_set_enabled(&toplevel->scene_tree->node, false);
  wl_list_remove(&toplevel->link);
}

void xdg_toplevel_commit(struct wl_listener *listener, void *data)
{
  /* Called when a new surface state is committed. */
  struct tinywl_toplevel *toplevel = wl_container_of(listener, toplevel, commit);

  if (toplevel->xdg_toplevel->base->initial_commit)
  {
    /* When an xdg_surface performs an initial commit, the compositor must
     * reply with a configure so the client can map the surface. tinywl
     * configures the xdg_toplevel with 0,0 size to let the client pick the
     * dimensions itself. */
    wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
  }
}

void xdg_toplevel_destroy(struct wl_listener *listener, void *data)
{
  /* Called when the xdg_toplevel is destroyed. */
  struct tinywl_toplevel *toplevel = wl_container_of(listener, toplevel, destroy);

  wl_list_remove(&toplevel->map.link);
  wl_list_remove(&toplevel->unmap.link);
  wl_list_remove(&toplevel->commit.link);
  wl_list_remove(&toplevel->destroy.link);
  wl_list_remove(&toplevel->request_move.link);
  wl_list_remove(&toplevel->request_resize.link);
  wl_list_remove(&toplevel->request_maximize.link);
  wl_list_remove(&toplevel->request_fullscreen.link);
  if (toplevel->scene_tree)
  {
    wlr_scene_node_destroy(&toplevel->scene_tree->node);
  }

  free(toplevel);
}


void xdg_toplevel_request_move(
    struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to begin an interactive
   * move, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provided serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct tinywl_toplevel *toplevel = wl_container_of(listener, toplevel, request_move);
  begin_interactive(toplevel, TINYWL_CURSOR_MOVE, 0);
}

void xdg_toplevel_request_resize(
    struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to begin an interactive
   * resize, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provided serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct wlr_xdg_toplevel_resize_event *event = data;
  struct tinywl_toplevel *toplevel = wl_container_of(listener, toplevel, request_resize);
  begin_interactive(toplevel, TINYWL_CURSOR_RESIZE, event->edges);
}

void xdg_toplevel_request_maximize(
    struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to maximize itself,
   * typically because the user clicked on the maximize button on client-side
   * decorations. tinywl doesn't support maximization, but to conform to
   * xdg-shell protocol we still must send a configure.
   * wlr_xdg_surface_schedule_configure() is used to send an empty reply.
   * However, if the request was sent before an initial commit, we don't do
   * anything and let the client finish the initial surface setup. */
  struct tinywl_toplevel *toplevel =
      wl_container_of(listener, toplevel, request_maximize);
  if (toplevel->xdg_toplevel->base->initialized)
  {
    wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
  }
}

void xdg_toplevel_request_fullscreen(
    struct wl_listener *listener, void *data)
{
  /* Just as with request_maximize, we must send a configure here. */
  struct tinywl_toplevel *toplevel =
      wl_container_of(listener, toplevel, request_fullscreen);
  if (toplevel->xdg_toplevel->base->initialized)
  {
    wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
  }
}


void xdg_popup_commit(struct wl_listener *listener, void *data)
{
  /* Called when a new surface state is committed. */
  struct tinywl_popup *popup = wl_container_of(listener, popup, commit);

  if (popup->xdg_popup->base->initial_commit)
  {
    /* When an xdg_surface performs an initial commit, the compositor must
     * reply with a configure so the client can map the surface.
     * tinywl sends an empty configure. A more sophisticated compositor
     * might change an xdg_popup's geometry to ensure it's not positioned
     * off-screen, for example. */
    wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
  }
}

void xdg_popup_destroy(struct wl_listener *listener, void *data)
{
  /* Called when the xdg_popup is destroyed. */
  struct tinywl_popup *popup = wl_container_of(listener, popup, destroy);

  wl_list_remove(&popup->commit.link);
  wl_list_remove(&popup->destroy.link);

  free(popup);
}

void server_new_xdg_popup(struct wl_listener *listener, void *data)
{
  /* This event is raised when a client creates a new popup. */
  struct wlr_xdg_popup *xdg_popup = data;

  struct tinywl_popup *popup = calloc(1, sizeof(*popup));
  popup->xdg_popup = xdg_popup;

  /* We must add xdg popups to the scene graph so they get rendered. The
   * wlroots scene graph provides a helper for this, but to use it we must
   * provide the proper parent scene node of the xdg popup. To enable this,
   * we always set the user data field of xdg_surfaces to the corresponding
   * scene node. */
  struct wlr_xdg_surface *parent = wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
  assert(parent != NULL);
  struct wlr_scene_tree *parent_tree = parent->data;
  xdg_popup->base->data = wlr_scene_xdg_surface_create(parent_tree, xdg_popup->base);

  popup->commit.notify = xdg_popup_commit;
  wl_signal_add(&xdg_popup->base->surface->events.commit, &popup->commit);

  popup->destroy.notify = xdg_popup_destroy;
  wl_signal_add(&xdg_popup->events.destroy, &popup->destroy);
}