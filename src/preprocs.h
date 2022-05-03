// clang-format off
#include <X11/Xutil.h>
#include <stdarg.h>

// Misc.
#define Mod             Mod4Mask
#define Underlined(s)   "<Box:Bottom=#089CAC:1>" s "</Box>"
#define Clickable(k, s) "<BtnL=xdotool key " k ">" s "</BtnL>"
#define Term            "st"
#define CMD(...)        ((const char *[]){__VA_ARGS__, NULL})

// Scratchpad names.
#define ScratchTerm     "scratchpad-term"
#define ScratchFM       "scratchpad-fm"
#define ScratchNM       "scratchpad-nmtui"

// KeyBindings.
#define WSKeys(key, ws)                                                        \
  {Mod,             key, select_ws,         {.i = ws}},                        \
  {Mod | ShiftMask, key, move_client_to_ws, {.i = ws}}
// scratchpad '.cmd' value should be {sch_id, cmd, arg1, arg2, ...}.
#define SCHKeys(key, c) {Mod | ControlMask, key, sch_toggle, { .cmd = c }}
#define SCHToggle(id, ...)                                                     \
  CMD((const char[]){id}, Term, "-g", "112x30+136+76", __VA_ARGS__)
