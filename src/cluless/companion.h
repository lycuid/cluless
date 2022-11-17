#ifndef __COMPANION_H__
#define __COMPANION_H__

#include <cluless/bindings.h>
#include <cluless/core/monitor.h>

#define WITH_COMPANION_CLIENTS(from, to)                                       \
  for (bool __cond = true; __cond;)                                            \
    for (Client *companion = companion_remove(from); __cond;                   \
         companion_insert(companion, to), __cond = !__cond)

Client *companion_remove(Workspace *);
void companion_insert(Client *, Workspace *);
void companion_toggle(Monitor *, const Arg *);

#endif
