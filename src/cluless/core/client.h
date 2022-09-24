#ifndef __CORE__CLIENT_H__
#define __CORE__CLIENT_H__

#include <X11/Xlib.h>
#include <cluless/core.h>

#define ClActive     (1 << 0)
#define ClFloating   (1 << 1)
#define ClFullscreen (1 << 2)
#define ClMoving     (1 << 3)
#define ClResizing   (1 << 4)

#define CL_UNTILED_STATE ClFloating | ClFullscreen

#define FOREACH_AVAILABLE_CLIENT(c)                                            \
  for (int keep = 1, it = 0, size = cl_register->size; keep && it < size;      \
       keep = !keep, ++it)                                                     \
    for (c = cl_register->pool[it].client; keep; keep = !keep)

#define cl_neighbour(c) ((c) ? (c)->prev ? (c)->prev : (c)->next : NULL)

typedef struct Client {
  State state;
  Window window;
  int minw, minh;
  struct Client *prev, *next;
} Client;

typedef struct ClientArray {
  struct Cell {
    Client *client;
  } * pool;
  size_t size, capacity;
} ClientArray;
extern const ClientArray *const cl_register;

Client *cl_alloc(Window);
void cl_free(Client *);
Client *cl_nexttiled(Client *);
Client *cl_last(Client *);

#endif
