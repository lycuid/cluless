#include "config.h"
#include "include/base.h"
#include "include/client.h"
#include "include/ewmh/docks.h"
#include "include/monitor.h"
#include "include/workspace.h"
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <stdlib.h>
#include <string.h>

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

typedef void (*EventHandler)(Monitor *, const XEvent *);
static EventHandler event_handlers[LASTEvent] = {
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

void onMapRequest(Monitor *mon, const XEvent *xevent)
{
  const XMapRequestEvent *e = &xevent->xmaprequest;
  EVENT("MapRequest on window: %lu.\n", e->window);
  Client *c;
  if (!(c = ws_getclient(mon->selws, e->window))) {
    c = cl_create(e->window);
    ws_attachclient(mon->selws, c);
    mon_setactive(mon, c);
    XSelectInput(mon->ctx->dpy, c->window, PropertyChangeMask);

    XClassHint *class = XAllocClassHint();
    for (size_t i = 0; i < Length(hooks); ++i) {
      const Hook *h = &hooks[i];
      if (XGetClassHint(mon->ctx->dpy, e->window, class))
        if (strstr(class->res_class, h->class_name) ||
            strstr(class->res_name, h->class_name)) {
          h->func(mon, &h->arg);
          break;
        }
    }
    XFree(class);
  }
  // client might be moved to another workspace by a Hook, so we only map the
  // window if the client is found in selws.
  if (!ws_getclient(mon->selws, c->window))
    return;
  XMapWindow(mon->ctx->dpy, c->window);
}

void onMapNotify(Monitor *mon, const XEvent *xevent)
{
  const XMapEvent *e = &xevent->xmap;
  EVENT("MapNotify on window: %lu.\n", e->window);
  Client *c;
  if (e->override_redirect || !(c = ws_getclient(mon->selws, e->window)))
    return;
  Window w;
  if (XGetTransientForHint(mon->ctx->dpy, c->window, &w))
    Set(c->state, ClTransient);
  XSetWindowBorderWidth(mon->ctx->dpy, c->window, mon->selws->borderpx);
  if (IsSet(c->state, ClActive))
    mon_focusclient(mon, c);
  mon_arrange(mon);
}

void onUnmapNotify(Monitor *mon, const XEvent *xevent)
{
  const XUnmapEvent *e = &xevent->xunmap;
  EVENT("UnmapNotify on window: %lu.\n", e->window);
  Client *c;
  if ((c = ws_getclient(mon->selws, e->window))) {
    ws_detachclient(mon->selws, c);
    mon_destroyclient(mon, c);
  }
}

void onConfigureRequest(Monitor *mon, const XEvent *xevent)
{
  const XConfigureRequestEvent *e = &xevent->xconfigurerequest;
  EVENT("ConfigureRequest on window: %lu.\n", e->window);
  XWindowChanges changes = {
      .x            = e->x,
      .y            = e->y,
      .width        = e->width,
      .height       = e->height,
      .border_width = e->border_width,
      .sibling      = e->above,
      .stack_mode   = e->detail,
  };
  XConfigureWindow(mon->ctx->dpy, e->window, e->value_mask, &changes);
  XSync(mon->ctx->dpy, False);
  mon_arrange(mon);
}

void onPropertyNotify(Monitor *mon, const XEvent *xevent)
{
  const XPropertyEvent *e = &xevent->xproperty;
  EVENT("PropertyNotify on window: %lu.\n", e->window);
  if (e->state == PropertyNewValue &&
      (e->atom == mon->ctx->netatoms[NetWMName] ||
       e->atom == mon->ctx->wmatoms[WMName]))
    mon_statuslog(mon);
}

void onKeyPress(Monitor *mon, const XEvent *xevent)
{
  const XKeyEvent *e = &xevent->xkey;
  for (size_t i = 0; i < Length(keys); ++i)
    if (e->keycode == XKeysymToKeycode(mon->ctx->dpy, keys[i].sym) &&
        e->state == keys[i].mask)
      keys[i].handler(mon, &keys[i].arg);
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
      buttons[i].handler(mon, &buttons[i].arg);
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
    XResizeWindow(mon->ctx->dpy, c->window, grabbed->cw + dx, grabbed->ch + dy);
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
  Client *c;
  for (size_t i = 0; i < Length(workspaces); ++i) {
    Workspace *from = mon_workspaceat(mon, i);
    if ((c = ws_getclient(from, e->window))) {
      ACTION("Destroying window: %lu.\n", e->window);
      ws_detachclient(from, c);
      mon_destroyclient(mon, c);
      break;
    }
  }
}

int xerror_handler(Display *dpy, XErrorEvent *e)
{
  char error_code[1024];
  XGetErrorText(dpy, e->error_code, error_code, 1024);
  LOG("[ERROR] resourceId: %lu.\n", e->resourceid);
  LOG("[ERROR] serial: %lu.\n", e->serial);
  LOG("[ERROR] error_code: %s.\n", error_code);
  LOG("[ERROR] request_code: %s.\n", RequestCodes[e->request_code]);
  LOG("[ERROR] minor_code: %u.\n", e->minor_code);
  return 1;
}

int main()
{
  Monitor mon;
  mon_init(&mon);
  XSetErrorHandler(xerror_handler);

  XEvent e;
  while (mon.ctx->running && !XNextEvent(mon.ctx->dpy, &e)) {
    if (EventRepr[e.type] && e.type != MotionNotify)
      EVENT("%s on window: %lu.\n", EventRepr[e.type], e.xany.window);
    ws_dump(mon.selws);
    if (event_handlers[e.type])
      event_handlers[e.type](&mon, &e);
    if (dock_event_handlers[e.type])
      dock_event_handlers[e.type](&mon, &e);
  }

  XCloseDisplay(mon.ctx->dpy);
  return 0;
}
