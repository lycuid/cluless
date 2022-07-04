#ifndef __CORE_H__
#define __CORE_H__

#include "debug.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>
#include <stdint.h>

#define RootWindowEventMasks SubstructureRedirectMask | SubstructureNotifyMask

// misc.
#define ButtonMasks (ButtonPressMask | ButtonReleaseMask)
#define LENGTH(s)   (sizeof(s) / sizeof(s[0]))
#define MAX(x, y)   (x) > (y) ? (x) : (y)
#define MIN(x, y)   (x) < (y) ? (x) : (y)

// cannot have nested, variable 'it' will repeat.
#define ITER(iterable) for (size_t it = 0; it < LENGTH(iterable); ++it)
#define FOREACH(var, iterable)                                                 \
  for (int cond = 1, it = 0, size = LENGTH(iterable); cond && it < size;       \
       cond = !cond, it = it + 1)                                              \
    for (var = iterable + it; cond; cond = !cond)

// flag ops.
#define SET(state, mask)    state |= (mask)
#define UNSET(state, mask)  state &= ~(mask)
#define TOGGLE(state, mask) state ^= (mask)
#define IS_SET(state, mask) (state & (mask))

// logging.
#define die(...)                                                               \
  {                                                                            \
    fprintf(stderr, __VA_ARGS__);                                              \
    exit(EXIT_FAILURE);                                                        \
  }
typedef enum {
  FmtWsCurrent,     // Current workspace (String -> String).
  FmtWsHidden,      // Hidden workspace (String -> String).
  FmtWsHiddenEmpty, // Hidden workspace, no windows (String -> String).
  FmtWsSeperator,   // Workspace seperator (String).
  FmtLayout,        // Workspace layout (String -> String).
  FmtSeperator,     // Seperator (String).
  FmtWindowTitle,   // focused window title (String -> String).
  FmtOptionsCount
} Logging;

typedef uint32_t State;
typedef struct {
  int x, y;
  uint32_t w, h;
} Geometry;

enum { CurNormal, CurResize, CurMove, CurNull };
enum {
  // ICCC Atoms.
  WMProtocols,
  WMName,
  WMDeleteWindow,
  WMTransientFor,
  // EWMH Atoms.
  NetWMName,
  NetWMWindowType,
  NetWMWindowTypeDock,
  NetWMStrut,
  NetWMStrutPartial,
  NetActiveWindow,
  NetClientList,
  NullAtom
};

// These are mainly the values that don't (shouldn't) change throughout the
// application lifetime.
typedef struct {
  bool running : 1;
  Display *dpy;
  Window root;
  Cursor cursors[CurNull];
  Atom atoms[NullAtom];
  FILE *statuslogger;
} Context;

// @FIXME: not sure about this function.
Context *request_context(void);
Context *create_context(void);
Window input_focused_window(void);
Geometry get_screen_rect(void);
bool send_event(Window, Atom);
int get_window_property(Window, Atom, int, uint8_t **);
int get_window_title(Window, XTextProperty *);

#endif
