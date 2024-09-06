#include "tall.h"
#include <X11/Xlib.h>
#include <cluless/core.h>
#include <cluless/layout.h>
#include <cluless/misc/magnify.h>

void tall(void)
{
    Monitor *mon = core->mon;
    int nstack   = -1;
    for (Client *c = curr_ws(mon)->cl_head; c; c = c->next)
        nstack += !IS_SET(c->state, CL_UNTILED_STATE);
    if (nstack == -1)
        return;
    const LayoutManager *lm    = &curr_ws(mon)->layout_manager;
    const Geometry draw_region = lm_drawregion(lm, &mon->screen);
    const int top = lm_top(lm, &draw_region), left = lm_left(lm, &draw_region),
              offset = lm_offset(lm);

#define mid (draw_region.w / 2)
    Client *master = curr_ws(mon)->cl_head, *stack;
    if (IS_SET(master->state, CL_UNTILED_STATE))
        master = cl_nexttiled(master);
    int x = left, y = top, w = (nstack ? mid : draw_region.w) - offset,
        h = draw_region.h - offset;
    lm_resize_client(lm, master, &GEOMETRY(x, y, w, h), &draw_region);

    if (nstack && (stack = cl_nexttiled(master))) {
        x = left + mid, y = top, w = mid - offset,
        h = (draw_region.h / nstack) - offset;
        for (; stack; y += h + offset, stack = cl_nexttiled(stack))
            lm_resize_client(lm, stack, &GEOMETRY(x, y, w, h), &draw_region);
    }
}
