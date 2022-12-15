#include "ewmh.h"
#include <X11/Xatom.h>
#include <cluless/layout.h>
#include <config.h>
#include <stdint.h>
#include <stdlib.h>

static inline void update_client_list(Window removed)
{
  XDeleteProperty(core->dpy, core->root, core->netatoms[NET_CLIENT_LIST]);
  FOREACH_AVAILABLE_CLIENT(const Client *c)
  {
    if (c->window != removed)
      XChangeProperty(core->dpy, core->root, core->netatoms[NET_CLIENT_LIST],
                      XA_WINDOW, 32, PropModeAppend, (uint8_t *)&c->window, 1);
  }
}

static inline void handle_bypass_compositor(Monitor *mon, Client *c)
{
  if (!c)
    return;
  uint32_t *value = NULL;
  core->get_window_property(c->window, core->netatoms[NET_WM_BYPASS_COMPOSITOR],
                            sizeof(uint32_t), (uint8_t **)&value);
  if (!value)
    return;
  Workspace *ws = mon_get_client_ws(mon, c);
  if (ws) {
    if (*value == 1) {
      SET(c->state, ClBypassed);
      lm_undecorate_client(&ws->layout_manager, c);
    } else {
      UNSET(c->state, ClBypassed);
      lm_decorate_client(&ws->layout_manager, c);
    }
  }
  mon_applylayout(mon);
  XFree(value);
}

void ewmh_maprequest(Monitor *mon, const XEvent *xevent)
{
  (void)mon;
  const XMapRequestEvent *e = &xevent->xmaprequest;
  XWindowAttributes attrs;
  XGetWindowAttributes(core->dpy, e->window, &attrs);
  if (!IS_SET(attrs.your_event_mask, FocusChangeMask))
    XSelectInput(core->dpy, e->window, attrs.your_event_mask | FocusChangeMask);
  update_client_list(0);
}

void ewmh_clientadd(Monitor *mon, Client *c)
{
  handle_bypass_compositor(mon, c);
}

void ewmh_propertynotify(Monitor *mon, const XEvent *xevent)
{
  const XPropertyEvent *e = &xevent->xproperty;
  if (e->atom != core->netatoms[NET_WM_BYPASS_COMPOSITOR])
    return;
  Client *c = NULL;
  ITER(workspaces)
  {
    if ((c = ws_getclient(&mon->wss[it], e->window)))
      break;
  }
  handle_bypass_compositor(mon, c);
}

void ewmh_destroynotify(Monitor *mon, const XEvent *xevent)
{
  (void)mon;
  update_client_list(xevent->xdestroywindow.window);
}

void ewmh_clientremove(Monitor *mon, Client *c)
{
  (void)mon;
  update_client_list(c ? c->window : 0);
}

void ewmh_focusin(Monitor *mon, const XEvent *xevent)
{
  const XFocusOutEvent *e = &xevent->xfocus;
  Client *c;
  if (!(c = ws_getclient(mon->selws, e->window)))
    return;
  XChangeProperty(core->dpy, core->root, core->netatoms[NET_ACTIVE_WINDOW],
                  XA_WINDOW, 32, PropModeReplace, (uint8_t *)&c->window, 1);
}

void ewmh_focusout(Monitor *mon, const XEvent *xevent)
{
  (void)mon;
  (void)xevent;
  XDeleteProperty(core->dpy, core->root, core->netatoms[NET_ACTIVE_WINDOW]);
}
