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

#define cl_neighbour(c) (c ? c->prev ? c->prev : c->next : NULL)

typedef struct Client {
  State state;
  Window window;
  int minw, minh;
  struct Client *prev, *next;
} Client;

Client *cl_create(Window);
Client *cl_nexttiled(Client *);
Client *cl_last(Client *);

#endif
