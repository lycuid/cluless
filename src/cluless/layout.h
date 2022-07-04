#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include <cluless/core/client.h>
#include <stdint.h>

typedef struct {
  char *symbol;
  void (*apply)();
} Layout;

typedef struct {
  uint32_t index;
  uint32_t screen_gappx, window_gappx;
  uint32_t borderpx, border_active, border_inactive;
} LayoutManager;

#define lm_reset(lm)                                                           \
  {                                                                            \
    (lm)->index           = 0;                                                 \
    (lm)->screen_gappx    = screen_gappx;                                      \
    (lm)->window_gappx    = window_gappx;                                      \
    (lm)->borderpx        = borderpx;                                          \
    (lm)->border_active   = border_active;                                     \
    (lm)->border_inactive = border_inactive;                                   \
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

void lm_decorate_client(LayoutManager *, Client *);

#endif
