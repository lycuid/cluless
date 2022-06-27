#include "ewmh.h"
#include <X11/Xatom.h>
#include <config.h>
#include <stdint.h>

// @TODO: figure out a better way of doing this.
// currently we are updating the client list on every window map and destroy.
static inline void update_client_list(Monitor *mon)
{
  uint32_t i = 0, managed_client_count = 0;
  for (i = 0; i < Length(workspaces); ++i)
    for (Client *c = mon_workspaceat(mon, i)->cl_head; c; c = c->next)
      managed_client_count++;

  Window wids[managed_client_count];
  for (i = 0, managed_client_count = 0; i < Length(workspaces); ++i)
    for (Client *c = mon_workspaceat(mon, i)->cl_head; c; c = c->next)
      wids[managed_client_count++] = c->window;

  XChangeProperty(mon->ctx->dpy, mon->ctx->root, mon->ctx->atoms[NetClientList],
                  XA_WINDOW, 32, PropModeReplace, (uint8_t *)wids,
                  managed_client_count);
}

// @TODO: find a better way to do this.
// currently adding focuschange mask everytime a window is mapped (which is
// unecessary, only need to do this once).
void ewmh_maprequest(Monitor *mon, const XEvent *xevent)
{
  const XMapRequestEvent *e = &xevent->xmaprequest;
  XWindowAttributes attrs;
  XGetWindowAttributes(mon->ctx->dpy, e->window, &attrs);
  if (!IsSet(attrs.your_event_mask, FocusChangeMask))
    XSelectInput(mon->ctx->dpy, e->window,
                 attrs.your_event_mask | FocusChangeMask);
  // map request only fires on windows with 'override_redirect' set to false.
  update_client_list(mon);
}

void ewmh_destroynotify(Monitor *mon, const XEvent *xevent)
{
  (void)xevent;
  update_client_list(mon);
}

void ewmh_focusin(Monitor *mon, const XEvent *xevent)
{
  const XFocusOutEvent *e = &xevent->xfocus;
  Client *c;
  if (!(c = ws_getclient(mon->selws, e->window)))
    return;
  XChangeProperty(mon->ctx->dpy, mon->ctx->root,
                  mon->ctx->atoms[NetActiveWindow], XA_WINDOW, 32,
                  PropModeReplace, (uint8_t *)&c->window, 1);
}

void ewmh_focusout(Monitor *mon, const XEvent *xevent)
{
  (void)xevent;
  XDeleteProperty(mon->ctx->dpy, mon->ctx->root,
                  mon->ctx->atoms[NetActiveWindow]);
}
