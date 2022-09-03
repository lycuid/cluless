#ifndef __EWMH__DOCKS_H__
#define __EWMH__DOCKS_H__

#include <cluless/bindings.h>
#include <cluless/core/monitor.h>

void dock_toggle(Monitor *, const Arg *);
void dock_mapnotify(Monitor *, const XEvent *);
void dock_propertynotify(Monitor *, const XEvent *);
void dock_unmapnotify(Monitor *, const XEvent *);
void dock_destroynotify(Monitor *, const XEvent *);

static const EventHandler dock_event_handlers[LASTEvent] = {
    [MapNotify]      = dock_mapnotify,
    [PropertyNotify] = dock_propertynotify,
    [UnmapNotify]    = dock_unmapnotify,
    [DestroyNotify]  = dock_destroynotify,
};

#endif
