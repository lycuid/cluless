// {{{ Includes.
#include <X11/Xutil.h>
#include <cluless/bindings.h>
#include <cluless/core.h>
#include <cluless/ewmh/docks.h>
#include <cluless/layout.h>
#include <cluless/layout/tall.h>
#include <cluless/scratchpad.h>
#include <cluless/window_rule.h>
// clang-format off
// }}}

// {{{ Defines.
// Constants.
#define Mod             Mod4Mask
#define Term            "st"
#define ScratchTerm     "scratchpad-term"
#define ScratchFM       "scratchpad-fm"
#define ScratchNM       "scratchpad-nmtui"
#define ScratchTermCmd  Term, "-g", "125x30+100+50", "-t", ScratchTerm
#define ScratchFMCmd    Term, "-g", "125x30+100+50", "-t", ScratchFM, "-e", "lf"
#define ScratchNMCmd    Term, "-g", "125x30+100+50", "-t", ScratchNM, "-e", "nmtui"

// Misc
#define UNDERLINED(s)   "<Box:Bottom=#089CAC:1>" s "</Box>"
#define CLICKABLE(k, s) "<BtnL=xdotool key " k ">" s "</BtnL>"
#define CMD(...)        ((const char *[]){__VA_ARGS__, NULL})

// KeyBindings.
#define WS_KEYS(key, arg)                                                      \
  {Mod,             key, select_ws,         arg},                              \
  {Mod | ShiftMask, key, move_client_to_ws, arg}
// }}}

static const uint32_t window_gappx    = 5;
static const uint32_t screen_gappx    = 5;
static const uint32_t borderpx        = 3;
static const uint32_t border_active   = 0x089cac;
static const uint32_t border_inactive = 0x252525;
static const uint32_t trim_title      = 30;

// statusbar command e.g {"command", "arg1", "arg2", ..., NULL}.
static const char *const statusbar[] = {"xdbar", NULL};

static const char *const workspaces[] = {
    CLICKABLE("super+1", " 1 "), CLICKABLE("super+2", " 2 "),
    CLICKABLE("super+3", " 3 "), CLICKABLE("super+4", " 4 "),
    CLICKABLE("super+5", " 5 ")};

static const Layout layouts[] = {{"[]=", tall}, {"<><", NULL}};

static const char *const LogFormat[FmtOptionsCount] = {
    [FmtWsCurrent]     = UNDERLINED("%s"),
    [FmtWsHidden]      = NULL,
    [FmtWsHiddenEmpty] = "<Fg=#353535>%s</Fg>",
    [FmtWsSeperator]   = " ",
    [FmtLayout]        = UNDERLINED(CLICKABLE("super+space", " %s ")),
    [FmtSeperator]     = " <Box:Left=#303030:2> </Box>",
    [FmtWindowTitle]   = "%s"};

static const WindowRule window_rules[] = {
    {ResClass, "librewolf",     move_client_to_ws,  {.i = 2}},
    {ResClass, "Brave-browser", move_client_to_ws,  {.i = 2}},
    {ResClass, "mpv",           float_client,       {0}},
    {ResClass, "vlc",           float_client,       {0}},
    // every scratchpad must have a unique 'char' id, which is used as reference
    // for toggling.
    {ResTitle, ScratchTerm,     sch_fromclient,     {.i = 't'}},
    {ResTitle, ScratchFM,       sch_fromclient,     {.i = 'f'}},
    {ResTitle, ScratchNM,       sch_fromclient,     {.i = 'n'}}};

static const Binding keys[] = {
    {Mod,                 XK_Return,  spawn,              {.cmd = CMD(Term)}},
    {Mod,                 XK_space,   cycle_layout,       {0}},
    {Mod,                 XK_d,       dock_toggle,        {0}},
    {Mod,                 XK_g,       toggle_gap,         {0}},
    {Mod,                 XK_b,       toggle_border,      {0}},
    {Mod | ShiftMask,     XK_space,   reset_layout,       {0}},
    {Mod | ShiftMask,     XK_q,       quit,               {0}},
    {Mod | ShiftMask,     XK_Return,  swap_master,        {0}},
    {Mod | ShiftMask,     XK_c,       kill_client,        {0}},
    {Mod | ShiftMask,     XK_j,       shift_client,       {.i = +1}},
    {Mod | ShiftMask,     XK_k,       shift_client,       {.i = -1}},
    {Mod,                 XK_j,       shift_focus,        {.i = +1}},
    {Mod,                 XK_k,       shift_focus,        {.i = -1}},
    {Mod,                 XK_t,       tile_client,        {0}},
    // scratchpad ('.cmd' value should be {sch_id, cmd, arg1, arg2, ...}).
    {Mod | ControlMask,   XK_Return,  sch_toggle,         {.cmd = CMD("t", ScratchTermCmd)}},
    {Mod | ControlMask,   XK_f,       sch_toggle,         {.cmd = CMD("f", ScratchFMCmd)}},
    {Mod | ControlMask,   XK_n,       sch_toggle,         {.cmd = CMD("n", ScratchNMCmd)}},
    // workspace.
    WS_KEYS(              XK_1,                           {.i = 0}),
    WS_KEYS(              XK_2,                           {.i = 1}),
    WS_KEYS(              XK_3,                           {.i = 2}),
    WS_KEYS(              XK_4,                           {.i = 3}),
    WS_KEYS(              XK_5,                           {.i = 4})};

static const Binding buttons[] = {
    {Mod,             Button1,    move_client,        {0}},
    {Mod,             Button3,    resize_client,      {0}},
    {ControlMask,     Button3,    focus_client,       {0}}};

// {{{ Undefs.
#undef Mod
#undef Term
#undef ScratchTerm
#undef ScratchFM
#undef ScratchNM
#undef ScratchTermCmd
#undef ScratchFMCmd
#undef ScratchNMCmd
#undef UNDERLINED
#undef CLICKABLE
#undef CMD
#undef WS_KEYS
// }}}

// vim:fdm=marker
