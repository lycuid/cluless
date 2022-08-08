#include "monitor.h"
#include <cluless/core/workspace.h>
#include <cluless/window_rule.h>
#include <config.h>
#include <stdlib.h>
#include <string.h>

void focusclient(Client *);
void restack();
void applylayout();
Workspace *get_client_ws(Client *);
void statuslog();

static Monitor mon = {
    .focusclient   = focusclient,
    .restack       = restack,
    .applylayout   = applylayout,
    .get_client_ws = get_client_ws,
    .statuslog     = statuslog,
};

Monitor *mon_init()
{
  mon.running = true;
  mon.wss     = malloc(LENGTH(workspaces) * sizeof(Workspace));
  ITER(workspaces) ws_init(&mon.wss[it], workspaces[it]);
  mon.selws  = &mon.wss[0];
  mon.screen = core->get_screen_rect();
  memset(&mon.grabbed, 0, sizeof(PointerGrab));
  return &mon;
}

void mon_manage_client(Monitor *mon, Client *c)
{
  if (!c)
    return;
  ws_attachclient(mon->selws, c);
  // just set the client active, without focusing it, as the client isn't mapped
  // yet, and we never know, it might get moved to another workspace via some
  // startup hook. Focusing the client on 'MapNotify' event is much more safe.
  SET(c->state, ClActive);
  XWindowAttributes attrs;
  XGetWindowAttributes(core->dpy, c->window, &attrs);
  XSelectInput(core->dpy, c->window,
               attrs.your_event_mask | PropertyChangeMask);
  window_rule_apply(mon, c);
}

void mon_unmanage_client(Monitor *mon, Client *c)
{
  Workspace *ws = mon->get_client_ws(c);
  if (!ws)
    return;
  Client *neighbour = cl_neighbour(c);
  // detaching the client before doing anything else, as the corresponding
  // window has already been destroyed (don't want any excitement).
  ws_detachclient(ws, c);
  if (ws == mon->selws && IS_SET(c->state, ClActive))
    mon->focusclient(neighbour);
  mon->applylayout();
}

void focusclient(Client *c)
{
  LayoutManager *lm = &mon.selws->layout_manager;
  if (!c)
    goto LOG_AND_EXIT;
  SET(c->state, ClActive);
  XSetWindowBorder(core->dpy, c->window, lm->border_active);
  for (Client *p = c->prev; p; p = p->prev) {
    UNSET(p->state, ClActive);
    XSetWindowBorder(core->dpy, p->window, lm->border_inactive);
  }
  for (Client *n = c->next; n; n = n->next) {
    UNSET(n->state, ClActive);
    XSetWindowBorder(core->dpy, n->window, lm->border_inactive);
  }
  if (IS_SET(c->state, ClFloating))
    XRaiseWindow(core->dpy, c->window);
  XSetInputFocus(core->dpy, c->window, RevertToParent, CurrentTime);
LOG_AND_EXIT:
  mon.statuslog();
}

void restack()
{
  Client *c = mon.selws->cl_head, *active = NULL;
  if (!c)
    return;

  int floating = 0, fullscreen = 0, i = 0;
  for (; c; c = c->next, i++) {
    if (IS_SET(c->state, ClFloating | ClTransient))
      floating++;
    if (IS_SET(c->state, ClFullscreen))
      fullscreen++;
    if (IS_SET(c->state, ClActive))
      active = c;
  }
  Window *stack = malloc(i * sizeof(Window));
  fullscreen += floating, i = 0;
#define AddToStack(c)                                                          \
  stack[IS_SET(c->state, ClFloating | ClTransient) ? i++                       \
        : IS_SET(c->state, ClFullscreen)           ? floating++                \
                                                   : fullscreen++] = c->window;
  if (active)
    AddToStack(active);
  for (c = mon.selws->cl_head; c; c = c->next)
    if (c != active)
      AddToStack(c);
#undef AddToStack
  XRestackWindows(core->dpy, stack, fullscreen);
  free(stack);
}

void applylayout()
{
  const Layout *layout = lm_getlayout(&mon.selws->layout_manager);
  if (layout->apply)
    layout->apply(&mon);
  mon.restack();
  mon.statuslog();
}

Workspace *get_client_ws(Client *c)
{
  if (c) {
    ITER(workspaces)
    {
      Workspace *ws = &mon.wss[it];
      if (ws_getclient(ws, c->window))
        return ws;
    }
  }
  return NULL;
}

// @FIXME: This function is an absolute mess.
void statuslog()
{
  if (!core->statuslogger)
    return;
#define StatusLog(...) fprintf(core->statuslogger, __VA_ARGS__)

  // workspaces.
  {
#define SIZE 1024
    char tmp[SIZE];
#define FormatWSString(fmt, string)                                            \
  {                                                                            \
    if (fmt) {                                                                 \
      memcpy(tmp, string, SIZE);                                               \
      sprintf(string, fmt, tmp);                                               \
    }                                                                          \
  }
    Workspace *ws;
    char string[SIZE];
    memset(string, 0, SIZE);
    ITER(workspaces)
    {
      ws = &mon.wss[it];
      if (it && LogFormat[FmtWsSeperator])
        StatusLog("%s", LogFormat[FmtWsSeperator]);
      sprintf(string, "%s", ws->id);
      if (ws == mon.selws) {
        FormatWSString(LogFormat[FmtWsCurrent], string);
      } else {
        FormatWSString(ws->cl_head ? LogFormat[FmtWsHidden]
                                   : LogFormat[FmtWsHiddenEmpty],
                       string);
      }
      StatusLog("%s", string);
    }
    if (LogFormat[FmtSeperator])
      StatusLog("%s", LogFormat[FmtSeperator]);
#undef FormatWSString
#undef SIZE
  }

  // layout.
  {
    const Layout *layout = lm_getlayout(&mon.selws->layout_manager);
    StatusLog(LogFormat[FmtLayout], layout->symbol);
    if (LogFormat[FmtSeperator])
      StatusLog("%s", LogFormat[FmtSeperator]);
  }

  // window title.
  {
    Client *active = ws_find(mon.selws, ClActive);
    if (active) {
      XTextProperty wm_name;
      if (core->get_window_title(active->window, &wm_name) && wm_name.nitems) {
        char title[TrimTitle + 1];
        memset(title, '.', sizeof(title));
        memcpy(title, wm_name.value,
               wm_name.nitems >= TrimTitle ? TrimTitle - 3 : wm_name.nitems);
        title[MIN(wm_name.nitems, TrimTitle)] = 0;
        StatusLog(LogFormat[FmtWindowTitle], title);
      }
    }
  }

  StatusLog("\n");
#undef StatusLog
  fflush(core->statuslogger);
}
