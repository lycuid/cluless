#include "scratchpad.h"
#include <X11/Xatom.h>
#include <cluless/core/workspace.h>
#include <config.h>
#include <stdlib.h>

#define sch_cnt   (1 << 8)
#define sch_at(i) sch_clients[i % sch_cnt]

static Client *sch_clients[sch_cnt], *revert_focus_to = NULL;

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
  // if the sch_client was detached from 'selws', that means it was mapped.
  if (from == mon->selws) {
    XUnmapWindow(core->dpy, sch_client->window);
    mon_focusclient(mon, revert_focus_to ? revert_focus_to : from->cl_head);
    return;
  }
  revert_focus_to = ws_find(mon->selws, ClActive);
  ws_attachclient(mon->selws, sch_client);
  XMapWindow(core->dpy, sch_client->window);
  mon_focusclient(mon, sch_client);
}

void sch_toggle(Monitor *mon, const Arg *arg)
{
  Client *sch_client = sch_at(arg->cmd[0][0]);
  sch_client ? sch_show_hide(mon, sch_client)
             : spawn(mon, &(Arg){.cmd = &arg->cmd[1]});
}

static inline void sch_forget(Window w)
{
  if (revert_focus_to && revert_focus_to->window == w)
    revert_focus_to = NULL;
  FOREACH(Client * *sch, sch_clients)
  {
    if (*sch && (*sch)->window == w && !(*sch = NULL))
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
