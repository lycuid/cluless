#include "scratchpad.h"
#include <X11/Xatom.h>
#include <cluless/core/workspace.h>
#include <config.h>
#include <stdlib.h>

#define sch_cnt   (1 << 8)
#define sch_at(i) sch_clients[i % sch_cnt]

static Client *sch_clients[sch_cnt];
// ID of the Window that 'might' need to be focused (if exists and present in
// 'selws'), when scratchpad gets hidden/destroyed.
Window revert_focus_hint = 0;

void sch_fromclient(Monitor *mon, const Arg *arg)
{
  Client *c = ws_find(mon->selws, ClActive);
  SET(c->state, ClFloating);
  // @NOTE: user can create multiple scratchpad windows with similar id, but we
  // need to make sure that we only track one of them (first one created), for
  // avoiding dangling pointers.
  if (!sch_at(arg->i))
    sch_at(arg->i) = c;
}

static inline void sch_show_hide(Monitor *mon, Client *sch_client)
{
  Workspace *from = mon_get_client_ws(mon, sch_client);
  // detach sch_client, if attached to any workspace.
  if (from)
    ws_detachclient(from, sch_client);
  Client *rf_client;
  // if the sch_client was detached from 'selws', that means it was mapped.
  if (from == mon->selws) {
    XUnmapWindow(core->dpy, sch_client->window);
    // 'mon_focusclient' should not be called, if sch_client wasn't focused
    // itself before unmapping.
    if (IS_SET(sch_client->state, ClActive))
      mon_focusclient(mon,
                      (rf_client = ws_getclient(mon->selws, revert_focus_hint))
                          ? rf_client
                          : mon->selws->cl_head);
    revert_focus_hint = 0;
  } else {
    if ((rf_client = ws_find(mon->selws, ClActive)))
      revert_focus_hint = rf_client->window;
    ws_attachclient(mon->selws, sch_client);
    XMapWindow(core->dpy, sch_client->window);
    mon_focusclient(mon, sch_client);
  }
}

void sch_toggle(Monitor *mon, const Arg *arg)
{
  Client *sch_client = sch_at(arg->cmd[0][0]), *c;
  if (sch_client)
    sch_show_hide(mon, sch_client);
  else {
    if ((c = ws_find(mon->selws, ClActive)))
      revert_focus_hint = c->window;
    spawn(mon, &(Arg){.cmd = &arg->cmd[1]});
  }
}

static inline void sch_forget(Window window)
{
  if (revert_focus_hint == window)
    revert_focus_hint = 0;
  FOREACH(Client * *sch, sch_clients)
  {
    if (*sch && (*sch)->window == window && !(*sch = NULL))
      break;
  }
}

void sch_destroynotify(Monitor *mon, const XEvent *xevent)
{
  (void)mon;
  sch_forget(xevent->xdestroywindow.window);
}

void sch_clientremove(Monitor *mon, Client *c)
{
  (void)mon;
  if (c)
    sch_forget(c->window);
}
