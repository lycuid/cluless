#ifndef __BASE_H__
#define __BASE_H__

#include "debug.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>
#include <stdint.h>

#define RootWindowEventMasks SubstructureRedirectMask | SubstructureNotifyMask

// misc.
#define Length(s)   (sizeof(s) / sizeof(*s))
#define ButtonMasks (ButtonPressMask | ButtonReleaseMask)
#define Max(x, y)   (x) > (y) ? (x) : (y)
#define Min(x, y)   (x) < (y) ? (x) : (y)

// flag ops.
#define Set(state, mask)    state |= (mask)
#define UnSet(state, mask)  state &= ~(mask)
#define Toggle(state, mask) state ^= (mask)
#define IsSet(state, mask)  (state & (mask))

// logging.
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define die(...)                                                               \
  {                                                                            \
    eprintf(__VA_ARGS__);                                                      \
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

typedef struct {
  int x, y;
  uint32_t w, h;
} Geometry;

typedef struct {
  char *symbol;
  void (*arrange)();
} Layout;

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
  AtomNull
};

// These are mainly the values that don't (shouldn't) change throughout the
// application lifetime.
typedef struct {
  bool running : 1;
  Display *dpy;
  Window root;
  Cursor cursors[CurNull];
  Atom atoms[AtomNull];
  FILE *pipefile;
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
