#ifndef __MISC__WINDOW_RULE_H__
#define __MISC__WINDOW_RULE_H__

#include <cluless/bindings.h>
#include <cluless/core/monitor.h>

// WM_CLASS(STRING)         = ResInstance, ResClass
// WM_NAME(STRING)          = ResTitle
// WM_WINDOW_ROLE(STRING)   = ResWindowRole
typedef enum { ResInstance, ResClass, ResTitle, ResWindowRole } ResourceType;
typedef struct {
  ResourceType res_type;
  char *value;
  Action action;
  Arg arg;
} WindowRule;

void window_rule_apply(Monitor *, Client *);

#endif
