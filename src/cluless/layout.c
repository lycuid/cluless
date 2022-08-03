#include "layout.h"
#include <cluless/core.h>

void lm_decorate_client(LayoutManager *lm, Client *c)
{
  XSetWindowBorderWidth(core->dpy, c->window, lm->borderpx);
}
