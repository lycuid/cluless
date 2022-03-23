#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "include/base.h"
#include "include/bindings.h"
#include "include/layouts/tall.h"
#include <X11/Xutil.h>

#define Mod             Mod4Mask
#define Underlined(s)   "<Box:Bottom=#089CAC:1>" s "</Box>"
#define Clickable(k, s) "<BtnL=xdotool key " k ">" s "</BtnL>"
// clang-format off
#define WSKeys(key, ws)                                                        \
  {Mod,             key, select_ws,         {.i = ws}},                        \
  {Mod | ShiftMask, key, move_client_to_ws, {.i = ws}}                         \

static const uint32_t window_gappx    = 5;
static const uint32_t screen_gappx    = 5;
static const uint32_t borderpx        = 3;
static const uint32_t border_active   = 0x089cac;
static const uint32_t border_inactive = 0x252525;
static const char *const pipe_cmd[]   = {"xdbar", NULL}; // statucbar.

static const char *const workspaces[] = {Clickable("super+1", " 1 "),
                                         Clickable("super+2", " 2 "),
                                         Clickable("super+3", " 3 "),
                                         Clickable("super+4", " 4 "),
                                         Clickable("super+5", " 5 ")};

static const Layout layouts[] = {{"[]=", tall}, {"<><", NULL}};

static const char *const LogFormat[FmtOptionsCount] = {
    [FmtWsCurrent]      = Underlined("%s"),
    [FmtWsHidden]       = NULL,
    [FmtWsHiddenEmpty]  = "<Fg=#353535>%s</Fg>",
    [FmtWsSeperator]    = " ",
    [FmtLayout]         = Underlined(Clickable("super+space", " %s ")),
    [FmtSeperator]      = " <Box:Left=#303030:2> </Box>",
    [FmtWindowTitle]    = "%s"};

static const WindowRule window_rules[] = {
    {"LibreWolf",     move_client_to_ws,  {.i = 2}},
    {"Brave-browser", move_client_to_ws,  {.i = 2}},
    {"mpv",           float_client,       {0}},
    {"vlc",           float_client,       {0}}};

static const Binding keys[] = {
    {Mod,             XK_space,   cycle_layout,       {0}},
    {Mod,             XK_g,       toggle_gap,         {0}},
    {Mod,             XK_b,       toggle_border,      {0}},
    {Mod | ShiftMask, XK_space,   reset_layout,       {0}},
    {Mod | ShiftMask, XK_q,       quit,               {0}},
    {Mod | ShiftMask, XK_Return,  swap_master,        {0}},
    {Mod | ShiftMask, XK_c,       kill_client,        {0}},
    {Mod | ShiftMask, XK_j,       shift_client,       {.i = 1}},
    {Mod | ShiftMask, XK_k,       shift_client,       {.i = -1}},
    {Mod,             XK_j,       shift_focus,        {.i = 1}},
    {Mod,             XK_k,       shift_focus,        {.i = -1}},
    {Mod,             XK_t,       tile_client,        {0}},
    WSKeys(           XK_1,                           0),
    WSKeys(           XK_2,                           1),
    WSKeys(           XK_3,                           2),
    WSKeys(           XK_4,                           3),
    WSKeys(           XK_5,                           4)};

static const Binding buttons[] = {
    {ControlMask,     Button3,    focus_client,       {0}},
    {Mod,             Button1,    move_resize,        {.i = Move}},
    {Mod,             Button3,    move_resize,        {.i = Resize}}};

#undef WSKeys
#undef Clickable
#undef Underlined
#undef Mod
#endif
