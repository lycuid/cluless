#include "tall.h"
#include <X11/Xlib.h>

void tall(Monitor *mon)
{
  int nstack = -1;
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    nstack += !IS_SET(c->state, ClFloating | ClTransient);
  if (nstack == -1)
    return;
  LayoutManager *lm    = &mon->selws->layout_manager;
  Geometry draw_region = lm_drawregion(lm, &mon->screen);

  // arrange 'Master' window.
  Client *master = mon->selws->cl_head, *stack;
  if (IS_SET(master->state, ClFloating | ClTransient))
    master = cl_nexttiled(master);
  int mx = draw_region.x + lm->window_gappx,
      my = draw_region.y + lm->window_gappx,
      mw = (nstack ? (draw_region.w / 2) : draw_region.w) -
           ((lm->borderpx + lm->window_gappx) * 2),
      mh = draw_region.h - ((lm->borderpx + lm->window_gappx) * 2);
  XMoveResizeWindow(mon->ctx->dpy, master->window, mx, my, mw, mh);

  // arrange 'Stack' windows.
  if (nstack && (stack = cl_nexttiled(master))) {
    int sx = draw_region.x + (draw_region.w / 2) + lm->window_gappx,
        sy = draw_region.y + lm->window_gappx,
        sw = (draw_region.w / 2) - ((lm->borderpx + lm->window_gappx) * 2),
        sh = (draw_region.h / nstack) - ((lm->borderpx + lm->window_gappx) * 2);
    for (; stack; sy += sh + ((lm->borderpx + lm->window_gappx) * 2),
                  stack = cl_nexttiled(stack))
      XMoveResizeWindow(mon->ctx->dpy, stack->window, sx, sy, sw, sh);
  }
}
