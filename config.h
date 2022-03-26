#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "include/base.h"
#include "include/bindings.h"
#include "include/layouts/tall.h"
#include "include/scratchpad.h"
#include <X11/Xutil.h>
#include <stdarg.h>

#define Mod             Mod4Mask
#define Underlined(s)   "<Box:Bottom=#089CAC:1>" s "</Box>"
#define Clickable(k, s) "<BtnL=xdotool key " k ">" s "</BtnL>"
#define Term            "st"
#define CMD(...)        ((const char *[]){__VA_ARGS__, NULL})

static const uint32_t window_gappx    = 5;
static const uint32_t screen_gappx    = 5;
static const uint32_t borderpx        = 3;
static const uint32_t border_active   = 0x089cac;
static const uint32_t border_inactive = 0x252525;
static const char *const pipe_cmd[]   = {"xdbar", NULL}; // statusbar.

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

#define ScratchTerm "scratchpad-term"
#define ScratchFM   "scratchpad-fm"
#define ScratchNM   "scratchpad-nmtui"

static const char *const scratchpads[] = {
    [0] = ScratchTerm, [1] = ScratchFM, [2] = ScratchNM};
// clang-format off

static const WindowRule window_rules[] = {
    {ResClass, "LibreWolf",     move_client_to_ws,  {.i = 2}},
    {ResClass, "Brave-browser", move_client_to_ws,  {.i = 2}},
    {ResClass, "mpv",           float_client,       {0}},
    {ResClass, "vlc",           float_client,       {0}},
    {ResTitle, ScratchTerm,     sch_create,         {.i = 0}},
    {ResTitle, ScratchFM,       sch_create,         {.i = 1}},
    {ResTitle, ScratchNM,       sch_create,         {.i = 2}}};

#define WSKeys(key, ws)                                                        \
  {Mod,               key, select_ws,         {.i = ws}},                      \
  {Mod | ShiftMask,   key, move_client_to_ws, {.i = ws}}

// scratchpad '.cmd' value should be {sch_name, cmd, arg1, arg2, ...}.
#define SCHKeys(key, c) {Mod | ControlMask, key, sch_toggle, {.cmd = c}}
#define SCHWindow       Term, "-g", "112x30+136+76"

static const Binding keys[] = {
    {Mod | ShiftMask,   XK_Return,  spawn,              {.cmd = CMD(Term)}},
    {Mod,               XK_space,   cycle_layout,       {0}},
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
    SCHKeys(            XK_Return,  CMD(ScratchTerm,  SCHWindow, "-t", ScratchTerm)),
    SCHKeys(            XK_f,       CMD(ScratchFM,    SCHWindow, "-t", ScratchFM, "-e", "lf")),
    SCHKeys(            XK_n,       CMD(ScratchNM,    SCHWindow, "-t", ScratchNM, "-e", "nmtui")),
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

#undef SCHWindow
#undef SCHKeys
#undef WSKeys
#undef ScratchTerm
#undef ScratchFM
#undef ScratchNM
#undef CMD
#undef Term
#undef Clickable
#undef Underlined
#undef Mod
#endif
