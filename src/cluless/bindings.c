#include "bindings.h"
#include <cluless/core.h>
#include <cluless/core/client.h>
#include <cluless/core/workspace.h>
#include <config.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

static inline void move_resize_client(Monitor *, State);

void quit(Monitor *mon, const Arg *arg)
{
  (void)arg;
  mon->running = false;
}

void spawn(Monitor *mon, const Arg *arg)
{
  (void)mon;
  if (fork())
    return;
  if (core->dpy)
    close(ConnectionNumber(core->dpy));
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
  mon->applylayout();
}

void kill_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c;
  if (!(c = ws_find(mon->selws, ClActive)))
    return;
  if (!send_event(c->window, core->wmatoms[WM_DELETE_WINDOW]))
    XKillClient(core->dpy, c->window);
}

void shift_client(Monitor *mon, const Arg *arg)
{
  int offset = arg->i;
  Client *c;
  if (!(c = ws_find(mon->selws, ClActive)))
    return;
  if (offset > 0)
    while (offset--)
      ws_clmovedown(mon->selws, c);
  else
    while (-offset++)
      ws_clmoveup(mon->selws, c);
  mon->applylayout();
}

void shift_focus(Monitor *mon, const Arg *arg)
{
  int offset = arg->i;
  Client *c;
  if (!(c = ws_find(mon->selws, ClActive))) {
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
  mon->focusclient(c);
}

void move_client_to_ws(Monitor *mon, const Arg *arg)
{
  Workspace *from = mon->selws, *to = mon_workspaceat(mon, arg->i);
  Client *c = ws_find(from, ClActive);
  if (!from || !to || from == to || !c)
    return;
  // if this function is called by a Rule, then the active client might not be
  // focused.
  Client *neighbour = cl_neighbour(c),
         *focused   = ws_getclient(mon->selws, input_focused_window());
  mon->focusclient(focused && focused != c ? focused
                   : neighbour             ? neighbour
                                           : from->cl_head);
  // to avoid attaching same client multiple times.
  ws_detachclient(from, c);
  if (!ws_getclient(to, c->window))
    ws_attachclient(to, c);
  // as the client is detached from the 'selws', it wont be destroyed on unmap.
  XUnmapWindow(core->dpy, c->window);
}

void select_ws(Monitor *mon, const Arg *arg)
{
  Workspace *from = mon->selws, *to = mon_workspaceat(mon, arg->i);
  if (!to || !from || to == from)
    return;
  mon->selws = to;
  // we can unmap safely as 'selws' has already been changed (unmapped client
  // wont be destroyed).
  Client *c = from->cl_head;
  for (; c; c = c->next)
    XUnmapWindow(core->dpy, c->window);
  for (c = cl_last(to->cl_head); c; c = c->prev)
    XMapWindow(core->dpy, c->window);

  if (!ws_find(to, ClActive))
    mon->focusclient(to->cl_head);
}

void tile_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  UNSET(c->state, CL_UNTILED_STATE);
  mon->applylayout();
}

void float_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  SET(c->state, ClFloating);
  mon->applylayout();
}

void cycle_layout(Monitor *mon, const Arg *arg)
{
  (void)arg;
  lm_nextlayout(&mon->selws->layout_manager);
  mon->applylayout();
}

void reset_layout(Monitor *mon, const Arg *arg)
{
  (void)arg;
  LayoutManager *lm = &mon->selws->layout_manager;
  lm_reset(lm);
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    lm_decorate_client(lm, c);
  mon->applylayout();
}

void move_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  move_resize_client(mon, ClMoving);
}

void resize_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  move_resize_client(mon, ClResizing);
}

void focus_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  mon->focusclient(mon->grabbed.client);
}

void toggle_gap(Monitor *mon, const Arg *arg)
{
  (void)arg;
  LayoutManager *lm = &mon->selws->layout_manager;
  lm->window_gappx  = lm->window_gappx == 0 ? window_gappx : 0;
  lm->screen_gappx  = lm->screen_gappx == 0 ? screen_gappx : 0;
  mon->applylayout();
}

void toggle_border(Monitor *mon, const Arg *arg)
{
  (void)arg;
  LayoutManager *lm = &mon->selws->layout_manager;
  lm->borderpx      = lm->borderpx == 0 ? borderpx : 0;
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    XSetWindowBorderWidth(core->dpy, c->window, lm->borderpx);
  mon->applylayout();
}

static inline void move_resize_client(Monitor *mon, State state)
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
  mon->focusclient(c);
  mon->applylayout();
}
