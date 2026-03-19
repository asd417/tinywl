#ifndef TINYWL_PROCESS_H
#define TINYWL_PROCESS_H

void process_cursor_move(struct tinywl_server *server);
void process_cursor_resize(struct tinywl_server *server);
void process_cursor_motion(struct tinywl_server *server, uint32_t time);

#endif