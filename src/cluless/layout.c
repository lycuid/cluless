#include "layout.h"
#include <cluless/core.h>
#include <cluless/misc/magnify.h>

void lm_decorate_client(const LayoutManager *lm, Client *c)
{
  if (!IS_SET(c->state, ClBypassed))
    XSetWindowBorderWidth(core->dpy, c->window, lm->borderpx);
}

void lm_undecorate_client(const LayoutManager *lm, Client *c)
{
  (void)lm;
  XSetWindowBorderWidth(core->dpy, c->window, 0);
}

void lm_resize_client(const LayoutManager *lm, const Client *c,
                      const Geometry *cl_geometry, const Geometry *draw_region)
{
  if (lm->magnify && IS_SET(c->state, ClActive) &&
      !IS_SET(c->state, CL_UNTILED_STATE))
    magnify(c->window, lm, cl_geometry, draw_region);
  else
    XMoveResizeWindow(core->dpy, c->window, cl_geometry->x, cl_geometry->y,
                      cl_geometry->w, cl_geometry->h);
}
