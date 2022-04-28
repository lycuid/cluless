#include "layout.h"
#include "base.h"

void lm_decorate_client(LayoutManager *lm, Client *c)
{
  Context *ctx = request_context();
  XSetWindowBorderWidth(ctx->dpy, c->window, lm->borderpx);
}
