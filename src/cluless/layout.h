#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include <cluless/core/client.h>
#include <stdint.h>

typedef struct {
  char *symbol;
  void (*apply)();
} Layout;

typedef struct {
  uint32_t index, screen_gappx, window_gappx, borderpx;
} LayoutManager;

#define lm_reset(lm)                                                           \
  {                                                                            \
    (lm)->index        = 0;                                                    \
    (lm)->screen_gappx = ScreenGapPX;                                          \
    (lm)->window_gappx = WindowGapPX;                                          \
    (lm)->borderpx     = BorderPX;                                             \
  }

#define lm_drawregion(lm, region)                                              \
  (Geometry)                                                                   \
  {                                                                            \
    .x = (region)->x + (lm)->screen_gappx,                                     \
    .y = (region)->y + (lm)->screen_gappx,                                     \
    .w = (region)->w - ((lm)->screen_gappx * 2),                               \
    .h = (region)->h - ((lm)->screen_gappx * 2)                                \
  }

#define lm_nextlayout(lm) ((lm)->index = ((lm)->index + 1) % LENGTH(layouts))
#define lm_getlayout(lm)  (&layouts[(lm)->index % LENGTH(layouts)])

#define lm_top(lm, region)  ((region)->x + (lm)->screen_gappx)
#define lm_left(lm, region) ((region)->y + (lm)->screen_gappx)
#define lm_offset(lm)       (((lm)->borderpx + (lm)->screen_gappx) * 2)

void lm_decorate_client(LayoutManager *, Client *);
void lm_undecorate_client(LayoutManager *, Client *);

#endif
