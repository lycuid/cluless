#include "logging.h"
#include <cluless/core.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTION_MAX_LEN (1 << 10) // individual section strings.
#define OUTPUT_MAX_LEN  (1 << 14) // final output string.

#define SAFE(fmt) fmt ? fmt : "%s"

static char workspace_section[SECTION_MAX_LEN];
static char layout_section[SECTION_MAX_LEN];
static char title_section[SECTION_MAX_LEN];

static char buffer1[OUTPUT_MAX_LEN] = "\0", buffer2[OUTPUT_MAX_LEN] = "\0";
static char *stale_output = buffer1, *new_output = buffer2;

#define SET_STATIC_LEN_VARIABLE(var, string)                                   \
    static int var = -1;                                                       \
    if (var == -1)                                                             \
        var = strlen(string);

static inline void create_workspace_section(void)
{
    Monitor *mon = core->mon;
    SET_STATIC_LEN_VARIABLE(ws_seperator_len, LogFormat[FmtWsSeperator]);
    Workspace *ws;
    char buffer[SECTION_MAX_LEN] = "\0";
    int j = 0, buflen = 0;
    memset(workspace_section, 0, SECTION_MAX_LEN);
    ITER(workspaces)
    {
        ws = mon_get_workspace_at(mon, it);
        buflen =
            sprintf(buffer,
                    ws == curr_ws(mon)
                        ? SAFE(LogFormat[FmtWsCurrent])
                        : (ws->cl_head ? SAFE(LogFormat[FmtWsHidden])
                                       : SAFE(LogFormat[FmtWsHiddenEmpty])),
                    ws->id);
        if (it && LogFormat[FmtWsSeperator])
            for (int i = 0; i < ws_seperator_len; ++i)
                workspace_section[j++] = LogFormat[FmtWsSeperator][i];
        for (int i = 0; i < buflen; ++i)
            workspace_section[j++] = buffer[i];
    }
    workspace_section[j] = 0;
}

static inline void create_layout_section(void)
{
    Monitor *mon         = core->mon;
    const Layout *layout = lm_getlayout(&curr_ws(mon)->layout_manager);
    memset(layout_section, 0, SECTION_MAX_LEN);
    sprintf(layout_section, SAFE(LogFormat[FmtLayout]), layout->symbol);
}

static inline void create_title_section(void)
{
    Monitor *mon   = core->mon;
    Client *active = ws_find(curr_ws(mon), ClActive);
    memset(title_section, 0, SECTION_MAX_LEN);
    if (active) {
        XTextProperty wm_name;
        if (core->get_window_title(active->window, &wm_name) &&
            wm_name.nitems) {
            char title[TrimTitle + 1];
            memset(title, '.', sizeof(title));
            memcpy(title, wm_name.value,
                   wm_name.nitems >= TrimTitle ? TrimTitle - 3
                                               : wm_name.nitems);
            title[MIN(wm_name.nitems, TrimTitle)] = 0;
            sprintf(title_section, SAFE(LogFormat[FmtWindowTitle]), title);
        }
    }
}

void log_statuslog(void)
{
    SET_STATIC_LEN_VARIABLE(seperator_len, LogFormat[FmtSeperator]);
    int j = 0;
    memset(new_output, 0, OUTPUT_MAX_LEN);

#define AppendSeperator()                                                      \
    for (int i = 0; i < seperator_len; ++i)                                    \
        new_output[j++] = LogFormat[FmtSeperator][i];
#define AppendSection(string)                                                  \
    for (int i = 0; string[i] != 0; ++i)                                       \
        new_output[j++] = string[i];

    create_workspace_section();
    AppendSection(workspace_section);
    AppendSeperator();

    create_layout_section();
    AppendSection(layout_section);
    AppendSeperator();

    create_title_section();
    AppendSection(title_section);

#undef AppendSection
#undef AppendSeperator

    if (!memcmp(stale_output, new_output, OUTPUT_MAX_LEN))
        return;
    fprintf(core->logger, "%s\n", new_output);
    { // swap.
        char *tmp  = new_output;
        new_output = stale_output, stale_output = tmp;
    }
    fflush(core->logger);
}
