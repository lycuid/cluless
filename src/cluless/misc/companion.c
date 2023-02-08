#include "companion.h"
#include <config.h>

Client *companion_remove(Workspace *from)
{
  Client *companion = NULL;
  for (Client *c; (c = ws_find(from, ClCompanion)); companion = c)
    if (ws_detachclient(from, c), companion)
      cl_append(companion, c);
  return companion;
}

void companion_insert(Client *companion, Workspace *to)
{
  // go to the first companion client and attach companion clients on 'to'
  // workspace after all clients have been mapped.
  while (companion && companion->prev)
    companion = companion->prev;
  ws_attachclient(to, companion);
}

void companion_toggle(Monitor *mon, const Arg *arg)
{
  (void)arg;
  Client *c = ws_find(mon->selws, ClActive);
  if (!c)
    return;
  TOGGLE(c->state, ClCompanion);
  XSetWindowBorder(core->dpy, c->window,
                   IS_SET(c->state, ClCompanion) ? CompanionActive
                                                 : BorderActive);
}
