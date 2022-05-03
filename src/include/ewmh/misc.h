#ifndef __EWMH__MISC_H__
#define __EWMH__MISC_H__

#include <include/core/monitor.h>

void ewmh_clientadd(Monitor *, Client *);
void ewmh_clientremove(Monitor *, Client *);
void ewmh_focusin(Monitor *, const XEvent *);
void ewmh_focusout(Monitor *, const XEvent *);

static const Hook ewmh_hooks[NullHook] = {
    [ClientAdd] = ewmh_clientadd, [ClientRemove] = ewmh_clientremove};

static const EventHandler ewmh_event_handlers[LASTEvent] = {
    [FocusIn] = ewmh_focusin, [FocusOut] = ewmh_focusout};

#endif
