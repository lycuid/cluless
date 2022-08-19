#include "core.h"
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <config.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void core_init(void);
Window input_focused_window(void);
Geometry get_screen_rect(void);
bool send_event(Window, Atom);
int get_window_property(Window, Atom, int, uint8_t **);
int get_window_title(Window, XTextProperty *);

static struct Core local = {
    .init                 = core_init,
    .input_focused_window = input_focused_window,
    .get_screen_rect      = get_screen_rect,
    .send_event           = send_event,
    .get_window_property  = get_window_property,
    .get_window_title     = get_window_title,
};
const struct Core *const core = &local;

static void stop_status_logging(int _)
{
  (void)_;
  local.statuslogger = NULL;
}
static void start_status_logging()
{
  // [Broken Pipe] piped program crashes/stops, nullify the pointer.
  signal(SIGPIPE, stop_status_logging);
  local.statuslogger = statusbar[0] ? popen(statusbar[0], "w") : NULL;
}

void core_init(void)
{
  if ((local.dpy = XOpenDisplay(NULL)) == NULL)
    die("Cannot open display.\n");
  local.root = DefaultRootWindow(local.dpy);
  start_status_logging();

  local.cursors[CurNormal] = XCreateFontCursor(local.dpy, XC_left_ptr);
  local.cursors[CurResize] = XCreateFontCursor(local.dpy, XC_sizing);
  local.cursors[CurMove]   = XCreateFontCursor(local.dpy, XC_fleur);

  // ICCC Atoms.
#define ATOM_REPR(atom)                                                        \
  local.wmatoms[atom] = XInternAtom(local.dpy, #atom, False)
  ATOM_REPR(WM_PROTOCOLS);
  ATOM_REPR(WM_NAME);
  ATOM_REPR(WM_DELETE_WINDOW);
  ATOM_REPR(WM_TRANSIENT_FOR);
#undef ATOM_REPR

  // EWMH Atoms.
#define NET_ATOM_REPR(atom)                                                    \
  local.netatoms[atom] = XInternAtom(local.dpy, "_" #atom, False)
  NET_ATOM_REPR(NET_WM_NAME);
  NET_ATOM_REPR(NET_WM_WINDOW_TYPE);
  NET_ATOM_REPR(NET_WM_WINDOW_TYPE_DOCK);
  NET_ATOM_REPR(NET_WM_STRUT);
  NET_ATOM_REPR(NET_WM_STRUT_PARTIAL);
  NET_ATOM_REPR(NET_ACTIVE_WINDOW);
  NET_ATOM_REPR(NET_CLIENT_LIST);
#undef NET_ATOM_REPR

  XChangeProperty(local.dpy, local.root,
                  XInternAtom(local.dpy, "_NET_SUPPORTED", False), XA_ATOM, 32,
                  PropModeReplace, (uint8_t *)local.netatoms,
                  LENGTH(local.netatoms));
  XDeleteProperty(local.dpy, local.root, local.netatoms[NET_CLIENT_LIST]);
  XChangeWindowAttributes(
      local.dpy, local.root, CWCursor | CWEventMask,
      &(XSetWindowAttributes){.cursor     = local.cursors[CurNormal],
                              .event_mask = RootWindowEventMasks});
  FOREACH(const Binding *key, keys)
  {
    XGrabKey(local.dpy, XKeysymToKeycode(local.dpy, key->sym), key->mask,
             local.root, 0, GrabModeAsync, GrabModeAsync);
  }
  FOREACH(const Binding *button, buttons)
  {
    XGrabButton(local.dpy, button->sym, button->mask, local.root, False,
                ButtonMasks, GrabModeAsync, GrabModeAsync, None, None);
  }
}

Window input_focused_window()
{
  int t;
  Window window;
  XGetInputFocus(local.dpy, &window, &t);
  return window;
}

Geometry get_screen_rect()
{
  return (Geometry){
      .x = 0,
      .y = 0,
      .w = DisplayWidth(local.dpy, DefaultScreen(local.dpy)),
      .h = DisplayHeight(local.dpy, DefaultScreen(local.dpy)),
  };
}

bool send_event(Window window, Atom protocol)
{
  Atom *protos = NULL;
  int n, exists = 0;
  if (XGetWMProtocols(local.dpy, window, &protos, &n)) {
    while (!exists && --n >= 0) {
      if ((exists = protos[n] == protocol)) {
        XEvent e               = {.type = ClientMessage};
        e.xclient.window       = window;
        e.xclient.format       = 32;
        e.xclient.message_type = local.wmatoms[WM_PROTOCOLS];
        e.xclient.data.l[0]    = protocol;
        e.xclient.data.l[1]    = CurrentTime;
        XSendEvent(local.dpy, window, False, NoEventMask, &e);
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
  return XGetWindowProperty(local.dpy, window, key, 0l, size, False, type,
                            &type, (int *)&n, &n, &n, value);
}

int get_window_title(Window window, XTextProperty *wm_name)
{
  int found = XGetTextProperty(local.dpy, window, wm_name,
                               local.netatoms[NET_WM_NAME]) &&
              wm_name->nitems;
  return found ? found
               : XGetTextProperty(local.dpy, window, wm_name,
                                  local.wmatoms[WM_NAME]);
}
