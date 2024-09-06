#ifndef __BINDINGS_H__
#define __BINDINGS_H__

#include <cluless/core/monitor.h>
#include <stdint.h>

typedef union {
    int i;
    const char **cmd;
} Arg;

typedef void (*Action)(const Arg *);

typedef struct {
    uint64_t mask, sym;
    Action action;
    Arg arg;
} Binding;

// key bindings.
void quit(const Arg *);
void spawn(const Arg *);
void swap_master(const Arg *);
void kill_client(const Arg *);
void shift_client(const Arg *);
void shift_focus(const Arg *);
void change_workspace(const Arg *);
void transfer_client_to(const Arg *);
void select_ws(const Arg *);
void tile_client(const Arg *);
void float_client(const Arg *);
void cycle_layout(const Arg *);
void reset_layout(const Arg *);
void toggle_gap(const Arg *);
void toggle_border(const Arg *);
void move_client_x(const Arg *);
void move_client_y(const Arg *);
void resize_client_x(const Arg *);
void resize_client_y(const Arg *);

// button bindings.
void mouse_move(const Arg *);
void mouse_resize(const Arg *);

#endif
