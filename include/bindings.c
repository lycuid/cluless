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
  Client *c = ws_getactive(mon->selws);
  if (!c || !(c = ws_detachclient(mon->selws, c->window)))
    return;
  ws_attachclient(mon->selws, c);
  mon_restack(mon);
  mon_arrange(mon);
}

void kill_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c;
  if (!(c = ws_getactive(mon->selws)))
    return;
  if (!send_event(c->window, mon->ctx->wmatoms[WMDeleteWindow]))
    XKillClient(mon->ctx->dpy, c->window);
}

void shift_client(Monitor *mon, const Arg *arg)
{
  int offset = arg->i;
  Client *c;
  if (!(c = ws_getactive(mon->selws)))
    return;
  if (offset > 0)
    while (offset--)
      ws_client_movedown(mon->selws, c);
  else
    while (-offset++)
      ws_client_moveup(mon->selws, c);
  mon_restack(mon);
  mon_arrange(mon);
}

void shift_focus(Monitor *mon, const Arg *arg)
{
  int offset = arg->i;
  Client *c;
  if (!(c = ws_getactive(mon->selws))) {
    if (mon->selws->cl_head) {
      mon_focusclient(mon, mon->selws->cl_head);
      mon_arrange(mon);
    }
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
  if (!from || !to || from == to)
    return;
  Client *c = ws_getactive(from);
  if ((c = ws_detachclient(from, c->window))) {
    mon_focusclient(mon, cl_neighbour(c));
    ws_attachclient(to, c);
    LockRootEvents();
    XUnmapWindow(mon->ctx->dpy, c->window);
    UnlockRootEvents();
  }
  mon_arrange(mon);
}

void select_ws(Monitor *mon, const Arg *arg)
{
  Workspace *from = mon->selws, *to = mon_workspaceat(mon, arg->i);
  if (!to || !from || to == from)
    return;
  mon->selws = to;

  LockRootEvents();
  Client *c = from->cl_head;
  for (; c; c = c->next)
    XUnmapWindow(mon->ctx->dpy, c->window);
  UnlockRootEvents();

  // map windows in the 'cl_head' order.
  for (c = cl_last(to->cl_head); c; c = c->prev)
    XMapRaised(mon->ctx->dpy, c->window);
  mon_arrange(mon);
}

void tile_client(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_getactive(mon->selws);
  if (!c)
    return;
  UnSet(c->state, ClFloating);
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
  uint32_t mask = state == Move ? ClMoving : ClResizing;
  if (ws_getlayout(mon->selws)->arrange)
    mask |= ClFloating;
  Set(c->state, mask);
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

void float_client(Monitor *mon, const Arg *arg)
{
  (void)mon;
  (void)arg;
}
