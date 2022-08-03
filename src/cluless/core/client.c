#include "client.h"
#include <cluless/core.h>
#include <stdlib.h>

Client *cl_create(Window w)
{
  Client *c = malloc(sizeof(Client));
  c->state  = 0x0;
  c->window = w;
  c->prev   = (c->next = NULL);
  XSizeHints size;
  long flags;
  XGetWMNormalHints(core->dpy, c->window, &size, &flags);
  c->minw = IS_SET(flags, PMinSize) ? size.min_width : 10;
  c->minh = IS_SET(flags, PMinSize) ? size.min_height : 10;
  return c;
}

Client *cl_nexttiled(Client *c)
{
  if (c)
    for (c = c->next; c && IS_SET(c->state, CL_UNTILED_STATE); c = c->next)
      ;
  return c;
}

Client *cl_last(Client *c)
{
  for (; c && c->next; c = c->next)
    ;
  return c;
}
