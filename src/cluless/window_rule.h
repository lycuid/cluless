#ifndef __WINDOW_RULE_H__
#define __WINDOW_RULE_H__

#include <cluless/bindings.h>
#include <cluless/core/monitor.h>

// WM_CLASS(STRING) = ResInstance, ResClass
// WM_NAME(STRING)  = ResTitle
typedef enum { ResInstance, ResClass, ResTitle } ResourceType;
typedef struct {
  ResourceType res_type;
  char *value;
  CustomHandler func;
  Arg arg;
} WindowRule;

void window_rule_apply(Monitor *, Client *);

#endif
