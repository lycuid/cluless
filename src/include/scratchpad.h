#ifndef __SCRATCHPAD_H__
#define __SCRATCHPAD_H__

#include <include/bindings.h>
#include <include/core/monitor.h>

// used to create a togglable scratchpad from a 'Client' object.
// uses a unique char id (provided in 'Arg') as a reference for toggling.
void sch_fromclient(Monitor *, const Arg *);
// toggles scratchpad (with unique char id) on/off the screen, independent of
// the workspace.
void sch_toggle(Monitor *, const Arg *);
// unmanage and nullify saved client pointer in 'sch_clients'.
void sch_destroynotify(Monitor *, const XEvent *);
void sch_clientremove(Monitor *, Client *);

static const EventHandler sch_event_handlers[LASTEvent] = {
    [DestroyNotify] = sch_destroynotify};

static const ClientHook sch_client_hooks[NullHook] = {[ClientRemove] =
                                                          sch_clientremove};

#endif
