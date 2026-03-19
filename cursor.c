#include "tinywl.h"
#include "process.h"
void reset_cursor_mode(struct tinywl_server *server)
{
  /* Reset the cursor mode to passthrough. */
  server->cursor_mode = TINYWL_CURSOR_PASSTHROUGH;
  server->grabbed_toplevel = NULL;
}

void server_cursor_motion(struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits a _relative_
   * pointer motion event (i.e. a delta) */
  struct tinywl_server *server =
      wl_container_of(listener, server, cursor_motion);
  struct wlr_pointer_motion_event *event = data;
  /* The cursor doesn't move unless we tell it to. The cursor automatically
   * handles constraining the motion to the output layout, as well as any
   * special configuration applied for the specific input device which
   * generated the event. You can pass NULL for the device if you want to move
   * the cursor around without any input. */
  wlr_cursor_move(server->cursor, &event->pointer->base,
                  event->delta_x, event->delta_y);
  process_cursor_motion(server, event->time_msec);
}

void server_cursor_motion_absolute(
    struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits an _absolute_
   * motion event, from 0..1 on each axis. This happens, for example, when
   * wlroots is running under a Wayland window rather than KMS+DRM, and you
   * move the mouse over the window. You could enter the window from any edge,
   * so we have to warp the mouse there. There is also some hardware which
   * emits these events. */
  struct tinywl_server *server =
      wl_container_of(listener, server, cursor_motion_absolute);
  struct wlr_pointer_motion_absolute_event *event = data;
  wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x,
                           event->y);
  process_cursor_motion(server, event->time_msec);
}

void server_cursor_button(struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits a button
   * event. */
  struct tinywl_server *server =
      wl_container_of(listener, server, cursor_button);
  struct wlr_pointer_button_event *event = data;
  /* Notify the client with pointer focus that a button press has occurred */
  wlr_seat_pointer_notify_button(server->seat,
                                 event->time_msec, event->button, event->state);
  if (event->state == WL_POINTER_BUTTON_STATE_RELEASED)
  {
    printf("DEBUG: Button released, resetting mode\n");
    /* If you released any buttons, we exit interactive move/resize mode. */
    reset_cursor_mode(server);
  }
  else
  {
    /* Focus that client if the button was _pressed_ */
    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct tinywl_toplevel *toplevel = desktop_toplevel_at(server,
                                                           server->cursor->x, server->cursor->y, &surface, &sx, &sy);
    focus_toplevel(toplevel);
  }
}

void server_cursor_axis(struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits an axis event,
   * for example when you move the scroll wheel. */
  struct tinywl_server *server =
      wl_container_of(listener, server, cursor_axis);
  struct wlr_pointer_axis_event *event = data;
  /* Notify the client with pointer focus of the axis event. */
  wlr_seat_pointer_notify_axis(server->seat,
                               event->time_msec, event->orientation, event->delta,
                               event->delta_discrete, event->source, event->relative_direction);
}

void server_cursor_frame(struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits an frame
   * event. Frame events are sent after regular pointer events to group
   * multiple events together. For instance, two axis events may happen at the
   * same time, in which case a frame event won't be sent in between. */
  struct tinywl_server *server =
      wl_container_of(listener, server, cursor_frame);
  /* Notify the client with pointer focus of the frame event. */
  wlr_seat_pointer_notify_frame(server->seat);
}