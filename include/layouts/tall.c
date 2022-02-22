#include "tall.h"
#include <X11/Xlib.h>

void tall(Monitor *mon)
{
  int nstack = -1;
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    nstack += !IsSet(c->state, ClFloating | ClTransient);
  if (nstack == -1)
    return;
  Workspace *ws        = mon->selws;
  Geometry draw_region = mon_drawregion(mon, ws);

  // tile 'Master' window.
  Client *master = mon->selws->cl_head, *stack;
  if (IsSet(master->state, ClFloating | ClTransient))
    master = cl_nexttiled(master);
  int mx = draw_region.x + ws->window_gappx,
      my = draw_region.y + ws->window_gappx,
      mw = (nstack ? (draw_region.w / 2) : draw_region.w) -
           ((ws->borderpx + ws->window_gappx) * 2),
      mh = draw_region.h - ((ws->borderpx + ws->window_gappx) * 2);
  XMoveResizeWindow(mon->ctx->dpy, master->window, mx, my, mw, mh);

  // tile 'Stack' windows.
  if (nstack && (stack = cl_nexttiled(master))) {
    int sx = draw_region.x + (draw_region.w / 2) + ws->window_gappx,
        sy = draw_region.y + ws->window_gappx,
        sw = (draw_region.w / 2) - ((ws->borderpx + ws->window_gappx) * 2),
        sh = (draw_region.h / nstack) - ((ws->borderpx + ws->window_gappx) * 2);
    for (; stack; sy += sh + ((ws->borderpx + ws->window_gappx) * 2),
                  stack = cl_nexttiled(stack))
      XMoveResizeWindow(mon->ctx->dpy, stack->window, sx, sy, sw, sh);
  }
  mon_restack(mon);
}
