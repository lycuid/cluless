#ifndef __MISC__MAGNIFY_H__
#define __MISC__MAGNIFY_H__

#include <cluless/bindings.h>
#include <cluless/core/client.h>
#include <cluless/layout.h>

void magnify(Window, const LayoutManager *, const Geometry *, const Geometry *);
void magnify_toggle(const Arg *);

#endif
