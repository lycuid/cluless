#ifndef __CORE__MONITOR_H__
#define __CORE__MONITOR_H__

#include <cluless/core/client.h>
#include <cluless/core/workspace.h>

typedef struct {
    int x, y;
    Client *client;
    Geometry cl_geometry;
    State state;
    Time at;
} PointerGrab;

typedef struct {
    Workspace *wss, *selws;
    Geometry screen;
    PointerGrab grabbed;
} Monitor;

void mon_init(Monitor *);
void mon_focusclient(Monitor *, Client *);
void mon_restack(Monitor *);
void mon_applylayout(Monitor *);
Workspace *mon_get_client_ws(Monitor *, Client *);

void mon_manage_client(Client *);
void mon_unmanage_client(Client *);

#endif
