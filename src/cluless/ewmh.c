#include "ewmh.h"
#include <X11/Xatom.h>
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
