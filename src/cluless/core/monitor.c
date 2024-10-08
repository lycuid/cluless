#include "monitor.h"
#include <cluless/core.h>
#include <cluless/core/logging.h>
#include <cluless/core/utils/vector.h>
#include <cluless/core/workspace.h>
#include <cluless/misc/window_rule.h>
#include <config.h>
#include <stdlib.h>
#include <string.h>

void mon_init(Monitor *mon)
{
    ITER(workspaces) { vec_append(&mon->workspaces, ws_new(workspaces[it])); }
    mon->curr_ws = 0;
    mon->screen  = core->get_screen_rect();
    memset(&mon->grabbed, 0, sizeof(PointerGrab));
}

void mon_manage_client(Client *c)
{
    if (!c)
        return;
    Monitor *mon = core->mon;
    ws_attachclient(curr_ws(mon), c);
    // just set the client active, without focusing it, as the client isn't
    // mapped yet, and we never know, it might get moved to another
    // workspace via some startup hook. Focusing the client on 'MapNotify'
    // event is much more safe.
    SET(c->state, ClActive);
    XWindowAttributes attrs;
    XGetWindowAttributes(core->dpy, c->window, &attrs);
    XSelectInput(core->dpy, c->window,
                 attrs.your_event_mask | PropertyChangeMask | FocusChangeMask);
    Window parent;
    if (XGetTransientForHint(core->dpy, c->window, &parent))
        SET(c->state, ClFloating);
    window_rule_apply(c);
}

void mon_unmanage_client(Client *c)
{
    Monitor *mon  = core->mon;
    Workspace *ws = mon_get_client_ws(mon, c);
    if (!ws)
        return;
    Client *neighbour = cl_neighbour(c);
    // detaching the client before doing anything else, as the corresponding
    // window has already been destroyed (don't want any excitement).
    ws_detachclient(ws, c);
    if (ws == curr_ws(mon) && IS_SET(c->state, ClActive))
        mon_focusclient(mon, neighbour);
}

void mon_focusclient(Monitor *mon, Client *c)
{
    if (!c)
        goto LAYOUT_AND_EXIT;
    // set client active.
    SET(c->state, ClActive);
    XSetWindowBorder(core->dpy, c->window,
                     IS_SET(c->state, ClCompanion) ? CompanionActive
                                                   : BorderActive);
    XUngrabButton(core->dpy, Button1, 0, c->window);

#define SET_INACTIVE(cl)                                                       \
    {                                                                          \
        UNSET(cl->state, ClActive);                                            \
        XSetWindowBorder(core->dpy, cl->window,                                \
                         IS_SET(cl->state, ClCompanion) ? CompanionInactive    \
                                                        : BorderInactive);     \
        XGrabButton(core->dpy, ButtonForFocus, 0, cl->window, False,           \
                    ButtonMasks, GrabModeAsync, GrabModeAsync, None, None);    \
    }
    for (Client *p = c->prev; p; p = p->prev)
        SET_INACTIVE(p);
    for (Client *n = c->next; n; n = n->next)
        SET_INACTIVE(n);
#undef SET_INACTIVE

    XSetInputFocus(core->dpy, c->window, RevertToParent, CurrentTime);
LAYOUT_AND_EXIT:
    mon_applylayout(mon);
}

void mon_restack(Monitor *mon)
{
    Workspace *ws = curr_ws(mon);
    if (!ws)
        return;
    Client *c = ws->cl_head, *active = NULL;
    if (!c)
        return;

    int floating = 0, fullscreen = 0, i = 0;
    for (; c; c = c->next, i++) {
        if (IS_SET(c->state, ClFloating))
            floating++;
        if (IS_SET(c->state, ClFullscreen))
            fullscreen++;
        if (IS_SET(c->state, ClActive))
            active = c;
    }
    Window *stack = (Window *)malloc(i * sizeof(Window));
    fullscreen += floating, i = 0;
#define AddToStack(c)                                                          \
    stack[IS_SET(c->state, ClFloating)     ? i++                               \
          : IS_SET(c->state, ClFullscreen) ? floating++                        \
                                           : fullscreen++] = c->window;
    if (active)
        AddToStack(active);
    for (c = ws->cl_head; c; c = c->next)
        if (c != active)
            AddToStack(c);
#undef AddToStack
    XRestackWindows(core->dpy, stack, fullscreen);
    free(stack);
}

void mon_applylayout(Monitor *mon)
{
    Workspace *ws = curr_ws(mon);
    if (!ws)
        return;
    const Layout *layout = lm_getlayout(&ws->layout_manager);
    if (layout->apply)
        layout->apply();
    mon_restack(mon);
    log_statuslog();
}

Workspace *mon_get_workspace_at(Monitor *mon, size_t at)
{
    return vec_get(&mon->workspaces, at);
}

Workspace *mon_get_client_ws(Monitor *mon, Client *c)
{
    if (c) {
        ITER(workspaces)
        {
            Workspace *ws = mon_get_workspace_at(mon, it);
            if (ws_getclient(ws, c->window))
                return ws;
        }
    }
    return NULL;
}
