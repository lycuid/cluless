#include "tall.h"
#include <X11/Xlib.h>

void tall(Monitor *mon)
{
  int nstack = -1;
  for (Client *c = mon->selws->cl_head; c; c = c->next)
    nstack += !IsSet(c->state, ClFloating | ClTransient);
  if (nstack == -1)
    return;
  Geometry draw_region = mon_drawregion(mon, mon->selws);

  // tile 'Master' window.
  Client *master = mon->selws->cl_head, *stack;
  if (IsSet(master->state, ClFloating | ClTransient))
    master = cl_nexttiled(master);
  int mx = draw_region.x + mon->selws->window_gappx,
      my = draw_region.y + mon->selws->window_gappx,
      mw = (nstack ? (draw_region.w / 2) : draw_region.w) -
           ((mon->selws->borderpx + mon->selws->window_gappx) * 2),
      mh = draw_region.h -
           ((mon->selws->borderpx + mon->selws->window_gappx) * 2);
  XMoveResizeWindow(mon->ctx->dpy, master->window, mx, my, mw, mh);

  // tile 'Stack' windows.
  if (nstack && (stack = cl_nexttiled(master))) {
    int sx = draw_region.x + (draw_region.w / 2) + mon->selws->window_gappx,
        sy = draw_region.y + mon->selws->window_gappx,
        sw = (draw_region.w / 2) -
             ((mon->selws->borderpx + mon->selws->window_gappx) * 2),
        sh = (draw_region.h / nstack) -
             ((mon->selws->borderpx + mon->selws->window_gappx) * 2);
    for (; stack;
         sy += sh + ((mon->selws->borderpx + mon->selws->window_gappx) * 2),
         stack = cl_nexttiled(stack))
      XMoveResizeWindow(mon->ctx->dpy, stack->window, sx, sy, sw, sh);
  }
}
