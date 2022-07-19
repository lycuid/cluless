#ifndef __CORE__MONITOR_H__
#define __CORE__MONITOR_H__

#include <cluless/core.h>
#include <cluless/core/client.h>
#include <cluless/core/workspace.h>

typedef struct {
  Client *client;
  int x, y, cx, cy, cw, ch;
  State state;
  Time at;
} PointerGrab;

typedef struct {
  Context *ctx;
  Workspace *wss;
  Workspace *selws; // This should never be 'NULL' :dansgame:.
  Geometry screen;
  PointerGrab grabbed;
} Monitor;

// hooks are only called on clients which are attached to the workspaces managed
// my the monitor.
ENUM(HookType, ClientAdd, ClientRemove);
typedef void (*ClientHook)(Monitor *, Client *);
typedef void (*EventHandler)(Monitor *, const XEvent *);

#define mon_workspaceat(mon, at) (&mon->wss[at % LENGTH(workspaces)])

void mon_init(Monitor *);
// client is added to some workspace in the monitor.
void mon_manage_client(Monitor *, Client *);
// client is removed from any/all workspaces in the monitor.
void mon_unmanage_client(Monitor *, Client *);
void mon_focusclient(Monitor *, Client *);
void mon_restack(Monitor *);
void mon_applylayout(Monitor *);
// returns workspace associated with given client.
Workspace *mon_get_client_ws(Monitor *, Client *);
void mon_statuslog(Monitor *);

#endif
