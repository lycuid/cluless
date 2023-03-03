#include "tall.h"
#include <X11/Xlib.h>
#include <cluless/misc/magnify.h>

void tall(Monitor *mon)
{
  int nstack = -1;
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    nstack += !IS_SET(c->state, CL_UNTILED_STATE);
  if (nstack == -1)
    return;
  const LayoutManager *lm = &mon->selws->layout_manager;
  Geometry draw_region    = lm_drawregion(lm, &mon->screen);
  const int top = lm_top(lm, &draw_region), left = lm_left(lm, &draw_region),
            offset = lm_offset(lm);

#define mid (draw_region.w / 2)
  Client *master = mon->selws->cl_head, *stack;
  if (IS_SET(master->state, CL_UNTILED_STATE))
    master = cl_nexttiled(master);
  int x = left, y = top, w = (nstack ? mid : draw_region.w) - offset,
      h = draw_region.h - offset;
  if (!IS_SET(master->state, ClActive) || !magnify(mon, master, x, y, w, h))
    XMoveResizeWindow(core->dpy, master->window, x, y, w, h);

  if (nstack && (stack = cl_nexttiled(master))) {
    x = left + mid, y = top, w = mid - offset,
    h = (draw_region.h / nstack) - offset;
    for (; stack; y += h + offset, stack = cl_nexttiled(stack)) {
      if (!IS_SET(stack->state, ClActive) || !magnify(mon, stack, x, y, w, h))
        XMoveResizeWindow(core->dpy, stack->window, x, y, w, h);
    }
  }
}
