#ifndef __SCRATCHPAD_H__
#define __SCRATCHPAD_H__

#include <include/bindings.h>
#include <include/core/monitor.h>

// used to create a togglable scratchpad out of a 'Client'.
// uses a unique char id (provided in 'Arg') as a reference for toggling.
void sch_fromclient(Monitor *, const Arg *);
// toggles scratchpad (with unique char id) on/off the screen, independent of
// the workspace.
void sch_toggle(Monitor *, const Arg *);
// to avoid memory segfaults and stuff (cleanup function).
void sch_clientremove(Monitor *, Client *);

static const Hook sch_hooks[NullHook] = {[ClientRemove] = sch_clientremove};

#endif
