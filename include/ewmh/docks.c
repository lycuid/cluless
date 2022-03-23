#include "docks.h"
#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>

enum {
  Left,
  Right,
  Top,
  Bottom,
  LeftStartY,
  LeftEndY,
  RightStartY,
  RightEndY,
  TopStartX,
  TopEndX,
  BottomStartX,
  BottomEndX,
  StrutEnd
};

typedef struct _DockCache {
  Window windowid;
  int64_t strut[StrutEnd];
  struct _DockCache *next;
} DockCache;

static DockCache *dock_cache = NULL;

void update_screen_geometry(Monitor *mon)
{
  // @TODO: '_NET_WM_STRUT_PARTIAL' not implemented (currently unnecessary, for
  // my personal use case, also, I don't understand it completely).
  int64_t left = 0, right = 0, top = 0, bottom = 0;
  for (DockCache *d = dock_cache; d; d = d->next) {
    left   = Max(left, d->strut[Left]);
    right  = Max(right, d->strut[Right]);
    top    = Max(top, d->strut[Top]);
    bottom = Max(bottom, d->strut[Bottom]);
  }
  mon->screen = get_screen_rect();
  mon->screen.x += left;
  mon->screen.y += top;
  mon->screen.w -= (left + right);
  mon->screen.h -= (top + bottom);
  INFO("Computed Struts: %d %d %d %d\n", mon->screen.x, mon->screen.y,
       mon->screen.w, mon->screen.h);
}

void dcache_insert(Window wid, int64_t *strut, int nstrut)
{
  DockCache *cache = malloc(sizeof(DockCache));
  cache->windowid  = wid;
  memset(cache->strut, 0, sizeof(int64_t) * StrutEnd);
  memcpy(cache->strut, strut, sizeof(int64_t) * nstrut);
  cache->next = dock_cache;
  dock_cache  = cache;
}

DockCache *dcache_get(Window wid)
{
  DockCache *d = dock_cache;
  for (; d && d->windowid != wid; d = d->next)
    ;
  return d;
}

DockCache *dcache_remove(Window wid)
{
  DockCache *d = dock_cache, *prev = NULL;
  for (; d; prev = d, d = d->next)
    if (d->windowid == wid) {
      if (prev)
        prev->next = d->next;
      else
        dock_cache = d->next;
      break;
    }
  return d;
}

void manage_dock(Monitor *mon, Window window)
{
  if (dcache_get(window))
    return;

  // checking if window is of type dock.
  Atom *dock_window = NULL;
  get_window_property(window, mon->ctx->netatoms[NetWMWindowType], 1,
                      (uint8_t **)&dock_window);
  if (!dock_window || *dock_window != mon->ctx->netatoms[NetWMWindowTypeDock])
    return;
  XFree(dock_window);

  // getting strut values for the dock type window.
  int nstrut     = StrutEnd;
  int64_t *strut = NULL;
  get_window_property(window, mon->ctx->netatoms[NetWMStrutPartial],
                      sizeof(int64_t) * nstrut, (uint8_t **)&strut);
  if (!strut)
    get_window_property(window, mon->ctx->netatoms[NetWMStrut],
                        sizeof(int64_t) * (nstrut = 4), (uint8_t **)&strut);
  if (!strut)
    return;
  dcache_insert(window, strut, nstrut);
  XFree(strut);

  update_screen_geometry(mon);
  mon_arrange(mon);
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
  if (e->atom != mon->ctx->netatoms[NetWMStrut] &&
      e->atom != mon->ctx->netatoms[NetWMStrutPartial])
    return;
  manage_dock(mon, e->window);
}

void dock_destroynotify(Monitor *mon, const XEvent *xevent)
{
  const XDestroyWindowEvent *e = &xevent->xdestroywindow;
  DockCache *cache;
  if (!(cache = dcache_remove(e->window)))
    return;
  free(cache);
  update_screen_geometry(mon);
  // @TODO: remove this, added only to avoid crashes.
  mon->ctx->pipefile = NULL;
  mon_arrange(mon);
}
