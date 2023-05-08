#include "core.h"
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <cluless/core/monitor.h>
#include <config.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void core_init(void);
Window input_focused_window(void);
Geometry get_screen_rect(void);
bool send_event(Window, Atom);
int get_window_property(Window, Atom, int, uint8_t **);
int get_window_title(Window, XTextProperty *);
uint32_t get_window_list(Window **);

static Monitor local_mon;

static Core local = {
    .mon                  = &local_mon,
    .running              = true,
    .init                 = core_init,
    .input_focused_window = input_focused_window,
    .get_screen_rect      = get_screen_rect,
    .send_event           = send_event,
    .get_window_property  = get_window_property,
    .get_window_title     = get_window_title,
    .get_window_list      = get_window_list,
};
const Core *const core = &local;

static void sighandler(int sig)
{
    if (signal(sig, sighandler) == sighandler && sig == SIGCHLD)
        wait(NULL);
}

void core_init(void)
{
    if ((local.dpy = XOpenDisplay(NULL)) == NULL)
        die("Cannot open display.\n");
    local.logger = stdout;
    sighandler(SIGCHLD);
    signal(SIGPIPE, SIG_IGN);

    local.cursors[CurNormal] = XCreateFontCursor(local.dpy, XC_left_ptr);
    local.cursors[CurResize] = XCreateFontCursor(local.dpy, XC_sizing);
    local.cursors[CurMove]   = XCreateFontCursor(local.dpy, XC_fleur);

    // ICCC Atoms.
#define WM_ATOM_REPR(atom)                                                     \
    local.wmatoms[atom] = XInternAtom(local.dpy, #atom, False)
    WM_ATOM_REPR(WM_PROTOCOLS);
    WM_ATOM_REPR(WM_NAME);
    WM_ATOM_REPR(WM_DELETE_WINDOW);
    WM_ATOM_REPR(WM_TRANSIENT_FOR);
    WM_ATOM_REPR(WM_WINDOW_ROLE);
#undef WM_ATOM_REPR

    // EWMH Atoms.
#define NET_ATOM_REPR(atom)                                                    \
    local.netatoms[atom] = XInternAtom(local.dpy, "_" #atom, False)
    NET_ATOM_REPR(NET_ACTIVE_WINDOW);
    NET_ATOM_REPR(NET_CLIENT_LIST);
    NET_ATOM_REPR(NET_WM_BYPASS_COMPOSITOR);
    NET_ATOM_REPR(NET_WM_NAME);
    NET_ATOM_REPR(NET_WM_STRUT);
    NET_ATOM_REPR(NET_WM_STRUT_PARTIAL);
    NET_ATOM_REPR(NET_WM_WINDOW_TYPE);
    NET_ATOM_REPR(NET_WM_WINDOW_TYPE_DOCK);
#undef NET_ATOM_REPR

    XStoreName(local.dpy, DefaultRootWindow(local.dpy), NAME "-" VERSION);
    XChangeProperty(local.dpy, DefaultRootWindow(local.dpy),
                    XInternAtom(local.dpy, "_NET_SUPPORTED", False), XA_ATOM,
                    32, PropModeReplace, (uint8_t *)local.netatoms,
                    LENGTH(local.netatoms));
    XDeleteProperty(local.dpy, DefaultRootWindow(local.dpy),
                    local.netatoms[NET_CLIENT_LIST]);
    XChangeWindowAttributes(
        local.dpy, DefaultRootWindow(local.dpy), CWCursor | CWEventMask,
        &(XSetWindowAttributes){.cursor     = local.cursors[CurNormal],
                                .event_mask = RootWindowEventMasks});
    FOREACH(const Binding *key, keys)
    {
        XGrabKey(local.dpy, XKeysymToKeycode(local.dpy, key->sym), key->mask,
                 DefaultRootWindow(local.dpy), 0, GrabModeAsync, GrabModeAsync);
    }
    FOREACH(const Binding *button, buttons)
    {
        XGrabButton(local.dpy, button->sym, button->mask,
                    DefaultRootWindow(local.dpy), False, ButtonMasks,
                    GrabModeAsync, GrabModeAsync, None, None);
    }
    mon_init(local.mon);
}

Window input_focused_window(void)
{
    int t;
    Window window;
    XGetInputFocus(local.dpy, &window, &t);
    return window;
}

Geometry get_screen_rect(void)
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

uint32_t get_window_list(Window **windows)
{
    Window orphan;
    uint32_t window_cnt;
    XQueryTree(local.dpy, DefaultRootWindow(local.dpy), &orphan, &orphan,
               windows, &window_cnt);
    return window_cnt;
}

void stop_running(void) { local.running = false; }
