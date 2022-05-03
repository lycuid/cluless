#include "scratchpad.h"
#include <config.h>
#include <include/core/workspace.h>

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

  // detach sch_client, if attached to any workspace.
  Workspace *from = NULL;
  for (size_t i = 0; i < Length(workspaces); ++i) {
    from = mon_workspaceat(mon, i);
    if (ws_getclient(from, sch_client->window)) {
      ws_detachclient(from, sch_client);
      break;
    }
    from = NULL;
  }
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

void sch_clientremove(Monitor *mon, Client *c)
{
  (void)mon;
  for (size_t i = 0; i < sch_cnt; ++i)
    if (sch_clients[i] == c)
      sch_clients[i] = NULL;
}
