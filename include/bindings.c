#include "bindings.h"
#include "base.h"
#include "client.h"
#include "workspace.h"
#include <config.h>
#include <stdbool.h>
#include <unistd.h>

void quit(Monitor *mon, const Arg *arg)
{
  (void)arg;
  mon->ctx->running = false;
}

void swap_master(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  ws_detachclient(mon->selws, c);
  ws_attachclient(mon->selws, c);
  mon_restack(mon);
  mon_arrange(mon);
}

void kill_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c;
  if (!(c = ws_find(mon->selws, ClActive)))
    return;
  if (!send_event(c->window, mon->ctx->wmatoms[WMDeleteWindow]))
    XKillClient(mon->ctx->dpy, c->window);
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
  mon_restack(mon);
  mon_arrange(mon);
}

void shift_focus(Monitor *mon, const Arg *arg)
{
  int offset = arg->i;
  Client *c;
  if (!(c = ws_find(mon->selws, ClActive))) {
    mon_focusclient(mon, mon->selws->cl_head);
    mon_arrange(mon);
    return;
  }
  if (offset > 0)
    while (offset--)
      c = c->next ? c->next : mon->selws->cl_head;
  else
    while (-offset++)
      c = c->prev ? c->prev : cl_last(c);
  mon_focusclient(mon, c);
  mon_arrange(mon);
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
  mon_focusclient(mon, focused && focused != c ? focused
                       : neighbour             ? neighbour
                                               : from->cl_head);
  // to avoid attaching same client multiple times.
  ws_detachclient(from, c);
  if (!ws_getclient(to, c->window))
    ws_attachclient(to, c);
  // as the client is detached from the 'selws', it wont be destroyed on unmap.
  XUnmapWindow(mon->ctx->dpy, c->window);
  mon_arrange(mon);
}

void select_ws(Monitor *mon, const Arg *arg)
{
  Workspace *from = mon->selws, *to = mon_workspaceat(mon, arg->i);
  if (!to || !from || to == from)
    return;
  mon->selws = to;
  // 'selws' has already been changed, the client won't be found in 'selws', on
  // unmap notify event, and hence wont be destroyed.
  Client *c = from->cl_head;
  for (; c; c = c->next)
    XUnmapWindow(mon->ctx->dpy, c->window);
  // map windows in proper order.
  for (c = cl_last(to->cl_head); c; c = c->prev)
    XMapWindow(mon->ctx->dpy, c->window);
  if (!(c = ws_find(to, ClActive)))
    c = to->cl_head;
  mon_focusclient(mon, c);
  mon_arrange(mon);
}

void tile_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  UnSet(c->state, CL_UNTILED_STATE);
  mon_restack(mon);
  mon_arrange(mon);
}

void float_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  Set(c->state, ClFloating);
  mon_restack(mon);
  mon_arrange(mon);
}

void cycle_layout(Monitor *mon, const Arg *arg)
{
  (void)arg;
  // @TODO: dont do this here (instead create a function that changes layout,
  // behind the scene, with out of bound error and stuff).
  mon->selws->layoutidx = (mon->selws->layoutidx + 1) % Length(layouts);
  mon_arrange(mon);
}

void reset_layout(Monitor *mon, const Arg *arg)
{
  (void)arg;
  mon->selws->layoutidx    = 0;
  mon->selws->window_gappx = window_gappx;
  mon->selws->screen_gappx = screen_gappx;
  mon->selws->borderpx     = borderpx;
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    XSetWindowBorderWidth(mon->ctx->dpy, c->window, mon->selws->borderpx);
  mon_arrange(mon);
}

void move_resize(Monitor *mon, const Arg *arg)
{
  int state = arg->i;
  Client *c = mon->grabbed.client;
  if (!c)
    return;
  XSetWindowAttributes attrs = {
      .cursor = mon->ctx->cursors[state == Move ? CurMove : CurResize]};
  XChangeWindowAttributes(mon->ctx->dpy, c->window, CWCursor, &attrs);
  State new_state = state == Move ? ClMoving : ClResizing;
  if (ws_getlayout(mon->selws)->arrange)
    new_state |= ClFloating;
  Set(c->state, new_state);
  mon_focusclient(mon, c);
  mon_restack(mon);
  mon_arrange(mon);
}

void focus_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  mon_focusclient(mon, mon->grabbed.client);
}

void toggle_gap(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Workspace *ws    = mon->selws;
  ws->window_gappx = ws->window_gappx == 0 ? window_gappx : 0;
  ws->screen_gappx = ws->screen_gappx == 0 ? screen_gappx : 0;
  mon_arrange(mon);
}

void toggle_border(Monitor *mon, const Arg *arg)
{
  (void)arg;
  mon->selws->borderpx = mon->selws->borderpx == 0 ? borderpx : 0;
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    XSetWindowBorderWidth(mon->ctx->dpy, c->window, mon->selws->borderpx);
  mon_arrange(mon);
}
