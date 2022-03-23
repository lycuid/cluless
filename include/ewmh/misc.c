#include "misc.h"
#include <X11/Xatom.h>
#include <config.h>
#include <stdint.h>

static uint32_t ClientCount = 0;

void ewmh_clientadd(Monitor *mon, Client *c)
{
  if (!c)
    return;
  XSelectInput(mon->ctx->dpy, c->window, FocusChangeMask);
  XChangeProperty(mon->ctx->dpy, mon->ctx->root,
                  mon->ctx->netatoms[NetClientList], XA_WINDOW, 32,
                  PropModeAppend, (uint8_t *)&c->window, 1);
  ClientCount++;
}

void ewmh_clientremove(Monitor *mon, Client *c)
{
  (void)c;
  Window wids[ClientCount];
  ClientCount = 0;
  for (size_t i = 0; i < Length(workspaces); ++i) {
    Workspace *ws = mon_workspaceat(mon, i);
    for (Client *c = ws->cl_head; c; c = c->next)
      wids[ClientCount++] = c->window;
  }
  XChangeProperty(mon->ctx->dpy, mon->ctx->root,
                  mon->ctx->netatoms[NetClientList], XA_WINDOW, 32,
                  PropModeReplace, (uint8_t *)wids, ClientCount);
}

void ewmh_focusin(Monitor *mon, const XEvent *xevent)
{
  const XFocusOutEvent *e = &xevent->xfocus;
  Client *c;
  if (!(c = ws_getclient(mon->selws, e->window)))
    return;
  XChangeProperty(mon->ctx->dpy, mon->ctx->root,
                  mon->ctx->netatoms[NetActiveWindow], XA_WINDOW, 32,
                  PropModeReplace, (uint8_t *)&c->window, 1);
}

void ewmh_focusout(Monitor *mon, const XEvent *xevent)
{
  (void)xevent;
  XDeleteProperty(mon->ctx->dpy, mon->ctx->root,
                  mon->ctx->netatoms[NetActiveWindow]);
}
