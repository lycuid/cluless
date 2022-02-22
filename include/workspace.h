#ifndef __WORKSPACE_H__
#define __WORKSPACE_H__

#include "base.h"
#include "client.h"

typedef struct {
  const char *id;
  Client *cl_head;
  uint32_t screen_gappx, window_gappx;
  uint32_t borderpx, border_active, border_inactive;
  uint32_t layoutidx;
} Workspace;

void ws_init(Workspace *, const char *);
Client *ws_getclient(Workspace *, Window);      // O(n)
Client *ws_getactive(Workspace *);              // O(n)
void ws_attachclient(Workspace *, Client *);    // O(1)
Client *ws_detachclient(Workspace *, Window);   // O(n)
void ws_client_moveup(Workspace *, Client *);   // O(1)
void ws_client_movedown(Workspace *, Client *); // O(1)
const Layout *ws_getlayout(Workspace *);

void ws_dump(Workspace *);

#endif
