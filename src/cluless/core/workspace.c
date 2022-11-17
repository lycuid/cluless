#include "workspace.h"
#include <config.h>
#include <string.h>

void ws_init(Workspace *ws, const char *id)
{
  ws->id      = id;
  ws->cl_head = NULL;
  lm_reset(&(ws->layout_manager));
}

Client *ws_getclient(Workspace *ws, Window w)
{
  if (!ws)
    return NULL;
  Client *c = ws->cl_head;
  while (c && c->window != w)
    c = c->next;
  return c;
}

Client *ws_find(Workspace *ws, State flags)
{
  if (!ws)
    return NULL;
  Client *c = ws->cl_head;
  while (c && !IS_SET(c->state, flags))
    c = c->next;
  return c;
}

void ws_attachclient(Workspace *ws, Client *c)
{
  if (!c || !ws)
    return;
  for (Client *cl = c; cl; cl = cl->next)
    lm_decorate_client(&ws->layout_manager, cl);
  ws->cl_head = cl_append(c, ws->cl_head);
  if (ws->cl_head)
    ws->cl_head->prev = NULL;
}

void ws_detachclient(Workspace *ws, Client *c)
{
  if (!c || !ws)
    return;
  if (c->next)
    c->next->prev = c->prev;
  if (c->prev)
    c->prev->next = c->next;
  else // client is on top of the list.
    ws->cl_head = c->next;
  c->next = c->prev = NULL;
}

void ws_clmoveup(Workspace *ws, Client *c)
{
  if (!c || !c->prev || !ws)
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
    ws->cl_head = c; // (Head [c]) <-> [p1] <-> [n]
}

void ws_clmovedown(Workspace *ws, Client *c)
{
  if (!c || !c->next || !ws)
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
    ws->cl_head = n1; // (Head [n1]) <-> [c] <-> [n2]
}
