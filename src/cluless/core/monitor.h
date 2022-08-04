#ifndef __CORE__MONITOR_H__
#define __CORE__MONITOR_H__

#include <cluless/core.h>
#include <cluless/core/client.h>
#include <cluless/core/workspace.h>
#include <stdbool.h>

typedef struct {
  Client *client;
  int x, y, cx, cy, cw, ch;
  State state;
  Time at;
} PointerGrab;

typedef struct {
  bool running : 1;
  Workspace *wss, *selws;
  Geometry screen;
  PointerGrab grabbed;

  void (*focusclient)(Client *);
  void (*restack)(void);
  void (*applylayout)(void);
  Workspace *(*get_client_ws)(Client *);
  void (*statuslog)(void);
} Monitor;

Monitor *mon_init();
void mon_manage_client(Monitor *, Client *);
void mon_unmanage_client(Monitor *, Client *);

// hooks are only called on clients which are attached to the workspaces managed
// my the monitor.
ENUM(HookType, ClientAdd, ClientRemove);
typedef void (*ClientHook)(Monitor *, Client *);
typedef void (*EventHandler)(Monitor *, const XEvent *);

#endif
