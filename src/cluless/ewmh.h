#ifndef __EWMH_H__
#define __EWMH_H__

#include <cluless/core.h>

void ewmh_maprequest(const XEvent *);
void ewmh_propertynotify(const XEvent *);
void ewmh_destroynotify(const XEvent *);
void ewmh_focusin(const XEvent *);
void ewmh_focusout(const XEvent *);
void ewmh_clientadd(Client *);
void ewmh_clientremove(Client *);

static const EventHandler ewmh_event_handlers[LASTEvent] = {
    [MapRequest]     = ewmh_maprequest,
    [PropertyNotify] = ewmh_propertynotify,
    [DestroyNotify]  = ewmh_destroynotify,
    [FocusIn]        = ewmh_focusin,
    [FocusOut]       = ewmh_focusout,
};

static const ClientHook ewmh_client_hooks[NullHookType] = {
    [ClientAdd]    = ewmh_clientadd,
    [ClientRemove] = ewmh_clientremove,
};

#endif
