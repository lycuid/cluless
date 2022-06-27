#include "scratchpad.h"
#include <cluless/core/workspace.h>
#include <config.h>
#include <stdlib.h>

#define sch_cnt   (1 << 8)
#define sch_at(i) sch_clients[i % sch_cnt]

static Client *sch_clients[sch_cnt];
static Client *revert_focus_to = NULL;

void sch_fromclient(Monitor *mon, const Arg *arg)
{
  Client *c = ws_find(mon->selws, ClActive);
  Set(c->state, ClFloating);
  // @NOTE: user can create multiple scratchpad windows with similar id, but we
  // need to make sure that we only track one of them (first one created), for
  // avoiding dangling pointers.
  if (!sch_at(arg->i))
    sch_at(arg->i) = c;
}

void sch_toggle(Monitor *mon, const Arg *arg)
{
  Client *sch_client = sch_at(arg->cmd[0][0]);
  if (!sch_client) {
    spawn(mon, &(Arg){.cmd = &arg->cmd[1]});
    return;
  }
  Workspace *from = mon_get_client_ws(mon, sch_client);
  // detach sch_client, if attached to any workspace.
  if (from)
    ws_detachclient(from, sch_client);
  // if the sch_client was detached from 'selws', that means it was mapped.
  if (from == mon->selws) {
    XUnmapWindow(mon->ctx->dpy, sch_client->window);
    mon_focusclient(mon, revert_focus_to ? revert_focus_to : from->cl_head);
    return;
  }
  revert_focus_to = ws_find(mon->selws, ClActive);
  ws_attachclient(mon->selws, sch_client);
  XMapWindow(mon->ctx->dpy, sch_client->window);
  mon_focusclient(mon, sch_client);
}

static inline void sch_nullify_local_pointer(Window w)
{
  if (revert_focus_to && revert_focus_to->window == w) {
    revert_focus_to = NULL;
    return;
  }
  for (size_t i = 0; i < sch_cnt; ++i) {
    if (sch_clients[i] && sch_clients[i]->window == w) {
      sch_clients[i] = NULL;
      return;
    }
  }
}

// @NOTE: 'ClientRemove' hook will be called on a client if, and only if, the
// client is attached to a workspace.
// If the scratchpad window gets destroyed while not being attached to any
// workspace, the window will no longer exist, but the corresponding
// 'sch_client' pointer will (dangling pointer).
// So we cannot rely on 'ClientRemove' hook alone to nullify the 'sch_client'
// pointer.
void sch_destroynotify(Monitor *mon, const XEvent *xevent)
{
  (void)mon;
  sch_nullify_local_pointer(xevent->xdestroywindow.window);
}

void sch_clientremove(Monitor *mon, Client *c)
{
  (void)mon;
  if (c)
    sch_nullify_local_pointer(c->window);
}
