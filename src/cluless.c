#include "config.h"
#include <X11/Xlib.h>
#include <cluless/core.h>
#include <cluless/core/client.h>
#include <cluless/core/logging.h>
#include <cluless/core/monitor.h>
#include <cluless/core/workspace.h>
#include <cluless/ewmh.h>
#include <cluless/ewmh/docks.h>
#include <cluless/misc/scratchpad.h>
#include <stdlib.h>
#include <string.h>

#define CALL(f, ...) f ? f(__VA_ARGS__) : (void)0

static inline void Broadcast(HookType, Monitor *, Client *);

void onMapRequest(Monitor *, const XEvent *);
void onMapNotify(Monitor *, const XEvent *);
void onUnmapNotify(Monitor *, const XEvent *);
void onConfigureRequest(Monitor *, const XEvent *);
void onPropertyNotify(Monitor *, const XEvent *);
void onFocusIn(Monitor *, const XEvent *);
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
    [FocusIn]          = onFocusIn,
    [KeyPress]         = onKeyPress,
    [ButtonPress]      = onButtonPress,
    [MotionNotify]     = onMotionNotify,
    [ButtonRelease]    = onButtonRelease,
    [DestroyNotify]    = onDestroyNotify,
};

static inline void Broadcast(HookType type, Monitor *mon, Client *c)
{
  XSync(core->dpy, False);
  CALL(default_client_hooks[type], mon, c);
  CALL(sch_client_hooks[type], mon, c);
  CALL(ewmh_client_hooks[type], mon, c);
  if (type == ClientRemove && c)
    cl_free(c);
  mon_applylayout(mon);
}

void onMapRequest(Monitor *mon, const XEvent *xevent)
{
  const XMapRequestEvent *e = &xevent->xmaprequest;
  Client *c                 = NULL;
  FOREACH_AVAILABLE_CLIENT(c)
  {
    // Ignore multiple map requests from already allocated clients.
    if (c->window == e->window)
      return;
  }
  Broadcast(ClientAdd, mon, (c = cl_alloc(e->window)));
  // client might be moved to another workspace by a WindowRule, so we only
  // map the window if the client is found in selws.
  if (ws_getclient(mon->selws, c->window))
    XMapWindow(core->dpy, c->window);
}

void onMapNotify(Monitor *mon, const XEvent *xevent)
{
  const XMapEvent *e = &xevent->xmap;
  Client *c;
  if ((c = ws_getclient(mon->selws, e->window)) && IS_SET(c->state, ClActive))
    mon_focusclient(mon, c);
}

void onUnmapNotify(Monitor *mon, const XEvent *xevent)
{
  const XUnmapEvent *e = &xevent->xunmap;
  for (Client *c = ws_getclient(mon->selws, e->window); c; c = NULL)
    Broadcast(ClientRemove, mon, c);
}

void onConfigureRequest(Monitor *mon, const XEvent *xevent)
{
  (void)mon;
  const XConfigureRequestEvent *e = &xevent->xconfigurerequest;
  XWindowChanges changes          = {
               .x            = e->x,
               .y            = e->y,
               .width        = e->width,
               .height       = e->height,
               .border_width = e->border_width,
               .sibling      = e->above,
               .stack_mode   = e->detail,
  };
  // This might restack windows and change focus.
  if (XConfigureWindow(core->dpy, e->window, e->value_mask, &changes)) {
    if (ws_getclient(mon->selws, e->window))
      // 'mon_focusclient' calls 'mon_applylayout', which restacks windows.
      mon_focusclient(mon, ws_find(mon->selws, ClActive));
    XSync(core->dpy, False);
  }
}

void onPropertyNotify(Monitor *mon, const XEvent *xevent)
{
  const XPropertyEvent *e = &xevent->xproperty;
  if (e->state == PropertyDelete)
    return;
  if (e->atom == core->netatoms[NET_WM_NAME] ||
      e->atom == core->wmatoms[WM_NAME])
    log_statuslog(mon);
}

void onFocusIn(Monitor *mon, const XEvent *xevent)
{
  const XFocusInEvent *e = &xevent->xfocus;
  // Enforce focus on currently active client, as some applications might change
  // input focus on their own.
  for (Client *c = ws_getclient(mon->selws, e->window); c; c = NULL)
    if (!IS_SET(c->state, ClActive))
      mon_focusclient(mon, ws_find(mon->selws, ClActive));
}

void onKeyPress(Monitor *mon, const XEvent *xevent)
{
  const XKeyEvent *e = &xevent->xkey;
  FOREACH(const Binding *key, keys)
  {
    if (e->keycode == XKeysymToKeycode(core->dpy, key->sym) &&
        e->state == key->mask)
      key->action(mon, &key->arg);
  }
}

void onButtonPress(Monitor *mon, const XEvent *xevent)
{
  const XButtonEvent *e = &xevent->xbutton;
  Client *c;
  // Grabbing button press for focus on allocated client window, whereas
  // every other button press events are grabbed on root window. So if
  // button press event happens on any allocated client window which is not
  // focused, then just focus it and return. Otherwise if the button press
  // happens on root window, then just call button press action function on
  // the subwindow.
  if ((c = ws_getclient(mon->selws, e->window))) {
    if (!IS_SET(c->state, ClActive))
      mon_focusclient(mon, c);
  } else if ((c = ws_getclient(mon->selws, e->subwindow))) {
    XGrabPointer(core->dpy, core->root, False, ButtonMasks | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    memset(&mon->grabbed, 0, sizeof(mon->grabbed));
    int state = e->button == Button1   ? Button1Mask
                : e->button == Button2 ? Button2Mask
                : e->button == Button3 ? Button3Mask
                : e->button == Button4 ? Button4Mask
                : e->button == Button5 ? Button5Mask
                                       : 0;

    XWindowAttributes wa;
    XGetWindowAttributes(core->dpy, c->window, &wa);
    mon->grabbed.client        = c;
    mon->grabbed.x             = e->x_root;
    mon->grabbed.y             = e->y_root;
    mon->grabbed.cl_geometry.x = wa.x;
    mon->grabbed.cl_geometry.y = wa.y;
    mon->grabbed.cl_geometry.w = wa.width;
    mon->grabbed.cl_geometry.h = wa.height;
    mon->grabbed.state         = e->state | state;
    mon->grabbed.at            = e->time;
    FOREACH(const Binding *button, buttons)
    {
      if (e->button == button->sym && e->state == button->mask)
        button->action(mon, &button->arg);
    }
  }
}

void onMotionNotify(Monitor *mon, const XEvent *xevent)
{
  const XMotionEvent *e = &xevent->xmotion;
  PointerGrab *grabbed  = &mon->grabbed;
  Client *c             = grabbed->client;
  if (!c || e->state != grabbed->state || (e->time - grabbed->at) < (1000 / 60))
    return;

  Geometry *cg = &grabbed->cl_geometry;
  int dx = e->x - grabbed->x, dy = e->y - grabbed->y;
  if (IS_SET(c->state, ClMoving))
    XMoveWindow(core->dpy, c->window, cg->x + dx, cg->y + dy);
  else if (IS_SET(c->state, ClResizing))
    XResizeWindow(core->dpy, c->window, MAX(cg->w + dx, c->minw),
                  MAX(cg->h + dy, c->minh));
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
  FOREACH_AVAILABLE_CLIENT(Client * c)
  {
    if (c->window == e->window) {
      Broadcast(ClientRemove, mon, c);
      break;
    }
  }
}

int xerror_handler(Display *dpy, XErrorEvent *e)
{
  char error_code[1024];
  XGetErrorText(dpy, e->error_code, error_code, 1024);
  ERR("resourceId: %lu.\n", e->resourceid);
  ERR("serial: %lu.\n", e->serial);
  ERR("error_code: %s.\n", error_code);
  ERR("request_code: %s.\n", RequestCodes[e->request_code]);
  ERR("minor_code: %u.\n", e->minor_code);
  return 1;
}

int main(int argc, char const **argv)
{
  if (argc == 2 && (!strcmp("-v", argv[1]) || !strcmp("--version", argv[1]))) {
    fprintf(stdout, NAME ": v" VERSION "\n");
    exit(EXIT_SUCCESS);
  }

  // @SETUP.
  Monitor mon;
  core->init();
  mon_init(&mon);
  XSetErrorHandler(xerror_handler);
  // Allocating clients for all the windows that are already created before
  // the window manager started.
  Window *windows = NULL;
  XWindowAttributes attrs;
  for (int i = core->get_window_list(&windows) - 1; i >= 0; i--) {
    XGetWindowAttributes(core->dpy, windows[i], &attrs);
    if (attrs.map_state == IsViewable && !attrs.override_redirect)
      Broadcast(ClientAdd, &mon, cl_alloc(windows[i]));
  }
  if (windows)
    XFree(windows);
  if (mon.selws->cl_head)
    mon_focusclient(&mon, mon.selws->cl_head);

  for (XEvent e; core->running && !XNextEvent(core->dpy, &e);) {
    EVENT_DBG(e);
    CALL(default_event_handlers[e.type], &mon, &e);
    CALL(ewmh_event_handlers[e.type], &mon, &e);
    CALL(sch_event_handlers[e.type], &mon, &e);
    CALL(dock_event_handlers[e.type], &mon, &e);
  }

  // @CLEANUP.
  XUngrabKey(core->dpy, AnyKey, AnyModifier, core->root);
  XUngrabButton(core->dpy, AnyButton, AnyModifier, core->root);
  // Force kill all open windows.
  FOREACH_AVAILABLE_CLIENT(Client * orphan)
  {
    XKillClient(core->dpy, orphan->window);
  }
  XSync(core->dpy, False);
  XCloseDisplay(core->dpy);

  return 0;
}
