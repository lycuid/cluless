#include "scratchpad.h"
#include "workspace.h"
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static Client *sch_clients[Length(scratchpads)];
static Client *revert_focus_to = NULL;

void sch_create(Monitor *mon, const Arg *arg)
{
  Client *c = ws_find(mon->selws, ClActive);
  Set(c->state, ClFloating);
  sch_clients[arg->i % Length(scratchpads)] = c;
}

void sch_toggle(Monitor *mon, const Arg *arg)
{
  const char *sch_id = arg->cmd[0];
  const char **cmd   = &arg->cmd[1];
  Client *sch_client = NULL;
  for (size_t i = 0; i < Length(scratchpads); ++i) {
    if (strcmp(scratchpads[i], sch_id) == 0) {
      sch_client = sch_clients[i];
      break;
    }
  }
  if (!sch_client) {
    spawn(mon, &(Arg){.cmd = cmd});
    return;
  }

  Workspace *from = NULL;
  for (size_t i = 0; i < Length(workspaces); ++i) {
    from = mon_workspaceat(mon, i);
    if (ws_getclient(from, sch_client->window)) {
      ws_detachclient(from, sch_client);
      break;
    }
  }
  // we have already detached the client. it is safe to unmap client without
  // killing it.
  if (from == mon->selws) {
    XUnmapWindow(mon->ctx->dpy, sch_client->window);
    mon_focusclient(mon, revert_focus_to);
    return;
  }
  revert_focus_to = ws_find(mon->selws, ClActive);
  ws_attachclient(mon->selws, sch_client);
  mon_setactive(mon, sch_client);
  XMapWindow(mon->ctx->dpy, sch_client->window);
}

void sch_clientremove(Monitor *mon, Client *c)
{
  (void)mon;
  for (size_t i = 0; i < Length(scratchpads); ++i)
    if (sch_clients[i] == c)
      sch_clients[i] = NULL;
}
