#include "docks.h"
#include <X11/Xlib.h>
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

static inline void update_screen_geometry(Monitor *mon)
{
  // @TODO: '_NET_WM_STRUT_PARTIAL' not implemented (currently unnecessary, for
  // my personal use case, also, I don't understand it completely).
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

static void dcache_put(Window window, int64_t *strut, int nstrut)
{
  DockCache *cache = malloc(sizeof(DockCache));
  cache->window    = window;
  memset(cache->strut, 0, sizeof(int64_t) * NullStrut);
  memcpy(cache->strut, strut, sizeof(int64_t) * nstrut);
  cache->next = dock.cache;
  dock.cache  = cache;
}

static DockCache *dcache_get(Window window)
{
  DockCache *d = dock.cache;
  for (; d && d->window != window; d = d->next)
    ;
  return d;
}

static DockCache *dcache_remove(Window window)
{
  DockCache *d = dock.cache, *prev = NULL;
  for (; d; prev = d, d = d->next)
    if (d->window == window) {
      if (prev)
        prev->next = d->next;
      else
        dock.cache = d->next;
      break;
    }
  return d;
}

static inline void manage_dock(Monitor *mon, Window window)
{
  if (dcache_get(window))
    return;

  // checking if window is of type dock.
  Atom *dock_window = NULL;
  core->get_window_property(window, core->netatoms[NET_WM_WINDOW_TYPE], 1,
                            (uint8_t **)&dock_window);
  if (!dock_window || *dock_window != core->netatoms[NET_WM_WINDOW_TYPE_DOCK])
    return;
  XFree(dock_window);

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
  dcache_put(window, strut, nstrut);
  XFree(strut);

  update_screen_geometry(mon);
  mon_applylayout(mon);
}

static inline void unmanage_dock(Monitor *mon, Window window)
{
  DockCache *cache;
  if (!(cache = dcache_remove(window)))
    return;
  free(cache);
  update_screen_geometry(mon);
  mon_applylayout(mon);
}

void dock_mapnotify(Monitor *mon, const XEvent *xevent)
{
  const XMapEvent *e = &xevent->xmap;
  if (!e->override_redirect || ws_getclient(mon->selws, e->window))
    return;
  manage_dock(mon, e->window);
}

void dock_propertynotify(Monitor *mon, const XEvent *xevent)
{
  const XPropertyEvent *e = &xevent->xproperty;
  if (e->atom != core->netatoms[NET_WM_STRUT] &&
      e->atom != core->netatoms[NET_WM_STRUT_PARTIAL])
    return;
  e->state == PropertyNewValue ? manage_dock(mon, e->window)
                               : unmanage_dock(mon, e->window);
}

void dock_unmapnotify(Monitor *mon, const XEvent *xevent)
{
  unmanage_dock(mon, xevent->xunmap.window);
}

void dock_destroynotify(Monitor *mon, const XEvent *xevent)
{
  unmanage_dock(mon, xevent->xdestroywindow.window);
}

void dock_toggle(Monitor *mon, const Arg *arg)
{
  (void)arg;
  dock.visible = !dock.visible;
  update_screen_geometry(mon);
  mon_applylayout(mon);
}
