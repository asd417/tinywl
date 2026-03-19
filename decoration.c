#include "tinywl.h"

void decoration_handle_request_mode(struct wl_listener *listener, void *data)
{
  struct tinywl_toplevel *toplevel =
      wl_container_of(listener, toplevel, decoration_request_mode);
  printf("Client requested mode: %d\n", toplevel->decoration->requested_mode);
  /* Safety check */
  if (toplevel->decoration && toplevel->xdg_toplevel->base->surface->mapped)
  {
    wlr_xdg_toplevel_decoration_v1_set_mode(toplevel->decoration,
                                            WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
  }
}
void decoration_handle_destroy(struct wl_listener *listener, void *data)
{
  struct tinywl_toplevel *toplevel =
      wl_container_of(listener, toplevel, decoration_destroy);

  /* 1. Stop listening for mode requests */
  wl_list_remove(&toplevel->decoration_request_mode.link);

  /* 2. Stop listening for this destroy event */
  wl_list_remove(&toplevel->decoration_destroy.link);

  /* 3. Nullify the pointer so other functions don't try to use it */
  toplevel->decoration = NULL;
}

void server_new_xdg_decoration(struct wl_listener *listener, void *data)
{
  struct wlr_xdg_toplevel_decoration_v1 *decoration = data;
  struct tinywl_toplevel *toplevel = decoration->toplevel->base->data;

  /* If the toplevel isn't initialized yet, we can't hook the decoration.
   * wlroots will usually retry this or the client will request a mode later. */
  if (!toplevel)
  {
    return;
  }
  toplevel->decoration = decoration;
  // Add a listener for when the client REQUESTS a specific mode
  // (Kitty will usually ask for 'Client Side' or 'None' first)
  toplevel->decoration_request_mode.notify = decoration_handle_request_mode;
  wl_signal_add(&decoration->events.request_mode, &toplevel->decoration_request_mode);

  // Also listen for when the decoration is destroyed
  toplevel->decoration_destroy.notify = decoration_handle_destroy;
  wl_signal_add(&decoration->events.destroy, &toplevel->decoration_destroy);
}
