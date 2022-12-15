#include "layout.h"
#include <cluless/core.h>

void lm_decorate_client(LayoutManager *lm, Client *c)
{
  if (!IS_SET(c->state, ClBypassed))
    XSetWindowBorderWidth(core->dpy, c->window, lm->borderpx);
}

void lm_undecorate_client(LayoutManager *lm, Client *c)
{
  (void)lm;
  XSetWindowBorderWidth(core->dpy, c->window, 0);
}
