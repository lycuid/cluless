#ifndef __CORE_H__
#define __CORE_H__

#include "debug.h"
#include <X11/Xutil.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define RootWindowEventMasks (SubstructureRedirectMask | SubstructureNotifyMask)
#define ButtonMasks          (ButtonPressMask | ButtonReleaseMask)

#define LENGTH(s) (sizeof(s) / sizeof(s[0]))
#define MAX(x, y) (x) > (y) ? (x) : (y)
#define MIN(x, y) (x) < (y) ? (x) : (y)

#define ENUM(identifier, ...) /* 'NULL' terminated enum values. */             \
  typedef enum { __VA_ARGS__, Null##identifier } identifier

#define ITER(iterable)                                                         \
  /* @NOTE: In case of nested, 'it' will repeat */                             \
  for (size_t it = 0; it < LENGTH(iterable); ++it)

#define FOREACH(var, iterable)                                                 \
  /* @NOTE: In case of nested, 'it' will repeat */                             \
  for (int keep = 1, it = 0, size = LENGTH(iterable); keep && it < size;       \
       keep = !keep, it++)                                                     \
    for (var = iterable + it; keep; keep = !keep)

#define SET(state, mask)    state |= (mask)
#define UNSET(state, mask)  state &= ~(mask)
#define TOGGLE(state, mask) state ^= (mask)
#define IS_SET(state, mask) ((state & (mask)) != 0)

#define die(...)                                                               \
  {                                                                            \
    fprintf(stderr, __VA_ARGS__);                                              \
    exit(EXIT_FAILURE);                                                        \
  }

typedef uint32_t State;
typedef struct {
  int x, y;
  uint32_t w, h;
} Geometry;

ENUM(CursorType, CurNormal, CurResize, CurMove);
ENUM(WMAtom, WM_PROTOCOLS, WM_NAME, WM_DELETE_WINDOW, WM_TRANSIENT_FOR);
ENUM(NetAtom, NET_WM_NAME, NET_WM_WINDOW_TYPE, NET_WM_WINDOW_TYPE_DOCK,
     NET_WM_STRUT, NET_WM_STRUT_PARTIAL, NET_ACTIVE_WINDOW, NET_CLIENT_LIST);

// These are mainly the values that don't (shouldn't) change throughout the
// application lifetime.
typedef struct Core {
  bool running;
  Display *dpy;
  Window root;
  Cursor cursors[NullCursorType];
  Atom wmatoms[NullWMAtom], netatoms[NullNetAtom];
  FILE *logger;

  void (*init)(void);
  Window (*input_focused_window)(void);
  Geometry (*get_screen_rect)(void);
  bool (*send_event)(Window, Atom);
  int (*get_window_property)(Window, Atom, int, uint8_t **);
  int (*get_window_title)(Window, XTextProperty *);
  uint32_t (*get_window_list)(Window **);
  void (*stop_running)();
} Core;
extern const Core *const core;

#endif
