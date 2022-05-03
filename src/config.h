// vim:fdm=marker:fmr={{{,}}}
#ifndef __CONFIG_H__
#define __CONFIG_H__

// Includes. {{{
#include "preprocs.h"
#include <include/bindings.h>
#include <include/core.h>
#include <include/ewmh/docks.h>
#include <include/layout.h>
#include <include/layout/tall.h>
#include <include/scratchpad.h>
#include <include/window_rule.h>
// }}}

static const uint32_t window_gappx    = 5;
static const uint32_t screen_gappx    = 5;
static const uint32_t borderpx        = 3;
static const uint32_t border_active   = 0x089cac;
static const uint32_t border_inactive = 0x252525;

// statusbar command e.g {"command", "arg1", "arg2", ..., NULL}.
static const char *const statusbar[] = {"xdbar", NULL};

static const char *const workspaces[] = {
    Clickable("super+1", " 1 "), Clickable("super+2", " 2 "),
    Clickable("super+3", " 3 "), Clickable("super+4", " 4 "),
    Clickable("super+5", " 5 ")};

static const Layout layouts[] = {{"[]=", tall}, {"<><", NULL}};

static const char *const LogFormat[FmtOptionsCount] = {
    [FmtWsCurrent]     = Underlined("%s"),
    [FmtWsHidden]      = NULL,
    [FmtWsHiddenEmpty] = "<Fg=#353535>%s</Fg>",
    [FmtWsSeperator]   = " ",
    [FmtLayout]        = Underlined(Clickable("super+space", " %s ")),
    [FmtSeperator]     = " <Box:Left=#303030:2> </Box>",
    [FmtWindowTitle]   = "%s"};

// clang-format off
static const WindowRule window_rules[] = {
    {ResClass, "LibreWolf",     move_client_to_ws,  {.i = 2}},
    {ResClass, "Brave-browser", move_client_to_ws,  {.i = 2}},
    {ResClass, "mpv",           float_client,       {0}},
    {ResClass, "vlc",           float_client,       {0}},
    // every scratchpad must have a unique 'char' id, which is used as reference
    // for toggling.
    {ResTitle, ScratchTerm,     sch_fromclient,     {.i = 't'}},
    {ResTitle, ScratchFM,       sch_fromclient,     {.i = 'f'}},
    {ResTitle, ScratchNM,       sch_fromclient,     {.i = 'n'}}};

static const Binding keys[] = {
    {Mod,               XK_Return,  spawn,              {.cmd = CMD(Term)}},
    {Mod,               XK_space,   cycle_layout,       {0}},
    {Mod,               XK_d,       dock_toggle,        {0}},
    {Mod,               XK_g,       toggle_gap,         {0}},
    {Mod,               XK_b,       toggle_border,      {0}},
    {Mod | ShiftMask,   XK_space,   reset_layout,       {0}},
    {Mod | ShiftMask,   XK_q,       quit,               {0}},
    {Mod | ShiftMask,   XK_Return,  swap_master,        {0}},
    {Mod | ShiftMask,   XK_c,       kill_client,        {0}},
    {Mod | ShiftMask,   XK_j,       shift_client,       {.i = 1}},
    {Mod | ShiftMask,   XK_k,       shift_client,       {.i = -1}},
    {Mod,               XK_j,       shift_focus,        {.i = 1}},
    {Mod,               XK_k,       shift_focus,        {.i = -1}},
    {Mod,               XK_t,       tile_client,        {0}},
    // scratchpad.
    SCHKeys(            XK_Return,  SCHToggle('t', "-t", ScratchTerm)),
    SCHKeys(            XK_f,       SCHToggle('f', "-t", ScratchFM, "-e", "lf")),
    SCHKeys(            XK_n,       SCHToggle('n', "-t", ScratchNM, "-e", "nmtui")),
    // workspace.
    WSKeys(             XK_1,                           0),
    WSKeys(             XK_2,                           1),
    WSKeys(             XK_3,                           2),
    WSKeys(             XK_4,                           3),
    WSKeys(             XK_5,                           4)};

static const Binding buttons[] = {
    {ControlMask,     Button3,    focus_client,       {0}},
    {Mod,             Button1,    move_resize,        {.i = Move}},
    {Mod,             Button3,    move_resize,        {.i = Resize}}};

#endif
