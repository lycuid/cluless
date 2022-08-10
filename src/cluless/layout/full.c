#include "full.h"

void full(Monitor *mon)
{
  Client *c = mon->selws->cl_head;
  if (!c)
    return;
  uint32_t border_offset = mon->selws->layout_manager.borderpx * 2;
  if (IS_SET(c->state, CL_UNTILED_STATE))
    c = cl_nexttiled(c);
  for (; c; c = cl_nexttiled(c))
    XMoveResizeWindow(core->dpy, c->window, mon->screen.x, mon->screen.y,
                      mon->screen.w - border_offset,
                      mon->screen.h - border_offset);
}
