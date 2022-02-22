#ifndef __EWMH__DOCKS_H__
#define __EWMH__DOCKS_H__

#include "include/monitor.h"

void dock_mapnotify(Monitor *, const XEvent *);
void dock_propertynotify(Monitor *, const XEvent *);
void dock_destroynotify(Monitor *, const XEvent *);

typedef void (*DockEventHandler)(Monitor *, const XEvent *);
static const DockEventHandler dock_event_handlers[LASTEvent] = {
    [MapNotify]      = dock_mapnotify,
    [PropertyNotify] = dock_propertynotify,
    [DestroyNotify]  = dock_destroynotify,
};

#endif
