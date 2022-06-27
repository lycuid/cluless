#include "monitor.h"
#include <cluless/core/workspace.h>
#include <cluless/window_rule.h>
#include <config.h>
#include <stdlib.h>
#include <string.h>

void mon_init(Monitor *mon)
{
  mon->ctx = create_context();
  mon->wss = malloc(Length(workspaces) * sizeof(Workspace));
  for (size_t i = 0; i < Length(workspaces); ++i)
    ws_init(mon_workspaceat(mon, i), workspaces[i]);
  mon->selws   = &mon->wss[0];
  mon->screen  = get_screen_rect();
  mon->grabbed = (PointerGrab){NULL, 0, 0, 0, 0, 0, 0, 0, 0};
}

void mon_manage_client(Monitor *mon, Client *c)
{
  if (!c)
    return;
  ws_attachclient(mon->selws, c);
  // just set the client active, without focusing it, as the client isn't mapped
  // yet, and we never know, it might get moved to another workspace via some
  // startup hook. Focusing the client on 'MapNotify' event is much more safe.
  Set(c->state, ClActive);
  XWindowAttributes attrs;
  XGetWindowAttributes(mon->ctx->dpy, c->window, &attrs);
  XSelectInput(mon->ctx->dpy, c->window,
               attrs.your_event_mask | PropertyChangeMask);
  window_rule_apply(mon, c);
}

void mon_unmanage_client(Monitor *mon, Client *c)
{
  Workspace *ws = mon_get_client_ws(mon, c);
  if (!ws)
    return;
  Client *neighbour = cl_neighbour(c);
  // detaching the client before doing anything else, as the corresponding
  // window has already been destroyed (don't want any excitement).
  ws_detachclient(ws, c);
  if (ws == mon->selws && IsSet(c->state, ClActive))
    mon_focusclient(mon, neighbour);
  mon_applylayout(mon);
}

void mon_focusclient(Monitor *mon, Client *c)
{
  if (!c)
    goto LOG_AND_EXIT;
  Set(c->state, ClActive);
  LayoutManager *lm = &mon->selws->layout_manager;
  XSetWindowBorder(mon->ctx->dpy, c->window, lm->border_active);
  for (Client *p = c->prev; p; p = p->prev) {
    UnSet(p->state, ClActive);
    XSetWindowBorder(mon->ctx->dpy, p->window, lm->border_inactive);
  }
  for (Client *n = c->next; n; n = n->next) {
    UnSet(n->state, ClActive);
    XSetWindowBorder(mon->ctx->dpy, n->window, lm->border_inactive);
  }
  if (IsSet(c->state, ClFloating))
    XRaiseWindow(mon->ctx->dpy, c->window);
  XSetInputFocus(mon->ctx->dpy, c->window, RevertToParent, CurrentTime);
LOG_AND_EXIT:
  mon_statuslog(mon);
}

void mon_restack(Monitor *mon)
{
  Client *c;
  int i, fs, fl;
  for (c = mon->selws->cl_head, i = fs = fl = 0; c; c = c->next, i++) {
    if (IsSet(c->state, ClFloating | ClTransient))
      fl++;
    if (IsSet(c->state, ClFullscreen))
      fs++;
  }
  fs += fl;
  Window wid[i];
  for (c = mon->selws->cl_head, i = 0; c; c = c->next)
    wid[IsSet(c->state, ClFloating | ClTransient) ? i++
        : IsSet(c->state, ClFullscreen)           ? fl++
                                                  : fs++] = c->window;
  XRestackWindows(mon->ctx->dpy, wid, fs);
}

void mon_applylayout(Monitor *mon)
{
  const Layout *layout = lm_getlayout(&mon->selws->layout_manager);
  if (layout->apply)
    layout->apply(mon);
  mon_statuslog(mon);
}

Workspace *mon_get_client_ws(Monitor *mon, Client *c)
{
  if (c) {
    for (uint32_t i = 0; i < Length(workspaces); ++i) {
      Workspace *ws = mon_workspaceat(mon, i);
      if (ws_getclient(ws, c->window))
        return ws;
    }
  }
  return NULL;
}

// @FIXME: This function is an absolute mess.
void mon_statuslog(Monitor *mon)
{
  if (!mon->ctx->statuslogger)
    return;
#define StatusLog(...) fprintf(mon->ctx->statuslogger, __VA_ARGS__)

  // workspaces.
  {
    int size = 1024;
    char tmp[size];
#define FormatWSString(fmt, string)                                            \
  {                                                                            \
    if (fmt) {                                                                 \
      memcpy(tmp, string, size);                                               \
      sprintf(string, fmt, tmp);                                               \
    }                                                                          \
  }
    Workspace *ws;
    char string[size];
    memset(string, 0, size);
    for (size_t i = 0; i < Length(workspaces); ++i) {
      if (i && LogFormat[FmtWsSeperator])
        StatusLog("%s", LogFormat[FmtWsSeperator]);
      ws = mon_workspaceat(mon, i);
      sprintf(string, "%s", ws->id);
      if (ws == mon->selws) {
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
  }

  // layout.
  {
    const Layout *layout = lm_getlayout(&mon->selws->layout_manager);
    StatusLog(LogFormat[FmtLayout], layout->symbol);
    if (LogFormat[FmtSeperator])
      StatusLog("%s", LogFormat[FmtSeperator]);
  }

  // window title.
  {
    Client *active = ws_find(mon->selws, ClActive);
    if (active) {
      XTextProperty wm_name;
      if (get_window_title(active->window, &wm_name) && wm_name.nitems) {
        size_t trim = 30;
        char title[trim + 1];
        memset(title, '.', sizeof(title));
        memcpy(title, wm_name.value,
               wm_name.nitems >= trim ? trim - 3 : wm_name.nitems);
        title[Min(wm_name.nitems, trim)] = 0;
        StatusLog(" %s ", title);
      }
    }
  }

  StatusLog("\n");
#undef StatusLog
  fflush(mon->ctx->statuslogger);
}
