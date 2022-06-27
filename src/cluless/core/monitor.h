#ifndef __CORE__MONITOR_H__
#define __CORE__MONITOR_H__

#include <cluless/core.h>
#include <cluless/core/client.h>
#include <cluless/core/workspace.h>

typedef struct {
  Client *client;
  int x, y, cx, cy, cw, ch;
  uint32_t mask;
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
typedef enum { ClientAdd, ClientRemove, NullHook } client_hook_t;
typedef void (*ClientHook)(Monitor *, Client *);
typedef void (*EventHandler)(Monitor *, const XEvent *);

#define mon_workspaceat(mon, at) (&mon->wss[at % Length(workspaces)])

void mon_init(Monitor *);
// client that can be reached by the monitor (present in some workspace in
// the monitor 'mon->wss').
void mon_manage_client(Monitor *, Client *);
// remove client from any/all workspaces (unreachable client).
void mon_unmanage_client(Monitor *, Client *);
void mon_focusclient(Monitor *, Client *);
void mon_restack(Monitor *);
void mon_applylayout(Monitor *);
// returns workspace associated with given client.
Workspace *mon_get_client_ws(Monitor *, Client *);
void mon_statuslog(Monitor *);

#endif
