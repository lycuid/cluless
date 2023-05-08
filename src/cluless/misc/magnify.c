#include "magnify.h"
#include <cluless/core.h>
#include <cluless/core/client.h>
#include <cluless/core/workspace.h>
#include <config.h>

static inline int clamp(int x, int l, int h)
{
    return x < l ? l : x > h ? h : x;
}

void magnify(Window window, const LayoutManager *lm, const Geometry *wg,
             const Geometry *draw_region)
{
    if (!lm || !wg || !draw_region)
        return;
    const int top = lm_top(lm, draw_region), left = lm_left(lm, draw_region),
              off = lm_offset(lm);
    int x = wg->x, y = wg->y,
        w = clamp(wg->w * MagnifyW, (draw_region->w - off) / 3,
                  draw_region->w - off),
        h = clamp(wg->h * MagnifyH, (draw_region->h - off) / 3,
                  draw_region->h - off);

    x = clamp(x - (w - wg->w) / 2, left, left + draw_region->w - off - w),
    y = clamp(y - (h - wg->h) / 2, top, top + draw_region->h - off - h);
    XMoveResizeWindow(core->dpy, window, x, y, w, h);
}

void magnify_toggle(const Arg *arg)
{
    (void)arg;
    Monitor *mon                       = core->mon;
    mon->selws->layout_manager.magnify = !mon->selws->layout_manager.magnify;
    mon_applylayout(mon);
}
