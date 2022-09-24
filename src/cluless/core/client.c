#include "client.h"
#include <cluless/core.h>
#include <stdlib.h>

#define vec_grow(v)                                                            \
  (v)->pool =                                                                  \
      realloc((v)->pool, ((v)->capacity += (1 << 5)) * sizeof(struct Cell));

#define vec_append(v, c)                                                       \
  do {                                                                         \
    if ((v)->size == (v)->capacity)                                            \
      vec_grow((v));                                                           \
    (v)->pool[(v)->size++].client = c;                                         \
  } while (0);

#define vec_remove(v, c)                                                       \
  do {                                                                         \
    for (size_t i = 0, size = (v)->size; size == (v)->size && i < size; ++i)   \
      if ((v)->pool[i].client == c)                                            \
        (v)->pool[i] = (v)->pool[--(v)->size];                                 \
  } while (0);

static ClientArray cl_array;
const ClientArray *const cl_register = &cl_array;

Client *cl_alloc(Window w)
{
  XSizeHints size;
  long flags;
  XGetWMNormalHints(core->dpy, w, &size, &flags);
  Client *c = malloc(sizeof(Client));
  vec_append(&cl_array, c);
  c->state  = 0x0;
  c->window = w;
  c->prev   = NULL;
  c->next   = NULL;
  c->minw   = IS_SET(flags, PMinSize) ? size.min_width : 10;
  c->minh   = IS_SET(flags, PMinSize) ? size.min_height : 10;
  return c;
}

void cl_free(Client *c)
{
  vec_remove(&cl_array, c);
  free(c);
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
