/* 'Companions' might seem similar to 'sticky' windows, although there is a
 * fundamental difference.
 * 'companions' follow you around across (work)spaces, i.e when you visit to a
 * different (work)space, the companion will follow you to that (work)space (by
 * detaching from the previous workspace and attaching as the 'head' of the
 * current workspace).
 * Best usecase:
 *    - with small floating windows (webcam, vlc, calendar, music).
 * Drawbacks:
 *    - position of companion(s), upon switching workspaces, would always be on
 *      the top of the client stack, of the current workspace.
 */
#ifndef __MISC__COMPANION_H__
#define __MISC__COMPANION_H__

#include <cluless/bindings.h>

Client *companion_remove(Workspace *);
void companion_insert(Client *, Workspace *);
void companion_toggle(const Arg *);

#endif
