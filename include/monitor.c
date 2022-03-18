#include "monitor.h"
#include "workspace.h"
#include <X11/Xatom.h>
#include <config.h>
#include <stdlib.h>
#include <string.h>

void mon_init(Monitor *mon)
{
  mon->ctx = create_context();
  mon->wss = malloc(Length(workspaces) * sizeof(Workspace));
  for (size_t i = 0; i < Length(workspaces); ++i)
    ws_init(&mon->wss[i], workspaces[i]);
  mon->selws   = &mon->wss[0];
  mon->screen  = get_screen_rect();
  mon->grabbed = (PointerGrab){NULL, 0, 0, 0, 0, 0, 0, 0, 0};
}

void mon_addclient(Monitor *mon, Client *c)
{
  ws_attachclient(mon->selws, c);
  // just set the client active, without focusing it, as the client isn't mapped
  // yet, and we never know, it might get moved to another workspace via some
  // startup hook. Focusing the client on 'MapNotify' event is much more safe.
  mon_setactive(mon, c);
  XChangeProperty(mon->ctx->dpy, mon->ctx->root,
                  mon->ctx->netatoms[NetClientList], XA_WINDOW, 32,
                  PropModeAppend, (uint8_t *)&c->window, 1);
  XSelectInput(mon->ctx->dpy, c->window, PropertyChangeMask);
}

void mon_removeclient(Monitor *mon, Client *c)
{
  // first focus client's neighbour and **then** detach the client, as no
  // neighbours will be present after client gets detached.
  mon_focusclient(mon, cl_neighbour(c));
  ws_detachclient(mon->selws, c);
  mon_arrange(mon);
  free(c);
  // update client list.
  size_t n = 0;
  Window wids[64];
  for (size_t i = 0; i < Length(workspaces); ++i)
    for (Client *c = mon->wss[i].cl_head; c; c = c->next)
      wids[n++] = c->window;
  XChangeProperty(mon->ctx->dpy, mon->ctx->root,
                  mon->ctx->netatoms[NetClientList], XA_WINDOW, 32,
                  PropModeReplace, (uint8_t *)wids, n);
}

void mon_focusclient(Monitor *mon, Client *c)
{
  if (!c) {
    XDeleteProperty(mon->ctx->dpy, mon->ctx->root,
                    mon->ctx->netatoms[NetActiveWindow]);
    return;
  }
  mon_setactive(mon, c);
  if (IsSet(c->state, ClFloating))
    XRaiseWindow(mon->ctx->dpy, c->window);
  XSetInputFocus(mon->ctx->dpy, c->window, RevertToParent, CurrentTime);
  XChangeProperty(mon->ctx->dpy, mon->ctx->root,
                  mon->ctx->netatoms[NetActiveWindow], XA_WINDOW, 32,
                  PropModeReplace, (uint8_t *)&c->window, 1);
  mon_statuslog(mon);
}

void mon_setactive(Monitor *mon, Client *c)
{
  if (!c)
    return;
  Set(c->state, ClActive);
  XSetWindowBorder(mon->ctx->dpy, c->window, mon->selws->border_active);
  for (Client *p = c->prev; p; p = p->prev) {
    UnSet(p->state, ClActive);
    XSetWindowBorder(mon->ctx->dpy, p->window, mon->selws->border_inactive);
  }
  for (Client *n = c->next; n; n = n->next) {
    UnSet(n->state, ClActive);
    XSetWindowBorder(mon->ctx->dpy, n->window, mon->selws->border_inactive);
  }
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

void mon_arrange(Monitor *mon)
{
  const Layout *layout = ws_getlayout(mon->selws);
  if (layout->arrange)
    layout->arrange(mon);
  mon_statuslog(mon);
}

void mon_statuslog(Monitor *mon)
{
  // @FIXME: File might be closed, but the pointer will still exist.
  // need to make sure the pointer is nullified somehow, if the file is closed.
  if (!mon->ctx->pipefile)
    return;

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
    char *string = malloc(size);
    memset(string, 0, size);
    for (size_t i = 0; i < Length(workspaces); ++i) {
      if (i && LogFormat[FmtWsSeperator])
        fprintf(mon->ctx->pipefile, "%s", LogFormat[FmtWsSeperator]);
      ws = mon_workspaceat(mon, i);
      sprintf(string, "%s", workspaces[i]);
      if (ws == mon->selws) {
        FormatWSString(LogFormat[FmtWsCurrent], string);
      } else {
        FormatWSString(ws->cl_head ? LogFormat[FmtWsHidden]
                                   : LogFormat[FmtWsHiddenEmpty],
                       string);
      }
      fprintf(mon->ctx->pipefile, "%s", string);
    }
    free(string);
    if (LogFormat[FmtSeperator])
      fprintf(mon->ctx->pipefile, "%s", LogFormat[FmtSeperator]);
#undef format_string
  }

  // layout.
  {
    const Layout *layout = ws_getlayout(mon->selws);
    fprintf(mon->ctx->pipefile, LogFormat[FmtLayout], layout->symbol);
    if (LogFormat[FmtSeperator])
      fprintf(mon->ctx->pipefile, "%s", LogFormat[FmtSeperator]);
  }

  // window title.
  {
    Client *active = ws_find(mon->selws, ClActive);
    if (active) {
      XTextProperty wm_name;
      int found = XGetTextProperty(mon->ctx->dpy, active->window, &wm_name,
                                   mon->ctx->netatoms[NetWMName]) &&
                  wm_name.nitems;
      if (!found)
        found = XGetTextProperty(mon->ctx->dpy, active->window, &wm_name,
                                 mon->ctx->wmatoms[WMName]) &&
                wm_name.nitems;
      if (found) {
        size_t trim = 30;
        char title[trim + 1];
        memset(title, '.', sizeof(title));
        memcpy(title, wm_name.value,
               wm_name.nitems >= trim ? trim - 3 : wm_name.nitems);
        title[Min(wm_name.nitems, trim)] = 0;
        fprintf(mon->ctx->pipefile, " %s ", title);
      }
    }
  }

  fprintf(mon->ctx->pipefile, "\n");
  fflush(mon->ctx->pipefile);
}
