#include "window_rule.h"
#include <config.h>
#include <stdbool.h>
#include <string.h>

static inline bool apply_window_rule(Monitor *mon, const WindowRule *rule,
                                     const char *val)
{
  bool value_matches = strcmp(val, rule->value) == 0;
  if (value_matches)
    rule->action(mon, &rule->arg);
  return value_matches;
}

void window_rule_apply(Monitor *mon, Client *c)
{
  XClassHint class;
  XTextProperty property;
  FOREACH(const WindowRule *rule, window_rules)
  {
    switch (rule->res_type) {
    case ResTitle: {
      if (core->get_window_title(c->window, &property) && property.nitems)
        if (apply_window_rule(mon, rule, (char *)property.value))
          goto DONE;
    } break;
    case ResClass: // fallthrough.
    case ResInstance: {
      if (XGetClassHint(core->dpy, c->window, &class))
        if (apply_window_rule(mon, rule,
                              rule->res_type == ResClass ? class.res_class
                                                         : class.res_name))
          goto DONE;
    } break;
    case ResWindowRole: {
      if (XGetTextProperty(core->dpy, c->window, &property,
                           core->wmatoms[WM_WINDOW_ROLE])) {
        if (apply_window_rule(mon, rule, (char *)property.value))
          goto DONE;
      }
    } break;
    }
  }
DONE:
  return;
}
