#include "core.h"
#include <X11/Xatom.h>
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
#define ATOM_REPR(atom) ctx.wmatoms[atom] = XInternAtom(ctx.dpy, #atom, False)
  ATOM_REPR(WM_PROTOCOLS);
  ATOM_REPR(WM_NAME);
  ATOM_REPR(WM_DELETE_WINDOW);
  ATOM_REPR(WM_TRANSIENT_FOR);
#undef ATOM_REPR

  // EWMH Atoms.
#define NET_ATOM_REPR(atom)                                                    \
  ctx.netatoms[atom] = XInternAtom(ctx.dpy, "_" #atom, False)
  NET_ATOM_REPR(NET_WM_NAME);
  NET_ATOM_REPR(NET_WM_WINDOW_TYPE);
  NET_ATOM_REPR(NET_WM_WINDOW_TYPE_DOCK);
  NET_ATOM_REPR(NET_WM_STRUT);
  NET_ATOM_REPR(NET_WM_STRUT_PARTIAL);
  NET_ATOM_REPR(NET_ACTIVE_WINDOW);
  NET_ATOM_REPR(NET_CLIENT_LIST);
#undef NET_ATOM_REPR

  XChangeProperty(
      ctx.dpy, ctx.root, XInternAtom(ctx.dpy, "_NET_SUPPORTED", False), XA_ATOM,
      32, PropModeReplace, (uint8_t *)ctx.netatoms, LENGTH(ctx.netatoms));
  XDeleteProperty(ctx.dpy, ctx.root, ctx.netatoms[NET_CLIENT_LIST]);
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
        e.xclient.message_type = ctx.wmatoms[WM_PROTOCOLS];
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
      XGetTextProperty(ctx.dpy, window, wm_name, ctx.netatoms[NET_WM_NAME]) &&
      wm_name->nitems;
  return found
             ? found
             : XGetTextProperty(ctx.dpy, window, wm_name, ctx.wmatoms[WM_NAME]);
}
