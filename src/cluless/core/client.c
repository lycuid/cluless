#include "client.h"
#include <stdlib.h>

Client *cl_create(Window w)
{
  Context *ctx = request_context();
  Client *c    = malloc(sizeof(Client));
  c->state     = 0x0;
  c->window    = w;
  c->prev      = NULL;
  c->next      = NULL;
  XSizeHints size;
  long flags;
  XGetWMNormalHints(ctx->dpy, c->window, &size, &flags);
  c->minw = IS_SET(flags, PMinSize) ? size.min_width : 10;
  c->minh = IS_SET(flags, PMinSize) ? size.min_height : 10;
  return c;
}

// O(n)
Client *cl_nexttiled(Client *c)
{
  if (c)
    for (c = c->next; c && IS_SET(c->state, CL_UNTILED_STATE); c = c->next)
      ;
  return c;
}

// O(n)
Client *cl_last(Client *c)
{
  for (; c && c->next; c = c->next)
    ;
  return c;
}
