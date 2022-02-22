#include "workspace.h"
#include <config.h>
#include <string.h>

void ws_init(Workspace *ws, const char *id)
{
  ws->id              = id;
  ws->cl_head         = NULL;
  ws->window_gappx    = window_gappx;
  ws->screen_gappx    = screen_gappx;
  ws->borderpx        = borderpx;
  ws->border_active   = border_active;
  ws->border_inactive = border_inactive;
  ws->layoutidx       = 0;
}

// O(n)
Client *ws_getclient(Workspace *ws, Window w)
{
  Client *c = ws->cl_head;
  for (; c && c->window != w; c = c->next)
    ;
  return c;
}

// O(n)
Client *ws_getactive(Workspace *ws)
{
  Client *c = ws->cl_head;
  for (; c && !IsSet(c->state, ClActive); c = c->next)
    ;
  return c;
}

// O(1)
void ws_attachclient(Workspace *ws, Client *c)
{
  if (!c)
    return;
  c->next = ws->cl_head;
  c->prev = NULL;
  if (ws->cl_head)
    ws->cl_head->prev = c;
  ws->cl_head = c;
}

// O(1)
// detached client still keeps pointers to its neighbour clients (necessary).
Client *ws_detachclient(Workspace *ws, Window w)
{
  Client *c = ws_getclient(ws, w);
  if (!c)
    return NULL;
  if (c->next)
    c->next->prev = c->prev;
  // if 'prev' pointer is null, then the client is on top of the list.
  if (c->prev)
    c->prev->next = c->next;
  else
    ws->cl_head = c->next;
  return c;
}

// O(1)
void ws_client_moveup(Workspace *ws, Client *c)
{
  if (!c || !c->prev)
    return;
  // [p2] <-> [p1] <-> [c] <-> [n]
  Client *p1 = c->prev, *p2 = p1->prev, *n = c->next;
  c->next  = p1; // [c] -> [p1]
  c->prev  = p2; // [p2] <- [c] -> [p1]
  p1->next = n;  // [p2] <- [c] -> [p1] -> [n]
  p1->prev = c;  // [p2] <- [c] <-> [p1] -> [n]
  if (n)
    n->prev = p1; // [p2] <- [c] <-> [p1] <-> [n]
  if (p2)
    p2->next = c; // [p2] <-> [c] <-> [p1] <-> [n]
  else
    ws->cl_head = c; // (Stack [c]) <-> [p1] <-> [n]
}

// O(1)
void ws_client_movedown(Workspace *ws, Client *c)
{
  if (!c || !c->next)
    return;
  // [p] <-> [c] <-> [n1] <-> [n2]
  Client *p = c->prev, *n1 = c->next, *n2 = n1->next;
  c->prev  = n1; // [n1] <- [c]
  c->next  = n2; // [n1] <- [c] -> [n2]
  n1->prev = p;  // [p] <- [n1] <- [c] -> [n2]
  n1->next = c;  // [p] <- [n1] <-> [c] -> [n2]
  if (n2)
    n2->prev = c; // [p] <- [n1] <-> [c] <-> [n2]
  if (p)
    p->next = n1; // [p] <-> [n1] <-> [c] <-> [n2]
  else
    ws->cl_head = n1; // (Stack [n1]) <-> [c] <-> [n2]
}

void ws_dump(Workspace *ws)
{
  Client *c = ws->cl_head;
  if (c) {
    LOG("%lu%s", c->window, IsSet(c->state, ClActive) ? "*" : "");
    for (; c->next; c = c->next)
      LOG(" -> [%lu] %lu%s", c->window, c->next->window,
          IsSet(c->next->state, ClActive) ? "*" : "");
    LOG(".\n");
  }
}

const Layout *ws_getlayout(Workspace *ws)
{
  return &layouts[ws->layoutidx % Length(layouts)];
}
