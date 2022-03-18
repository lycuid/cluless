#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "base.h"
#include "client.h"
#include "workspace.h"

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

#define mon_drawregion(mon, ws)                                                \
  (Geometry)                                                                   \
  {                                                                            \
    .x = mon->screen.x + ws->screen_gappx,                                     \
    .y = mon->screen.y + ws->screen_gappx,                                     \
    .w = mon->screen.w - (ws->screen_gappx * 2),                               \
    .h = mon->screen.h - (ws->screen_gappx * 2)                                \
  }

#define mon_workspaceat(mon, at) (&mon->wss[at % Length(workspaces)])

void mon_init(Monitor *);
void mon_addclient(Monitor *, Client *);
void mon_removeclient(Monitor *, Client *);
void mon_focusclient(Monitor *, Client *);
void mon_setactive(Monitor *, Client *);
void mon_restack(Monitor *);
void mon_arrange(Monitor *);
void mon_statuslog(Monitor *);

#endif
