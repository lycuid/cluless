#ifndef __MISC__SCRATCHPAD_H__
#define __MISC__SCRATCHPAD_H__

#include <cluless/bindings.h>
#include <cluless/core.h>

// used to create a togglable scratchpad from a 'Client' object.
// uses a unique char id (provided in 'Arg') as a reference for toggling.
void sch_fromclient(const Arg *);
// toggles scratchpad (with unique char id) on/off the screen, independent of
// the workspace.
void sch_toggle(const Arg *);
// unmanage and nullify saved client pointer in 'sch_clients'.
void sch_destroynotify(const XEvent *);
void sch_clientremove(Client *);

static const EventHandler sch_event_handlers[LASTEvent] = {
    [DestroyNotify] = sch_destroynotify,
};

static const ClientHook sch_client_hooks[NullHookType] = {
    [ClientRemove] = sch_clientremove,
};

#endif
