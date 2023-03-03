#include "bindings.h"
#include <cluless/core.h>
#include <cluless/core/client.h>
#include <cluless/core/workspace.h>
#include <cluless/misc/companion.h>
#include <config.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

static inline void mouse_move_resize(Monitor *, State);
static inline void move_resize(Monitor *, int, int, int, int);

void quit(Monitor *mon, const Arg *arg)
{
  (void)mon;
  (void)arg;
  core->stop_running();
}

void spawn(Monitor *mon, const Arg *arg)
{
  (void)mon;
  if (fork())
    return;
  if (core->dpy)
    close(ConnectionNumber(core->dpy));
  // 'stdout' maybe redirected to statusbar program, or such, so the only thing
  // we must dump to 'stdout', is status logs.
  close(fileno(stdout));
  close(fileno(stderr));
  setsid();
  execvp(arg->cmd[0], (char **)arg->cmd);
  exit(EXIT_SUCCESS);
}

void swap_master(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  ws_detachclient(mon->selws, c);
  ws_attachclient(mon->selws, c);
  mon_applylayout(mon);
}

void kill_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  if (!core->send_event(c->window, core->wmatoms[WM_DELETE_WINDOW]))
    XKillClient(core->dpy, c->window);
  mon_applylayout(mon);
}

void shift_client(Monitor *mon, const Arg *arg)
{
  int offset = arg->i;
  Client *c  = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  if (offset > 0)
    while (offset--)
      ws_clmovedown(mon->selws, c);
  else
    while (-offset++)
      ws_clmoveup(mon->selws, c);
  mon_applylayout(mon);
}

void shift_focus(Monitor *mon, const Arg *arg)
{
  int offset = arg->i;
  Client *c  = ws_find(mon->selws, ClActive);
  if (!c) {
    c = mon->selws->cl_head;
    goto LAYOUT_AND_EXIT;
  }
  if (offset > 0)
    while (offset--)
      c = c->next ? c->next : mon->selws->cl_head;
  else
    while (-offset++)
      c = c->prev ? c->prev : cl_last(c);
LAYOUT_AND_EXIT:
  mon_focusclient(mon, c);
}

void move_client_to_ws(Monitor *mon, const Arg *arg)
{
  Workspace *from = mon->selws, *to = &mon->wss[arg->i];
  Client *c = ws_find(from, ClActive);
  if (!from || !to || from == to || !c)
    return;
  // if this function is called by a Rule, then the active client might not be
  // focused.
  Client *neighbour = cl_neighbour(c),
         *focused   = ws_getclient(mon->selws, core->input_focused_window());
  mon_focusclient(mon, focused && focused != c ? focused
                       : neighbour             ? neighbour
                                               : from->cl_head);
  // to avoid attaching same client multiple times.
  ws_detachclient(from, c);
  if (!ws_getclient(to, c->window))
    ws_attachclient(to, c);
  // as the client is detached from the 'selws', it wont be destroyed on unmap.
  XUnmapWindow(core->dpy, c->window);
  mon_applylayout(mon);
}

void select_ws(Monitor *mon, const Arg *arg)
{
  Workspace *from = mon->selws, *to = &mon->wss[arg->i];
  if (!from || !to || from == to)
    return;
  // we can unmap safely as 'selws' has already been changed (unmapped client
  // wont be destroyed).
  mon->selws = to;
  companion_insert(companion_remove(from), to);
  Client *c;
  for (c = from->cl_head; c; c = c->next)
    XUnmapWindow(core->dpy, c->window);
  mon_applylayout(mon); // to avoid layout glitches.
  for (c = to->cl_head; c; c = c->next)
    XMapWindow(core->dpy, c->window);
  // If any client with 'ClActive' state exists, then it will be focused by
  // event handlers, otherwise we need to focus some client manually.
  if (!ws_find(to, ClActive))
    mon_focusclient(mon, to->cl_head);
}

void tile_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  UNSET(c->state, CL_UNTILED_STATE);
  mon_applylayout(mon);
}

void float_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  SET(c->state, ClFloating);
  mon_applylayout(mon);
}

void cycle_layout(Monitor *mon, const Arg *arg)
{
  (void)arg;
  lm_nextlayout(&mon->selws->layout_manager);
  mon_applylayout(mon);
}

void reset_layout(Monitor *mon, const Arg *arg)
{
  (void)arg;
  LayoutManager *lm = &mon->selws->layout_manager;
  lm_reset(lm);
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    lm_decorate_client(lm, c);
  mon_applylayout(mon);
}

void mouse_move(Monitor *mon, const Arg *arg)
{
  (void)arg;
  mouse_move_resize(mon, ClMoving);
}

void mouse_resize(Monitor *mon, const Arg *arg)
{
  (void)arg;
  mouse_move_resize(mon, ClResizing);
}

void toggle_gap(Monitor *mon, const Arg *arg)
{
  (void)arg;
  LayoutManager *lm = &mon->selws->layout_manager;
  lm->window_gappx  = lm->window_gappx == 0 ? WindowGapPX : 0;
  lm->screen_gappx  = lm->screen_gappx == 0 ? ScreenGapPX : 0;
  mon_applylayout(mon);
}

void toggle_border(Monitor *mon, const Arg *arg)
{
  (void)arg;
  LayoutManager *lm = &mon->selws->layout_manager;
  lm->borderpx      = BorderPX - lm->borderpx;
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    XSetWindowBorderWidth(core->dpy, c->window, lm->borderpx);
  mon_applylayout(mon);
}

void move_client_x(Monitor *mon, const Arg *arg)
{
  move_resize(mon, arg->i, 0, 0, 0);
}

void move_client_y(Monitor *mon, const Arg *arg)
{
  move_resize(mon, 0, arg->i, 0, 0);
}

void resize_client_x(Monitor *mon, const Arg *arg)
{
  move_resize(mon, 0, 0, arg->i, 0);
}

void resize_client_y(Monitor *mon, const Arg *arg)
{
  move_resize(mon, 0, 0, 0, arg->i);
}

// {{{ Util functions.
static inline void mouse_move_resize(Monitor *mon, State state)
{
  Client *c = mon->grabbed.client;
  if (!c)
    return;
  // set cursor
  int cur = core->cursors[IS_SET(state, ClMoving) ? CurMove : CurResize];
  XChangeWindowAttributes(core->dpy, c->window, CWCursor,
                          &(XSetWindowAttributes){.cursor = cur});
  // set client as floating if the layout is not NULL (floating layout).
  if (lm_getlayout(&mon->selws->layout_manager)->apply)
    SET(state, ClFloating);
  // update client state.
  SET(c->state, state);
  mon_focusclient(mon, c);
}

static inline void move_resize(Monitor *mon, int dx, int dy, int dw, int dh)
{
  Client *c;
  if ((dx + dy + dw + dh) == 0 || !(c = ws_find(mon->selws, ClActive)))
    return;
  if (!IS_SET(c->state, ClFloating)) {
    SET(c->state, ClFloating);
    mon_applylayout(mon);
  }
  XWindowAttributes attrs;
  XGetWindowAttributes(core->dpy, c->window, &attrs);
  XMoveResizeWindow(core->dpy, c->window, attrs.x + dx, attrs.y + dy,
                    attrs.width + dw, attrs.height + dh);
}
// }}}

// vim:fdm=marker
