#ifndef __BINDINGS_H__
#define __BINDINGS_H__

#include <cluless/core/monitor.h>
#include <stdint.h>

typedef union {
  int i;
  const char **cmd;
} Arg;

typedef void (*CustomHandler)(Monitor *, const Arg *);

typedef struct {
  uint64_t mask, sym;
  CustomHandler func;
  Arg arg;
} Binding;

// key bindings.
void quit(Monitor *, const Arg *);
void spawn(Monitor *, const Arg *);
void swap_master(Monitor *, const Arg *);
void kill_client(Monitor *, const Arg *);
void shift_client(Monitor *, const Arg *);
void shift_focus(Monitor *, const Arg *);
void move_client_to_ws(Monitor *, const Arg *);
void select_ws(Monitor *, const Arg *);
void tile_client(Monitor *, const Arg *);
void float_client(Monitor *, const Arg *);
void cycle_layout(Monitor *, const Arg *);
void reset_layout(Monitor *, const Arg *);
void toggle_gap(Monitor *, const Arg *);
void toggle_border(Monitor *, const Arg *);
void move_client_x(Monitor *, const Arg *);
void move_client_y(Monitor *, const Arg *);
void resize_client_x(Monitor *, const Arg *);
void resize_client_y(Monitor *, const Arg *);

// button bindings.
void mouse_move(Monitor *, const Arg *);
void mouse_resize(Monitor *, const Arg *);
void focus_client(Monitor *, const Arg *);

#endif
