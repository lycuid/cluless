#include "window_rule.h"
#include <config.h>
#include <stdbool.h>
#include <string.h>

static inline bool TryApplyWindowRule(Monitor *mon, const WindowRule *rule,
                                      const char *val)
{
  bool value_matches = strcmp(val, rule->value) == 0;
  if (value_matches) {
    rule->func(mon, &rule->arg);
  }
  return value_matches;
}

void window_rule_apply(Monitor *mon, Client *c)
{
  XClassHint class;
  XTextProperty wm_name;
  for (size_t i = 0; i < Length(window_rules); ++i) {
    const WindowRule *rule = &window_rules[i];
    if (rule->res_type == ResTitle) {
      if (get_window_title(c->window, &wm_name) && wm_name.nitems)
        if (TryApplyWindowRule(mon, rule, (char *)wm_name.value))
          break;
    }
    if (XGetClassHint(mon->ctx->dpy, c->window, &class)) {
      const char *res_type =
          rule->res_type == ResClass ? class.res_class : class.res_name;
      if (TryApplyWindowRule(mon, rule, res_type))
        break;
    }
  }
}
