#ifndef __EWMH_H__
#define __EWMH_H__

#include <cluless/core/monitor.h>

void ewmh_maprequest(Monitor *, const XEvent *);
void ewmh_destroynotify(Monitor *, const XEvent *);
void ewmh_focusin(Monitor *, const XEvent *);
void ewmh_focusout(Monitor *, const XEvent *);
void ewmh_clientremove(Monitor *, Client *);

static const EventHandler ewmh_event_handlers[LASTEvent] = {
    [MapRequest]    = ewmh_maprequest,
    [DestroyNotify] = ewmh_destroynotify,
    [FocusIn]       = ewmh_focusin,
    [FocusOut]      = ewmh_focusout,
};

static const ClientHook ewmh_client_hooks[NullHookType] = {
    [ClientRemove] = ewmh_clientremove,
};

#endif
