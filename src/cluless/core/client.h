#ifndef __CORE__CLIENT_H__
#define __CORE__CLIENT_H__

#include <X11/Xlib.h>
#include <cluless/core.h>
#include <cluless/core/vector.h>

#define ClActive     (1 << 0)
#define ClFloating   (1 << 1)
#define ClFullscreen (1 << 2)
#define ClMoving     (1 << 3)
#define ClResizing   (1 << 4)
#define ClCompanion  (1 << 5)
#define ClBypassed   (1 << 6)

#define CL_UNTILED_STATE ClFloating | ClFullscreen | ClBypassed

#define cl_neighbour(c) ((c) ? (c)->prev ? (c)->prev : (c)->next : NULL)

typedef struct Client {
  State state;
  Window window;
  int minw, minh;
  struct Client *prev, *next;
} Client;

typedef Vector(Client *) ClientVector;
extern const ClientVector *const cl_register;
#define FOREACH_AVAILABLE_CLIENT(c) VEC_FOREACH(c, cl_register)

Client *cl_alloc(Window);
void cl_free(Client *);
Client *cl_append(Client *, Client *);
Client *cl_nexttiled(Client *);
Client *cl_last(Client *);

#endif
