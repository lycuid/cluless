#include "docks.h"
#include <X11/Xlib.h>
#include <cluless/core.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

ENUM(Strut, Left, Right, Top, Bottom, LeftStartY, LeftEndY, RightStartY,
     RightEndY, TopStartX, TopEndX, BottomStartX, BottomEndX);

typedef struct DockCache {
    Window window;
    int64_t strut[NullStrut];
    struct DockCache *next;
} DockCache;

static struct Dock {
    bool visible : 1;
    DockCache *cache;
} dock = {true, NULL};

static inline void update_screen_geometry(void);

static void dcache_update(DockCache *, int64_t *, int);
static void dcache_put(Window, int64_t *, int);
static DockCache *dcache_get(Window);
static DockCache *dcache_remove(Window);

static inline void manage_dock(Window);
static inline void unmanage_dock(Window);

static inline void update_screen_geometry(void)
{
    Monitor *mon = core->mon;
    // @TODO: '_NET_WM_STRUT_PARTIAL' not implemented (currently
    // unnecessary, for my personal use case, also, I don't understand it
    // completely).
    int64_t left = 0, right = 0, top = 0, bottom = 0;
    for (DockCache *d = dock.cache; dock.visible && d; d = d->next) {
        left   = MAX(left, d->strut[Left]);
        right  = MAX(right, d->strut[Right]);
        top    = MAX(top, d->strut[Top]);
        bottom = MAX(bottom, d->strut[Bottom]);
    }
    mon->screen = core->get_screen_rect();
    mon->screen.x += left;
    mon->screen.y += top;
    mon->screen.w -= (left + right);
    mon->screen.h -= (top + bottom);
}

static void dcache_update(DockCache *cache, int64_t *strut, int nstrut)
{
    memset(cache->strut, 0, sizeof(int64_t) * NullStrut);
    memcpy(cache->strut, strut, sizeof(int64_t) * nstrut);
}

static void dcache_put(Window window, int64_t *strut, int nstrut)
{
    DockCache *cache = malloc(sizeof(DockCache));
    cache->window    = window;
    dcache_update(cache, strut, nstrut);
    cache->next = dock.cache;
    dock.cache  = cache;

    // add 'PropertyChangeMask' to the dock windows as these window are gonna be
    // more likely to have 'override_redirect' set to true.
    XWindowAttributes attrs;
    XGetWindowAttributes(core->dpy, window, &attrs);
    XSelectInput(core->dpy, window, attrs.your_event_mask | PropertyChangeMask);
}

static DockCache *dcache_get(Window window)
{
    DockCache *d = dock.cache;
    while (d && d->window != window)
        d = d->next;
    return d;
}

static DockCache *dcache_remove(Window window)
{
    DockCache *d = dock.cache, *prev = NULL;
    for (; d; prev = d, d = d->next) {
        if (d->window == window) {
            if (prev)
                prev->next = d->next;
            else
                dock.cache = d->next;
            break;
        }
    }
    return d;
}

static void manage_dock(Window window)
{
    Monitor *mon = core->mon;
    /* checking if window is of type dock. */ {
        Atom *dock_window = NULL;
        core->get_window_property(window, core->netatoms[NET_WM_WINDOW_TYPE], 1,
                                  (uint8_t **)&dock_window);
        if (!dock_window ||
            *dock_window != core->netatoms[NET_WM_WINDOW_TYPE_DOCK])
            return;
        XFree(dock_window);
    }

    // getting strut values for the dock type window.
    int nstrut     = NullStrut;
    int64_t *strut = NULL;
    core->get_window_property(window, core->netatoms[NET_WM_STRUT_PARTIAL],
                              sizeof(int64_t) * nstrut, (uint8_t **)&strut);
    if (!strut)
        core->get_window_property(window, core->netatoms[NET_WM_STRUT],
                                  sizeof(int64_t) * (nstrut = 4),
                                  (uint8_t **)&strut);
    if (!strut)
        return;
    // add or update cache, depending on if the DockCache already exists.
    DockCache *cache;
    (cache = dcache_get(window)) ? dcache_update(cache, strut, nstrut)
                                 : dcache_put(window, strut, nstrut);
    XFree(strut);

    update_screen_geometry();
    mon_applylayout(mon);
}

static void unmanage_dock(Window window)
{
    Monitor *mon = core->mon;
    DockCache *cache;
    if (!(cache = dcache_remove(window)))
        return;
    free(cache);
    update_screen_geometry();
    mon_applylayout(mon);
}

void dock_mapnotify(const XEvent *xevent)
{
    const XMapEvent *e = &xevent->xmap;
    Monitor *mon       = core->mon;
    if (!e->override_redirect || ws_getclient(curr_ws(mon), e->window))
        return;
    manage_dock(e->window);
}

void dock_propertynotify(const XEvent *xevent)
{
    const XPropertyEvent *e = &xevent->xproperty;
    if (e->atom != core->netatoms[NET_WM_STRUT] &&
        e->atom != core->netatoms[NET_WM_STRUT_PARTIAL])
        return;
    e->state == PropertyNewValue ? manage_dock(e->window)
                                 : unmanage_dock(e->window);
}

void dock_unmapnotify(const XEvent *xevent)
{
    unmanage_dock(xevent->xunmap.window);
}

void dock_destroynotify(const XEvent *xevent)
{
    unmanage_dock(xevent->xdestroywindow.window);
}

void dock_toggle(const Arg *arg)
{
    (void)arg;
    Monitor *mon = core->mon;
    dock.visible = !dock.visible;
    update_screen_geometry();
    mon_applylayout(mon);
}
