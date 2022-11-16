#ifndef __CORE__LOGGER_H__
#define __CORE__LOGGER_H__

#include <cluless/core/monitor.h>

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

void log_statuslog(Monitor *);

#endif
