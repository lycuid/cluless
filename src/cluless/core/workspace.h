#ifndef __CORE__WORKSPACE_H__
#define __CORE__WORKSPACE_H__

#include <cluless/core/client.h>
#include <cluless/layout.h>

typedef struct {
    const char *id;
    Client *cl_head;
    LayoutManager layout_manager;
} Workspace;

void ws_init(Workspace *, const char *);
Client *ws_getclient(Workspace *, Window);
Client *ws_find(Workspace *, State);
void ws_attachclient(Workspace *, Client *);
void ws_detachclient(Workspace *, Client *);
void ws_clmoveup(Workspace *, Client *);
void ws_clmovedown(Workspace *, Client *);

#endif
