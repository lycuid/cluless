#ifndef __CORE__WORKSPACE_H__
#define __CORE__WORKSPACE_H__

#include <include/core/client.h>
#include <include/layout.h>

typedef struct {
  const char *id;
  Client *cl_head;
  LayoutManager layout_manager;
} Workspace;

void ws_init(Workspace *, const char *);
Client *ws_getclient(Workspace *, Window);   // O(n)
Client *ws_find(Workspace *, State);         // O(n)
void ws_attachclient(Workspace *, Client *); // O(1)
void ws_detachclient(Workspace *, Client *); // O(1)
void ws_clmoveup(Workspace *, Client *);     // O(1)
void ws_clmovedown(Workspace *, Client *);   // O(1)

#endif
