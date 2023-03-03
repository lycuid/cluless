#include "magnify.h"
#include <cluless/core/client.h>
#include <cluless/core/workspace.h>
#include <stdbool.h>

static bool Magnify = false;

static inline int clamp(int x, int l, int h)
{
  return x < l ? l : x > h ? h : x;
}

bool magnify(Monitor *mon, Client *c, int cx, int cy, int cw, int ch)
{
  if (!Magnify || !c || IS_SET(c->state, CL_UNTILED_STATE))
    return false;
  const LayoutManager *lm = &mon->selws->layout_manager;
  Geometry draw_region    = lm_drawregion(lm, &mon->screen);
  const int top = lm_top(lm, &draw_region), left = lm_left(lm, &draw_region),
            offset = lm_offset(lm);
  int x = cx, y = cy,
      w = clamp(cw * 1.2, (draw_region.w - offset) / 3, draw_region.w - offset),
      h = clamp(ch * 1.2, (draw_region.h - offset) / 3, draw_region.h - offset);

  x = clamp(x - (w - cw) / 2, left, left + draw_region.w - offset - w),
  y = clamp(y - (h - ch) / 2, top, top + draw_region.h - offset - h);
  return XMoveResizeWindow(core->dpy, c->window, x, y, w, h);
}

void magnify_toggle(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Magnify = !Magnify;
  mon_applylayout(mon);
}
