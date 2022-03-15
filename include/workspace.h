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
Client *ws_getclient(Workspace *, Window);   // O(n)
Client *ws_find(Workspace *, State);         // O(n)
void ws_attachclient(Workspace *, Client *); // O(1)
void ws_detachclient(Workspace *, Client *); // O(1)
void ws_clmoveup(Workspace *, Client *);     // O(1)
void ws_clmovedown(Workspace *, Client *);   // O(1)

#define ws_getlayout(ws) (&layouts[(ws)->layoutidx % Length(layouts)])

void ws_dump(Workspace *);

#endif
