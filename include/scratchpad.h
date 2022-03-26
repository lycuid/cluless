#ifndef __SCRATCHPAD_H__
#define __SCRATCHPAD_H__

#include "bindings.h"
#include "monitor.h"

void sch_create(Monitor *, const Arg *);
void sch_toggle(Monitor *, const Arg *);
void sch_clientremove(Monitor *, Client *);

static const Hook sch_hooks[LASTEvent] = {[ClientRemove] = sch_clientremove};

#endif
