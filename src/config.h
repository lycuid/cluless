// {{{ Includes.
#include <X11/Xutil.h>
#include <cluless/bindings.h>
#include <cluless/core/logging.h>
#include <cluless/ewmh/docks.h>
#include <cluless/layout.h>
#include <cluless/layout/full.h>
#include <cluless/layout/tall.h>
#include <cluless/misc/companion.h>
#include <cluless/misc/scratchpad.h>
#include <cluless/misc/window_rule.h>
// clang-format off
// }}}

// {{{ Macros used only in this file (gets 'undef'-ed later).
#define Mod             Mod4Mask
#define Term            "alacritty"
#define ScratchTerm     "scratchpad-term"
#define ScratchFM       "scratchpad-fm"
#define ScratchNM       "scratchpad-nmtui"
#define UNDERLINED(s)   "<Box:Bottom=#089CAC:1>" s "</Box>"
#define CLICKABLE(s, k) "<BtnL=xdotool key " k ">" s "</BtnL>"
#define CMD(...)        ((const char *[]){__VA_ARGS__, NULL})
#define SchGeometry                                                             \
  "-o", "window.dimensions.columns=125",                                        \
  "-o", "window.dimensions.lines=30",                                           \
  "-o", "window.position.x=100",                                                \
  "-o", "window.position.y=50"
// }}}

#define WindowGapPX         5
#define ScreenGapPX         5
#define BorderPX            3
#define BorderActive        0x089cac
#define BorderInactive      0x252525
#define CompanionActive     0xff793f
#define CompanionInactive   0xaaa69d
#define TrimTitle           30
#define ButtonForFocus      Button1

static const char *const workspaces[] = {
    CLICKABLE(" 1 ", "super+1"), CLICKABLE(" 2 ", "super+2"),
    CLICKABLE(" 3 ", "super+3"), CLICKABLE(" 4 ", "super+4"),
    CLICKABLE(" 5 ", "super+5"),
};

static const Layout layouts[] = {{"[]=", tall}, {"[M]", full}, {"<><", NULL}};

static const char *const LogFormat[FmtOptionsCount] = {
    [FmtWsCurrent]     = UNDERLINED("%s"),
    [FmtWsHidden]      = NULL,
    [FmtWsHiddenEmpty] = "<Fg=#353535>%s</Fg>",
    [FmtWsSeperator]   = " ",
    [FmtLayout]        = UNDERLINED(CLICKABLE(" %s ", "super+space")),
    [FmtSeperator]     = " <Box:Left=#303030:2> </Box>",
    [FmtWindowTitle]   = "%s",
};

static const WindowRule window_rules[] = {
    {ResClass, "Brave-browser", move_client_to_ws,  {.i = 2}},
    {ResClass, "mpv",           float_client,       {0}},
    // every scratchpad must have a unique 'ascii' char id, which is used as
    // reference for toggling.
    {ResTitle, ScratchTerm,     sch_fromclient,     {.i = 't'}},
    {ResTitle, ScratchFM,       sch_fromclient,     {.i = 'f'}},
    {ResTitle, ScratchNM,       sch_fromclient,     {.i = 'n'}},
};

static const Binding keys[] = {
    {Mod,                 XK_Return,  spawn,              {.cmd = CMD(Term)}},
    {Mod,                 XK_space,   cycle_layout,       {0}},
    {Mod,                 XK_d,       dock_toggle,        {0}},
    {Mod,                 XK_g,       toggle_gap,         {0}},
    {Mod,                 XK_b,       toggle_border,      {0}},
    {Mod,                 XK_c,       companion_toggle,   {0}},
    {Mod | ShiftMask,     XK_space,   reset_layout,       {0}},
    {Mod | ShiftMask,     XK_q,       quit,               {0}},
    {Mod | ShiftMask,     XK_Return,  swap_master,        {0}},
    {Mod | ShiftMask,     XK_c,       kill_client,        {0}},
    {Mod | ShiftMask,     XK_j,       shift_client,       {.i = +1}},
    {Mod | ShiftMask,     XK_k,       shift_client,       {.i = -1}},
    {Mod,                 XK_Down,    move_client_y,      {.i = +15}},
    {Mod,                 XK_Up,      move_client_y,      {.i = -15}},
    {Mod,                 XK_Right,   move_client_x,      {.i = +15}},
    {Mod,                 XK_Left,    move_client_x,      {.i = -15}},
    {Mod | ShiftMask,     XK_Down,    resize_client_y,    {.i = +15}},
    {Mod | ShiftMask,     XK_Up,      resize_client_y,    {.i = -15}},
    {Mod | ShiftMask,     XK_Right,   resize_client_x,    {.i = +15}},
    {Mod | ShiftMask,     XK_Left,    resize_client_x,    {.i = -15}},
    {Mod,                 XK_j,       shift_focus,        {.i = +1}},
    {Mod,                 XK_k,       shift_focus,        {.i = -1}},
    {Mod,                 XK_t,       tile_client,        {0}},
    // scratchpad ('.cmd' value should be {sch_id, cmd, arg1, arg2, ...}).
#define SchWindow Term, SchGeometry, "-t"
#define SchWindow Term, SchGeometry, "-t"
    {Mod | ControlMask,   XK_Return,  sch_toggle,         {.cmd = CMD("t", SchWindow, ScratchTerm)}},
    {Mod | ControlMask,   XK_f,       sch_toggle,         {.cmd = CMD("f", SchWindow, ScratchFM, "-e", "vifm")}},
    {Mod | ControlMask,   XK_n,       sch_toggle,         {.cmd = CMD("n", SchWindow, ScratchNM, "-e", "nmtui")}},
#undef SchWindow
    // workspace.
#define WS_KEYS(key, arg)                                                      \
    {Mod,                 key,        select_ws,          arg},                \
    {Mod | ShiftMask,     key,        move_client_to_ws,  arg}
    WS_KEYS(              XK_1,                           {.i = 0}),
    WS_KEYS(              XK_2,                           {.i = 1}),
    WS_KEYS(              XK_3,                           {.i = 2}),
    WS_KEYS(              XK_4,                           {.i = 3}),
    WS_KEYS(              XK_5,                           {.i = 4}),
#undef WS_KEYS
};

static const Binding buttons[] = {
    {Mod,                 Button1,    mouse_move,         {0}},
    {Mod,                 Button3,    mouse_resize,       {0}},
};

// {{{ 'undef'-ing.
#undef Mod
#undef Term
#undef ScratchTerm
#undef ScratchFM
#undef ScratchNM
#undef UNDERLINED
#undef CLICKABLE
#undef CMD
// }}}

// vim:fdm=marker
