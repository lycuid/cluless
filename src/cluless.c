#include "config.h"
#include <X11/Xlib.h>
#include <cluless/core.h>
#include <cluless/core/client.h>
#include <cluless/core/monitor.h>
#include <cluless/core/workspace.h>
#include <cluless/ewmh.h>
#include <cluless/ewmh/docks.h>
#include <cluless/scratchpad.h>
#include <stdlib.h>
#include <string.h>

static inline void ManageClientHook(HookType, Monitor *, Client *);

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

static const ClientHook default_client_hooks[NullHookType] = {
    [ClientAdd]    = mon_manage_client,
    [ClientRemove] = mon_unmanage_client,
};

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
    [DestroyNotify]    = onDestroyNotify,
};

static inline void ManageClientHook(HookType type, Monitor *mon, Client *c)
{
  if (type != ClientRemove)
    CALL(default_client_hooks[type], mon, c);
  CALL(sch_client_hooks[type], mon, c);
  if (type == ClientRemove)
    CALL(default_client_hooks[type], mon, c);
  if (type == ClientRemove && c)
    free(c);
}

void onMapRequest(Monitor *mon, const XEvent *xevent)
{
  const XMapRequestEvent *e = &xevent->xmaprequest;
  Client *c;
  if (!(c = ws_getclient(mon->selws, e->window)))
    ManageClientHook(ClientAdd, mon, (c = cl_create(e->window)));
  // client might be moved to another workspace by a WindowRule, so we only map
  // the window if the client is found in selws.
  if (!ws_getclient(mon->selws, c->window))
    return;
  Window w;
  if (XGetTransientForHint(core->dpy, c->window, &w))
    SET(c->state, ClTransient);
  mon->applylayout();
  XMapWindow(core->dpy, c->window);
}

void onMapNotify(Monitor *mon, const XEvent *xevent)
{
  const XMapEvent *e = &xevent->xmap;
  Client *c;
  if ((c = ws_getclient(mon->selws, e->window)) && IS_SET(c->state, ClActive))
    mon->focusclient(c);
}

void onUnmapNotify(Monitor *mon, const XEvent *xevent)
{
  const XUnmapEvent *e = &xevent->xunmap;
  Client *c;
  if ((c = ws_getclient(mon->selws, e->window)))
    ManageClientHook(ClientRemove, mon, c);
}

void onConfigureRequest(Monitor *mon, const XEvent *xevent)
{
  (void)mon;
  const XConfigureRequestEvent *e = &xevent->xconfigurerequest;
  XWindowChanges changes          = {.x            = e->x,
                                     .y            = e->y,
                                     .width        = e->width,
                                     .height       = e->height,
                                     .border_width = e->border_width,
                                     .sibling      = e->above,
                                     .stack_mode   = e->detail};
  XConfigureWindow(core->dpy, e->window, e->value_mask, &changes);
  XSync(core->dpy, False);
}

void onPropertyNotify(Monitor *mon, const XEvent *xevent)
{
  const XPropertyEvent *e = &xevent->xproperty;
  if (e->state == PropertyNewValue && (e->atom == core->netatoms[NET_WM_NAME] ||
                                       e->atom == core->wmatoms[WM_NAME]))
    mon->statuslog();
}

void onKeyPress(Monitor *mon, const XEvent *xevent)
{
  const XKeyEvent *e = &xevent->xkey;
  FOREACH(const Binding *key, keys)
  {
    if (e->keycode == XKeysymToKeycode(core->dpy, key->sym) &&
        e->state == key->mask)
      key->func(mon, &key->arg);
  }
}

void onButtonPress(Monitor *mon, const XEvent *xevent)
{
  const XButtonEvent *e = &xevent->xbutton;
  XGrabPointer(core->dpy, core->root, False, ButtonMasks | PointerMotionMask,
               GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
  Client *c;
  memset(&mon->grabbed, 0, sizeof(mon->grabbed));
  if ((c = ws_getclient(mon->selws, e->subwindow))) {
    int state = e->button == Button1   ? Button1Mask
                : e->button == Button2 ? Button2Mask
                : e->button == Button3 ? Button3Mask
                : e->button == Button4 ? Button4Mask
                : e->button == Button5 ? Button5Mask
                                       : 0;

    XWindowAttributes wa;
    XGetWindowAttributes(core->dpy, c->window, &wa);
    mon->grabbed.client = c;
    mon->grabbed.x      = e->x_root;
    mon->grabbed.y      = e->y_root;
    mon->grabbed.cx     = wa.x;
    mon->grabbed.cy     = wa.y;
    mon->grabbed.cw     = wa.width;
    mon->grabbed.ch     = wa.height;
    mon->grabbed.state  = e->state | state;
    mon->grabbed.at     = e->time;
  }
  FOREACH(const Binding *button, buttons)
  {
    if (e->button == button->sym && e->state == button->mask)
      button->func(mon, &button->arg);
  }
}

void onMotionNotify(Monitor *mon, const XEvent *xevent)
{
  const XMotionEvent *e = &xevent->xmotion;
  PointerGrab *grabbed  = &mon->grabbed;
  Client *c             = grabbed->client;
  if (!c || e->state != grabbed->state || (e->time - grabbed->at) < (1000 / 60))
    return;

  int dx = e->x - grabbed->x, dy = e->y - grabbed->y;
  if (IS_SET(c->state, ClMoving))
    XMoveWindow(core->dpy, c->window, grabbed->cx + dx, grabbed->cy + dy);
  else if (IS_SET(c->state, ClResizing))
    XResizeWindow(core->dpy, c->window, MAX(grabbed->cw + dx, c->minw),
                  MAX(grabbed->ch + dy, c->minh));
  grabbed->at = e->time;
}

void onButtonRelease(Monitor *mon, const XEvent *xevent)
{
  (void)xevent;
  Client *c;
  if (!(c = mon->grabbed.client))
    return;
  XChangeWindowAttributes(
      core->dpy, c->window, CWCursor,
      &(XSetWindowAttributes){.cursor = core->cursors[CurNormal]});
  UNSET(c->state, ClMoving | ClResizing);
  memset(&mon->grabbed, 0, sizeof(mon->grabbed));
  XUngrabPointer(core->dpy, CurrentTime);
}

void onDestroyNotify(Monitor *mon, const XEvent *xevent)
{
  const XDestroyWindowEvent *e = &xevent->xdestroywindow;
  ITER(workspaces)
  {
    Client *c = ws_getclient(&mon->wss[it], e->window);
    if (c) {
      ManageClientHook(ClientRemove, mon, c);
      break;
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

static inline void argparse(int argc, const char **argv)
{
  if (argc == 2 && (!strcmp("-v", argv[1]) || !strcmp("--version", argv[1]))) {
    fprintf(stdout, NAME "-" VERSION "\n");
    exit(EXIT_SUCCESS);
  }
}

int main(int argc, char const **argv)
{
  argparse(argc, argv);

  core_init();
  Monitor *mon = mon_init();
  XSetErrorHandler(xerror_handler);

  XEvent e;
  while (mon->running && !XNextEvent(core->dpy, &e)) {
    if (EventRepr[e.type] && e.type != MotionNotify)
      LOG("[EVENT] %s on window: %lu.\n", EventRepr[e.type], e.xany.window);
    if (e.type != DestroyNotify)
      CALL(default_event_handlers[e.type], mon, &e);
    CALL(sch_event_handlers[e.type], mon, &e);
    CALL(dock_event_handlers[e.type], mon, &e);
    CALL(ewmh_event_handlers[e.type], mon, &e);
    if (e.type == DestroyNotify)
      CALL(default_event_handlers[e.type], mon, &e);
    switch (e.type) {
    case MapNotify:
    case UnmapNotify:
    case FocusIn:
    case ConfigureNotify:
      mon->applylayout();
    }
  }

  XUngrabKey(core->dpy, AnyKey, AnyModifier, core->root);
  XUngrabButton(core->dpy, AnyButton, AnyModifier, core->root);
  XCloseDisplay(core->dpy);

  return 0;
}
