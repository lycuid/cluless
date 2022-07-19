#include "core.h"
#include <X11/cursorfont.h>
#include <config.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static Context ctx;

Context *request_context() { return &ctx; }

static void stop_status_logging() { ctx.statuslogger = NULL; }
static void start_status_logging()
{
  // [Broken Pipe] piped program crashes/stops, nullify the pointer.
  signal(SIGPIPE, stop_status_logging);
  ctx.statuslogger = statusbar[0] ? popen(statusbar[0], "w") : NULL;
}

Context *create_context()
{
  if ((ctx.dpy = XOpenDisplay(NULL)) == NULL)
    die("Cannot open display.\n");
  ctx.root    = DefaultRootWindow(ctx.dpy);
  ctx.running = true;
  start_status_logging();

  ctx.cursors[CurNormal] = XCreateFontCursor(ctx.dpy, XC_left_ptr);
  ctx.cursors[CurResize] = XCreateFontCursor(ctx.dpy, XC_sizing);
  ctx.cursors[CurMove]   = XCreateFontCursor(ctx.dpy, XC_fleur);

  // ICCC Atoms.
  ctx.atoms[WMProtocols]    = XInternAtom(ctx.dpy, "WM_PROTOCOLS", False);
  ctx.atoms[WMName]         = XInternAtom(ctx.dpy, "WM_NAME", False);
  ctx.atoms[WMDeleteWindow] = XInternAtom(ctx.dpy, "WM_DELETE_WINDOW", False);
  ctx.atoms[WMTransientFor] = XInternAtom(ctx.dpy, "WM_TRANSIENT_FOR", False);
  // EWMH Atoms.
  ctx.atoms[NetWMName] = XInternAtom(ctx.dpy, "_NET_WM_NAME", False);
  ctx.atoms[NetWMWindowType] =
      XInternAtom(ctx.dpy, "_NET_WM_WINDOW_TYPE", False);
  ctx.atoms[NetWMWindowTypeDock] =
      XInternAtom(ctx.dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
  ctx.atoms[NetWMStrut] = XInternAtom(ctx.dpy, "_NET_WM_STRUT", False);
  ctx.atoms[NetWMStrutPartial] =
      XInternAtom(ctx.dpy, "_NET_WM_STRUT_PARTIAL", False);
  ctx.atoms[NetActiveWindow] =
      XInternAtom(ctx.dpy, "_NET_ACTIVE_WINDOW", False);
  ctx.atoms[NetClientList] = XInternAtom(ctx.dpy, "_NET_CLIENT_LIST", False);

  XDeleteProperty(ctx.dpy, ctx.root, ctx.atoms[NetClientList]);
  XChangeWindowAttributes(
      ctx.dpy, ctx.root, CWCursor | CWEventMask,
      &(XSetWindowAttributes){.cursor     = ctx.cursors[CurNormal],
                              .event_mask = RootWindowEventMasks});
  FOREACH(const Binding *key, keys)
  {
    XGrabKey(ctx.dpy, XKeysymToKeycode(ctx.dpy, key->sym), key->mask, ctx.root,
             0, GrabModeAsync, GrabModeAsync);
  }
  FOREACH(const Binding *button, buttons)
  {
    XGrabButton(ctx.dpy, button->sym, button->mask, ctx.root, False,
                ButtonMasks, GrabModeAsync, GrabModeAsync, None, None);
  }
  return &ctx;
}

Window input_focused_window()
{
  int t;
  Window window;
  XGetInputFocus(ctx.dpy, &window, &t);
  return window;
}

Geometry get_screen_rect()
{
  return (Geometry){.x = 0,
                    .y = 0,
                    .w = DisplayWidth(ctx.dpy, DefaultScreen(ctx.dpy)),
                    .h = DisplayHeight(ctx.dpy, DefaultScreen(ctx.dpy))};
}

bool send_event(Window window, Atom protocol)
{
  Atom *protos = NULL;
  int n, exists = 0;
  if (XGetWMProtocols(ctx.dpy, window, &protos, &n)) {
    while (!exists && --n >= 0) {
      if ((exists = protos[n] == protocol)) {
        XEvent e               = {.type = ClientMessage};
        e.xclient.window       = window;
        e.xclient.format       = 32;
        e.xclient.message_type = ctx.atoms[WMProtocols];
        e.xclient.data.l[0]    = protocol;
        e.xclient.data.l[1]    = CurrentTime;
        XSendEvent(ctx.dpy, window, False, NoEventMask, &e);
      }
    }
    XFree(protos);
  }
  return exists;
}

int get_window_property(Window window, Atom key, int size, uint8_t **value)
{
  uint64_t n;
  Atom type = AnyPropertyType;
  return XGetWindowProperty(ctx.dpy, window, key, 0l, size, False, type, &type,
                            (int *)&n, &n, &n, value);
}

int get_window_title(Window window, XTextProperty *wm_name)
{
  int found =
      XGetTextProperty(ctx.dpy, window, wm_name, ctx.atoms[NetWMName]) &&
      wm_name->nitems;
  return found ? found
               : XGetTextProperty(ctx.dpy, window, wm_name, ctx.atoms[WMName]);
}
