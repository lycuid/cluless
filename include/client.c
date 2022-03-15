#include "client.h"
#include "base.h"
#include <stdlib.h>

Client *cl_create(Window w)
{
  Client *c = malloc(sizeof(Client));
  c->state  = 0x0;
  c->window = w;
  c->prev   = NULL;
  c->next   = NULL;
  return c;
}

// O(n)
// next non-floating, non-fullscreen window.
Client *cl_nexttiled(Client *c)
{
  if (c)
    for (c = c->next; c && IsSet(c->state, CL_UNTILED_STATE); c = c->next)
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
