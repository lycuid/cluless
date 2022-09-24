#include "ewmh.h"
#include <X11/Xatom.h>
#include <config.h>
#include <stdint.h>
#include <stdlib.h>

void ewmh_maprequest(Monitor *mon, const XEvent *xevent)
{
  (void)mon;
  const XMapRequestEvent *e = &xevent->xmaprequest;
  XWindowAttributes attrs;
  XGetWindowAttributes(core->dpy, e->window, &attrs);
  if (!IS_SET(attrs.your_event_mask, FocusChangeMask))
    XSelectInput(core->dpy, e->window, attrs.your_event_mask | FocusChangeMask);
  XChangeProperty(core->dpy, core->root, core->netatoms[NET_CLIENT_LIST],
                  XA_WINDOW, 32, PropModeAppend, (uint8_t *)&e->window, 1);
}

static inline void update_client_list(Monitor *mon)
{
  XDeleteProperty(core->dpy, core->root, core->netatoms[NET_CLIENT_LIST]);
  Window *managed_windows;
  int window_count = 0;
  ITER(workspaces)
  {
    for (Client *c = mon->wss[it].cl_head; c; c = c->next)
      window_count++;
  }
  managed_windows = malloc(window_count * sizeof(Window));
  window_count    = 0;
  ITER(workspaces)
  {
    for (Client *c = mon->wss[it].cl_head; c; c = c->next)
      managed_windows[window_count++] = c->window;
  }
  XChangeProperty(core->dpy, core->root, core->netatoms[NET_CLIENT_LIST],
                  XA_WINDOW, 32, PropModeReplace, (uint8_t *)managed_windows,
                  window_count);
  free(managed_windows);
}

void ewmh_destroynotify(Monitor *mon, const XEvent *xevent)
{
  (void)xevent;
  update_client_list(mon);
}

void ewmh_clientremove(Monitor *mon, Client *c)
{
  (void)c;
  update_client_list(mon);
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
