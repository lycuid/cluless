// {{{ Includes.
#include <X11/Xutil.h>
#include <cluless/bindings.h>
#include <cluless/core.h>
#include <cluless/core/logging.h>
#include <cluless/ewmh/docks.h>
#include <cluless/layout.h>
#include <cluless/layout/full.h>
#include <cluless/layout/tall.h>
#include <cluless/misc/companion.h>
#include <cluless/misc/magnify.h>
#include <cluless/misc/scratchpad.h>
#include <cluless/misc/window_rule.h>
// clang-format off
// }}}

// {{{ Macros used only in this file (gets 'undef'-ed later).
#define Mod               Mod4Mask
#define UNDERLINED(s)     "<Box:Bottom=#089CAC:1>" s "</Box>"
#define CLICKABLE(s, k)   "<BtnL=xdotool key " k ">" s "</BtnL>"
#define CMD(...)          ((const char *[]){__VA_ARGS__, NULL})
#define NO_ARG            {0}
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
#define MagnifyW            1.2
#define MagnifyH            1.2

static const char *const workspaces[] = {
    CLICKABLE(" ○ ", "super+1"), CLICKABLE(" ○ ", "super+2"),
    CLICKABLE(" ○ ", "super+3"), CLICKABLE(" ○ ", "super+4"),
    CLICKABLE(" ○ ", "super+5"),
};

static const Layout layouts[] = {{"[]=", tall}, {"[M]", full}, {"<><", NULL}};

static const char *const LogFormat[FmtOptionsCount] = {
    [FmtWsCurrent]     = UNDERLINED(" ◉ "),
    [FmtWsHidden]      = NULL,
    [FmtWsHiddenEmpty] = "<Fg=#353535>%s</Fg>",
    [FmtWsSeperator]   = " ",
    [FmtLayout]        = UNDERLINED(CLICKABLE(" %s ", "super+space")),
    [FmtSeperator]     = "  <Fn=1><Fg=#373737></Fg></Fn>  ",
    [FmtWindowTitle]   = "%s",
};

static const WindowRule window_rules[] = {
    {ResWindowRole, "browser",          transfer_client_to,   {.i = 2}},
    {ResClass,      "mpv",              float_client,         NO_ARG},
    // every scratchpad must have a unique 'ascii' char id, which is used as
    // reference for toggling.
    {ResTitle,      "scratchpad-term",  sch_fromclient,     {.i = 't'}},
    {ResTitle,      "scratchpad-fm",    sch_fromclient,     {.i = 'f'}},
    {ResTitle,      "scratchpad-nmtui", sch_fromclient,     {.i = 'n'}},
};

static const Binding keys[] = {
    {Mod,                   XK_Return,  spawn,              {.cmd = CMD("alacritty")}},
    {Mod,                   XK_space,   cycle_layout,       NO_ARG},
    {Mod,                   XK_d,       dock_toggle,        NO_ARG},
    {Mod,                   XK_g,       toggle_gap,         NO_ARG},
    {Mod,                   XK_b,       toggle_border,      NO_ARG},
    {Mod,                   XK_m,       magnify_toggle,     NO_ARG},
    {Mod,                   XK_c,       companion_toggle,   NO_ARG},
    {Mod | ShiftMask,       XK_space,   reset_layout,       NO_ARG},
    {Mod | ShiftMask,       XK_q,       quit,               NO_ARG},
    {Mod | ShiftMask,       XK_Return,  swap_master,        NO_ARG},
    {Mod | ShiftMask,       XK_c,       kill_client,        NO_ARG},
    {Mod | ShiftMask,       XK_j,       shift_client,       {.i = +1}},
    {Mod | ShiftMask,       XK_k,       shift_client,       {.i = -1}},
    {Mod,                   XK_Down,    move_client_y,      {.i = +15}},
    {Mod,                   XK_Up,      move_client_y,      {.i = -15}},
    {Mod,                   XK_Right,   move_client_x,      {.i = +15}},
    {Mod,                   XK_Left,    move_client_x,      {.i = -15}},
    {Mod | ShiftMask,       XK_Down,    resize_client_y,    {.i = +15}},
    {Mod | ShiftMask,       XK_Up,      resize_client_y,    {.i = -15}},
    {Mod | ShiftMask,       XK_Right,   resize_client_x,    {.i = +15}},
    {Mod | ShiftMask,       XK_Left,    resize_client_x,    {.i = -15}},
    {Mod,                   XK_j,       shift_focus,        {.i = +1}},
    {Mod,                   XK_k,       shift_focus,        {.i = -1}},
    {Mod,                   XK_t,       tile_client,        NO_ARG},

    // scratchpad ('.cmd' value should be {sch_id, cmd, arg1, arg2, ...}).
    {Mod | ControlMask,     XK_Return,  sch_toggle,         {.cmd = CMD("t", "scratchpad-term")}},
    {Mod | ControlMask,     XK_f,       sch_toggle,         {.cmd = CMD("f", "scratchpad-fm")}},
    {Mod | ControlMask,     XK_n,       sch_toggle,         {.cmd = CMD("n", "scratchpad-nmtui")}},

    // workspace.
#define WS_KEYS(key, arg)                                                      \
    {Mod,                   key,        select_ws,          arg},              \
    {Mod | ShiftMask,       key,        transfer_client_to, arg}
    WS_KEYS(                XK_1,                           {.i = 0}),
    WS_KEYS(                XK_2,                           {.i = 1}),
    WS_KEYS(                XK_3,                           {.i = 2}),
    WS_KEYS(                XK_4,                           {.i = 3}),
    WS_KEYS(                XK_5,                           {.i = 4}),
#undef WS_KEYS

};

static const Binding buttons[] = {
    {Mod,                   Button1,    mouse_move,         NO_ARG},
    {Mod,                   Button3,    mouse_resize,       NO_ARG},
};

// {{{ 'undef'-ing.
#undef NO_ARG
#undef CMD
#undef CLICKABLE
#undef UNDERLINED
#undef Mod
// }}}

// vim:fdm=marker
