#include "config.h"
#include <X11/Xlib.h>
#include <include/core.h>
#include <include/core/client.h>
#include <include/core/monitor.h>
#include <include/core/workspace.h>
#include <include/ewmh.h>
#include <include/ewmh/docks.h>
#include <include/scratchpad.h>
#include <include/window_rule.h>
#include <stdlib.h>
#include <string.h>

static inline void ManageClientHook(client_hook_t, Monitor *, Client *);

void onMapRequest(Monitor *, const XEvent *);
void onMapNotify(Monitor *, const XEvent *);
void onUnmapNotify(Monitor *, const XEvent *);
void onConfigureRequest(Monitor *, const XEvent *);
void onPropertyNotify(Monitor *, const XEvent *);
void onKeyPress(Monitor *, const XEvent *);
void onButtonPress(Monitor *, const XEvent *);
void onMotionNotify(Monitor *, const XEvent *);
void onButtonRelease(Monitor *, const XEvent *);
void onDestroyNotify(Monitor *, const XEvent *);

static const ClientHook default_client_hooks[NullHook] = {
    [ClientAdd] = mon_manage_client, [ClientRemove] = mon_unmanage_client};

static const EventHandler default_event_handlers[LASTEvent] = {
    [MapRequest]       = onMapRequest,
    [MapNotify]        = onMapNotify,
    [UnmapNotify]      = onUnmapNotify,
    [ConfigureRequest] = onConfigureRequest,
    [PropertyNotify]   = onPropertyNotify,
    [KeyPress]         = onKeyPress,
    [ButtonPress]      = onButtonPress,
    [MotionNotify]     = onMotionNotify,
    [ButtonRelease]    = onButtonRelease,
    [DestroyNotify]    = onDestroyNotify};

static inline void ManageClientHook(client_hook_t type, Monitor *mon, Client *c)
{
  if (type != ClientRemove && default_client_hooks[type])
    default_client_hooks[type](mon, c);
  if (sch_client_hooks[type])
    sch_client_hooks[type](mon, c);
  if (type == ClientRemove && default_client_hooks[type])
    default_client_hooks[type](mon, c);
  if (type == ClientRemove && c)
    free(c);
}

void onMapRequest(Monitor *mon, const XEvent *xevent)
{
  const XMapRequestEvent *e = &xevent->xmaprequest;
  Client *c;
  if (!(c = ws_getclient(mon->selws, e->window))) {
    c = cl_create(e->window);
    ManageClientHook(ClientAdd, mon, c);
    window_rule_apply(mon, c);
  }
  // client might be moved to another workspace by a WindowRule, so we only map
  // the window if the client is found in selws.
  if (!ws_getclient(mon->selws, c->window))
    return;
  Window w;
  if (XGetTransientForHint(mon->ctx->dpy, c->window, &w))
    Set(c->state, ClTransient);
  mon_restack(mon);
  mon_applylayout(mon);
  XMapWindow(mon->ctx->dpy, c->window);
}

void onMapNotify(Monitor *mon, const XEvent *xevent)
{
  const XMapEvent *e = &xevent->xmap;
  EVENT("MapNotify on window: %lu.\n", e->window);
  Client *c;
  if ((c = ws_getclient(mon->selws, e->window)) && IsSet(c->state, ClActive))
    mon_focusclient(mon, c);
}

void onUnmapNotify(Monitor *mon, const XEvent *xevent)
{
  const XUnmapEvent *e = &xevent->xunmap;
  EVENT("UnmapNotify on window: %lu.\n", e->window);
  Client *c;
  if ((c = ws_getclient(mon->selws, e->window)))
    ManageClientHook(ClientRemove, mon, c);
}

void onConfigureRequest(Monitor *mon, const XEvent *xevent)
{
  const XConfigureRequestEvent *e = &xevent->xconfigurerequest;
  EVENT("ConfigureRequest on window: %lu.\n", e->window);
  XWindowChanges changes = {.x            = e->x,
                            .y            = e->y,
                            .width        = e->width,
                            .height       = e->height,
                            .border_width = e->border_width,
                            .sibling      = e->above,
                            .stack_mode   = e->detail};
  XConfigureWindow(mon->ctx->dpy, e->window, e->value_mask, &changes);
  XSync(mon->ctx->dpy, False);
  mon_applylayout(mon);
}

void onPropertyNotify(Monitor *mon, const XEvent *xevent)
{
  const XPropertyEvent *e = &xevent->xproperty;
  EVENT("PropertyNotify on window: %lu.\n", e->window);
  if (e->state == PropertyNewValue && (e->atom == mon->ctx->atoms[NetWMName] ||
                                       e->atom == mon->ctx->atoms[WMName]))
    mon_statuslog(mon);
}

void onKeyPress(Monitor *mon, const XEvent *xevent)
{
  const XKeyEvent *e = &xevent->xkey;
  for (size_t i = 0; i < Length(keys); ++i)
    if (e->keycode == XKeysymToKeycode(mon->ctx->dpy, keys[i].sym) &&
        e->state == keys[i].mask)
      keys[i].func(mon, &keys[i].arg);
}

void onButtonPress(Monitor *mon, const XEvent *xevent)
{
  const XButtonEvent *e = &xevent->xbutton;
  XGrabPointer(mon->ctx->dpy, mon->ctx->root, False,
               ButtonMasks | PointerMotionMask, GrabModeAsync, GrabModeAsync,
               None, None, CurrentTime);
  Client *c;
  memset(&mon->grabbed, 0, sizeof(mon->grabbed));
  if ((c = ws_getclient(mon->selws, e->subwindow))) {
    int mask = e->button == Button1   ? Button1Mask
               : e->button == Button2 ? Button2Mask
               : e->button == Button3 ? Button3Mask
               : e->button == Button4 ? Button4Mask
               : e->button == Button5 ? Button5Mask
                                      : 0;

    XWindowAttributes wa;
    XGetWindowAttributes(mon->ctx->dpy, c->window, &wa);
    mon->grabbed = (PointerGrab){.client = c,
                                 .x      = e->x_root,
                                 .y      = e->y_root,
                                 .cx     = wa.x,
                                 .cy     = wa.y,
                                 .cw     = wa.width,
                                 .ch     = wa.height,
                                 .mask   = e->state | mask,
                                 .at     = e->time};
  }
  for (size_t i = 0; i < Length(buttons); ++i)
    if (e->button == buttons[i].sym && e->state == buttons[i].mask)
      buttons[i].func(mon, &buttons[i].arg);
}

void onMotionNotify(Monitor *mon, const XEvent *xevent)
{
  const XMotionEvent *e = &xevent->xmotion;
  PointerGrab *grabbed  = &mon->grabbed;
  Client *c             = grabbed->client;
  if (!c || e->state != grabbed->mask || (e->time - grabbed->at) < (1000 / 60))
    return;

  int dx = e->x - grabbed->x, dy = e->y - grabbed->y;
  if (IsSet(c->state, ClMoving))
    XMoveWindow(mon->ctx->dpy, c->window, grabbed->cx + dx, grabbed->cy + dy);
  else if (IsSet(c->state, ClResizing))
    XResizeWindow(mon->ctx->dpy, c->window, Max(grabbed->cw + dx, c->minw),
                  Max(grabbed->ch + dy, c->minh));
  grabbed->at = e->time;
}

void onButtonRelease(Monitor *mon, const XEvent *xevent)
{
  (void)xevent;
  Client *c;
  if (!(c = mon->grabbed.client))
    return;
  XSetWindowAttributes attrs = {.cursor = mon->ctx->cursors[CurNormal]};
  XChangeWindowAttributes(mon->ctx->dpy, c->window, CWCursor, &attrs);
  UnSet(c->state, ClMoving | ClResizing);
  memset(&mon->grabbed, 0, sizeof(mon->grabbed));
  XUngrabPointer(mon->ctx->dpy, CurrentTime);
}

void onDestroyNotify(Monitor *mon, const XEvent *xevent)
{
  const XDestroyWindowEvent *e = &xevent->xdestroywindow;
  EVENT("DestroyNotify on window: %lu.\n", e->window);
  for (size_t i = 0; i < Length(workspaces); ++i) {
    Client *c = NULL;
    if ((c = ws_getclient(mon_workspaceat(mon, i), e->window))) {
      ManageClientHook(ClientRemove, mon, c);
      return;
    }
  }
}

int xerror_handler(Display *dpy, XErrorEvent *e)
{
  char error_code[1024];
  XGetErrorText(dpy, e->error_code, error_code, 1024);
  LOG("[ERROR] Error occurred during event no: %lu.\n", e->serial);
  ERROR("resourceId: %lu.\n", e->resourceid);
  ERROR("serial: %lu.\n", e->serial);
  ERROR("error_code: %s.\n", error_code);
  ERROR("request_code: %s.\n", RequestCodes[e->request_code]);
  ERROR("minor_code: %u.\n", e->minor_code);
  return 1;
}

int main()
{
  XEvent e;
  Monitor mon;
  mon_init(&mon);
  XSetErrorHandler(xerror_handler);

  while (mon.ctx->running && !XNextEvent(mon.ctx->dpy, &e)) {
    if (EventRepr[e.type] && e.type != MotionNotify)
      EVENT("%s on window: %lu.\n", EventRepr[e.type], e.xany.window);
    if (e.type != DestroyNotify && default_event_handlers[e.type])
      default_event_handlers[e.type](&mon, &e);
    if (sch_event_handlers[e.type])
      sch_event_handlers[e.type](&mon, &e);
    if (dock_event_handlers[e.type])
      dock_event_handlers[e.type](&mon, &e);
    if (ewmh_event_handlers[e.type])
      ewmh_event_handlers[e.type](&mon, &e);
    if (e.type == DestroyNotify && default_event_handlers[e.type])
      default_event_handlers[e.type](&mon, &e);
  }

  XCloseDisplay(mon.ctx->dpy);
  return 0;
}
