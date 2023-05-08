#ifndef __CORE_H__
#define __CORE_H__

#include "debug.h"
#include <X11/Xutil.h>
#include <cluless/core/monitor.h>
#include <cluless/core/utils.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define RootWindowEventMasks (SubstructureRedirectMask | SubstructureNotifyMask)
#define ButtonMasks          (ButtonPressMask | ButtonReleaseMask)

// hooks are only called on clients which are attached to the workspaces managed
// my the monitor.
typedef void (*EventHandler)(const XEvent *);

ENUM(CursorType, CurNormal, CurResize, CurMove);
ENUM(WMAtom, WM_PROTOCOLS, WM_NAME, WM_DELETE_WINDOW, WM_TRANSIENT_FOR,
     WM_WINDOW_ROLE);
ENUM(NetAtom, NET_ACTIVE_WINDOW, NET_CLIENT_LIST, NET_WM_BYPASS_COMPOSITOR,
     NET_WM_NAME, NET_WM_STRUT, NET_WM_STRUT_PARTIAL, NET_WM_WINDOW_TYPE,
     NET_WM_WINDOW_TYPE_DOCK);

// These are mainly the values that don't (shouldn't) change throughout the
// application lifetime.
typedef struct Core {
    bool running;
    Display *dpy;
    Monitor *mon;
    Cursor cursors[NullCursorType];
    Atom wmatoms[NullWMAtom], netatoms[NullNetAtom];
    FILE *logger;

    void (*init)(void);
    Window (*input_focused_window)(void);
    Geometry (*get_screen_rect)(void);
    bool (*send_event)(Window, Atom);
    int (*get_window_property)(Window, Atom, int, uint8_t **);
    int (*get_window_title)(Window, XTextProperty *);
    uint32_t (*get_window_list)(Window **);
    void (*stop_running)(void);
} Core;
extern const Core *const core;

#endif
