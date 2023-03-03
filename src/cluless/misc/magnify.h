#ifndef __MISC__MAGNIFY_H__
#define __MISC__MAGNIFY_H__

#include <cluless/bindings.h>
#include <cluless/core/client.h>
#include <cluless/core/monitor.h>
#include <stdbool.h>

bool magnify(Monitor *, Client *c, int, int, int, int);
void magnify_toggle(Monitor *, const Arg *);

#endif
