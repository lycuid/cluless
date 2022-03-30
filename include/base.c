#include "base.h"
#include <X11/cursorfont.h>
#include <config.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static Context ctx;

Context *create_context()
{
  if ((ctx.dpy = XOpenDisplay(NULL)) == NULL)
    die("Cannot open display.\n");
  ctx.root    = DefaultRootWindow(ctx.dpy);
  ctx.running = true;
  // @FIXME: need to know if the file is closed.
  ctx.pipefile = pipe_cmd[0] ? popen(pipe_cmd[0], "w") : NULL;

  ctx.cursors[CurNormal] = XCreateFontCursor(ctx.dpy, XC_left_ptr);
  ctx.cursors[CurResize] = XCreateFontCursor(ctx.dpy, XC_sizing);
  ctx.cursors[CurMove]   = XCreateFontCursor(ctx.dpy, XC_fleur);

  ctx.wmatoms[WMProtocols]    = XInternAtom(ctx.dpy, "WM_PROTOCOLS", False);
  ctx.wmatoms[WMName]         = XInternAtom(ctx.dpy, "WM_NAME", False);
  ctx.wmatoms[WMDeleteWindow] = XInternAtom(ctx.dpy, "WM_DELETE_WINDOW", False);
  ctx.wmatoms[WMTransientFor] = XInternAtom(ctx.dpy, "WM_TRANSIENT_FOR", False);

  ctx.netatoms[NetWMName] = XInternAtom(ctx.dpy, "_NET_WM_NAME", False);
  ctx.netatoms[NetWMWindowType] =
      XInternAtom(ctx.dpy, "_NET_WM_WINDOW_TYPE", False);
  ctx.netatoms[NetWMWindowTypeDock] =
      XInternAtom(ctx.dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
  ctx.netatoms[NetWMStrut] = XInternAtom(ctx.dpy, "_NET_WM_STRUT", False);
  ctx.netatoms[NetWMStrutPartial] =
      XInternAtom(ctx.dpy, "_NET_WM_STRUT_PARTIAL", False);
  ctx.netatoms[NetActiveWindow] =
      XInternAtom(ctx.dpy, "_NET_ACTIVE_WINDOW", False);
  ctx.netatoms[NetClientList] = XInternAtom(ctx.dpy, "_NET_CLIENT_LIST", False);

  XDeleteProperty(ctx.dpy, ctx.root, ctx.netatoms[NetClientList]);
  XSetWindowAttributes attrs = {.cursor     = ctx.cursors[CurNormal],
                                .event_mask = RootWindowEventMasks};
  XChangeWindowAttributes(ctx.dpy, ctx.root, CWCursor | CWEventMask, &attrs);
  for (size_t i = 0; i < Length(keys); ++i)
    XGrabKey(ctx.dpy, XKeysymToKeycode(ctx.dpy, keys[i].sym), keys[i].mask,
             ctx.root, 0, GrabModeAsync, GrabModeAsync);
  for (size_t i = 0; i < Length(buttons); ++i)
    XGrabButton(ctx.dpy, buttons[i].sym, buttons[i].mask, ctx.root, False,
                ButtonMasks, GrabModeAsync, GrabModeAsync, None, None);
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
    while (!exists && --n >= 0)
      if ((exists = protos[n] == ctx.wmatoms[WMDeleteWindow])) {
        XEvent e               = {.type = ClientMessage};
        e.xclient.window       = window;
        e.xclient.format       = 32;
        e.xclient.message_type = ctx.wmatoms[WMProtocols];
        e.xclient.data.l[0]    = protocol;
        e.xclient.data.l[1]    = CurrentTime;
        XSendEvent(ctx.dpy, window, False, NoEventMask, &e);
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
      XGetTextProperty(ctx.dpy, window, wm_name, ctx.netatoms[NetWMName]) &&
      wm_name->nitems;
  return found
             ? found
             : XGetTextProperty(ctx.dpy, window, wm_name, ctx.wmatoms[WMName]);
}
