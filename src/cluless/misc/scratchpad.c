#include "scratchpad.h"
#include <X11/Xatom.h>
#include <cluless/core.h>
#include <cluless/core/workspace.h>
#include <config.h>
#include <stdlib.h>

#define sch_cnt   (1 << 8)
#define sch_at(i) sch_clients[i % sch_cnt]

static Client *sch_clients[sch_cnt];
// ID of the Window that 'might' need to be focused (if exists and present in
// 'selws'), when scratchpad gets hidden/destroyed.
Window revert_focus_hint = 0;

void sch_fromclient(const Arg *arg)
{
    Monitor *mon = core->mon;
    Client *c    = ws_find(mon->selws, ClActive);
    // @NOTE: user can create multiple scratchpad windows with similar id,
    // but we need to make sure that we only track one of them (first one
    // created), for avoiding dangling pointers.
    if (sch_at(arg->i))
        return;
    SET(c->state, ClFloating);
    sch_at(arg->i) = c;
}

static inline void sch_show_hide(Client *sch_client)
{
    Monitor *mon    = core->mon;
    Workspace *from = mon_get_client_ws(mon, sch_client);
    // detach sch_client, if attached to any workspace.
    if (from)
        ws_detachclient(from, sch_client);
    Client *rf_client;
    // if the sch_client was detached from 'selws', that means it was
    // mapped.
    if (from == mon->selws) {
        XUnmapWindow(core->dpy, sch_client->window);
        // 'mon_focusclient' should not be called, if sch_client wasn't
        // focused itself before unmapping.
        if (IS_SET(sch_client->state, ClActive))
            mon_focusclient(
                mon, (rf_client = ws_getclient(mon->selws, revert_focus_hint))
                         ? rf_client
                         : mon->selws->cl_head);
        revert_focus_hint = 0;
    } else {
        if ((rf_client = ws_find(mon->selws, ClActive)))
            revert_focus_hint = rf_client->window;
        ws_attachclient(mon->selws, sch_client);
        XMapWindow(core->dpy, sch_client->window);
        mon_focusclient(mon, sch_client);
    }
}

void sch_toggle(const Arg *arg)
{
    Monitor *mon       = core->mon;
    Client *sch_client = sch_at(arg->cmd[0][0]), *c;
    if (sch_client)
        sch_show_hide(sch_client);
    else {
        if ((c = ws_find(mon->selws, ClActive)))
            revert_focus_hint = c->window;
        spawn(&(Arg){.cmd = &arg->cmd[1]});
    }
}

static inline void sch_remove(Window window)
{
    if (revert_focus_hint == window)
        revert_focus_hint = 0;
    FOREACH(Client * *sch, sch_clients)
    {
        if (*sch && (*sch)->window == window && !(*sch = NULL))
            break;
    }
}

void sch_destroynotify(const XEvent *xevent)
{
    sch_remove(xevent->xdestroywindow.window);
}

void sch_clientremove(Client *c)
{
    if (c)
        sch_remove(c->window);
}
