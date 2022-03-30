#ifndef __DEBUG__
#define __DEBUG__
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <stdio.h>

#define ERROR(fmt, ...)                                                        \
  {                                                                            \
    fprintf(stderr, "[ERROR] " #fmt, __VA_ARGS__);                             \
    fflush(stderr);                                                            \
  }

#define LOG(...)                                                               \
  {                                                                            \
    fprintf(stdout, __VA_ARGS__);                                              \
    fflush(stdout);                                                            \
  }

#define EVENT(...)                                                             \
  {                                                                            \
    LOG("[EVENT] ");                                                           \
    LOG(__VA_ARGS__);                                                          \
  }
#define INFO(...)                                                              \
  {                                                                            \
    LOG("[INFO] ");                                                            \
    LOG(__VA_ARGS__);                                                          \
  }
#define ACTION(...)                                                            \
  {                                                                            \
    LOG("[ACTION] ");                                                          \
    LOG(__VA_ARGS__);                                                          \
  }

static const char *const EventRepr[LASTEvent] = {
    [KeyPress]         = "KeyPress",
    [KeyRelease]       = "KeyRelease",
    [ButtonPress]      = "ButtonPress",
    [ButtonRelease]    = "ButtonRelease",
    [MotionNotify]     = "MotionNotify",
    [EnterNotify]      = "EnterNotify",
    [LeaveNotify]      = "LeaveNotify",
    [FocusIn]          = "FocusIn",
    [FocusOut]         = "FocusOut",
    [KeymapNotify]     = "KeymapNotify",
    [Expose]           = "Expose",
    [GraphicsExpose]   = "GraphicsExpose",
    [NoExpose]         = "NoExpose",
    [VisibilityNotify] = "VisibilityNotify",
    [CreateNotify]     = "CreateNotify",
    [DestroyNotify]    = "DestroyNotify",
    [UnmapNotify]      = "UnmapNotify",
    [MapNotify]        = "MapNotify",
    [MapRequest]       = "MapRequest",
    [ReparentNotify]   = "ReparentNotify",
    [ConfigureNotify]  = "ConfigureNotify",
    [ConfigureRequest] = "ConfigureRequest",
    [GravityNotify]    = "GravityNotify",
    [ResizeRequest]    = "ResizeRequest",
    [CirculateNotify]  = "CirculateNotify",
    [CirculateRequest] = "CirculateRequest",
    [PropertyNotify]   = "PropertyNotify",
    [SelectionClear]   = "SelectionClear",
    [SelectionRequest] = "SelectionRequest",
    [SelectionNotify]  = "SelectionNotify",
    [ColormapNotify]   = "ColormapNotify",
    [ClientMessage]    = "ClientMessage",
    [MappingNotify]    = "MappingNotify",
    [GenericEvent]     = "GenericEvent",
};

static const char *const RequestCodes[] = {
    [X_CreateWindow]            = "X_CreateWindow",
    [X_ChangeWindowAttributes]  = "X_ChangeWindowAttributes",
    [X_GetWindowAttributes]     = "X_GetWindowAttributes",
    [X_DestroyWindow]           = "X_DestroyWindow",
    [X_DestroySubwindows]       = "X_DestroySubwindows",
    [X_ChangeSaveSet]           = "X_ChangeSaveSet",
    [X_ReparentWindow]          = "X_ReparentWindow",
    [X_MapWindow]               = "X_MapWindow",
    [X_MapSubwindows]           = "X_MapSubwindows",
    [X_UnmapWindow]             = "X_UnmapWindow",
    [X_UnmapSubwindows]         = "X_UnmapSubwindows",
    [X_ConfigureWindow]         = "X_ConfigureWindow",
    [X_CirculateWindow]         = "X_CirculateWindow",
    [X_GetGeometry]             = "X_GetGeometry",
    [X_QueryTree]               = "X_QueryTree",
    [X_InternAtom]              = "X_InternAtom",
    [X_GetAtomName]             = "X_GetAtomName",
    [X_ChangeProperty]          = "X_ChangeProperty",
    [X_DeleteProperty]          = "X_DeleteProperty",
    [X_GetProperty]             = "X_GetProperty",
    [X_ListProperties]          = "X_ListProperties",
    [X_SetSelectionOwner]       = "X_SetSelectionOwner",
    [X_GetSelectionOwner]       = "X_GetSelectionOwner",
    [X_ConvertSelection]        = "X_ConvertSelection",
    [X_SendEvent]               = "X_SendEvent",
    [X_GrabPointer]             = "X_GrabPointer",
    [X_UngrabPointer]           = "X_UngrabPointer",
    [X_GrabButton]              = "X_GrabButton",
    [X_UngrabButton]            = "X_UngrabButton",
    [X_ChangeActivePointerGrab] = "X_ChangeActivePointerGrab",
    [X_GrabKeyboard]            = "X_GrabKeyboard",
    [X_UngrabKeyboard]          = "X_UngrabKeyboard",
    [X_GrabKey]                 = "X_GrabKey",
    [X_UngrabKey]               = "X_UngrabKey",
    [X_AllowEvents]             = "X_AllowEvents",
    [X_GrabServer]              = "X_GrabServer",
    [X_UngrabServer]            = "X_UngrabServer",
    [X_QueryPointer]            = "X_QueryPointer",
    [X_GetMotionEvents]         = "X_GetMotionEvents",
    [X_TranslateCoords]         = "X_TranslateCoords",
    [X_WarpPointer]             = "X_WarpPointer",
    [X_SetInputFocus]           = "X_SetInputFocus",
    [X_GetInputFocus]           = "X_GetInputFocus",
    [X_QueryKeymap]             = "X_QueryKeymap",
    [X_OpenFont]                = "X_OpenFont",
    [X_CloseFont]               = "X_CloseFont",
    [X_QueryFont]               = "X_QueryFont",
    [X_QueryTextExtents]        = "X_QueryTextExtents",
    [X_ListFonts]               = "X_ListFonts",
    [X_ListFontsWithInfo]       = "X_ListFontsWithInfo",
    [X_SetFontPath]             = "X_SetFontPath",
    [X_GetFontPath]             = "X_GetFontPath",
    [X_CreatePixmap]            = "X_CreatePixmap",
    [X_FreePixmap]              = "X_FreePixmap",
    [X_CreateGC]                = "X_CreateGC",
    [X_ChangeGC]                = "X_ChangeGC",
    [X_CopyGC]                  = "X_CopyGC",
    [X_SetDashes]               = "X_SetDashes",
    [X_SetClipRectangles]       = "X_SetClipRectangles",
    [X_FreeGC]                  = "X_FreeGC",
    [X_ClearArea]               = "X_ClearArea",
    [X_CopyArea]                = "X_CopyArea",
    [X_CopyPlane]               = "X_CopyPlane",
    [X_PolyPoint]               = "X_PolyPoint",
    [X_PolyLine]                = "X_PolyLine",
    [X_PolySegment]             = "X_PolySegment",
    [X_PolyRectangle]           = "X_PolyRectangle",
    [X_PolyArc]                 = "X_PolyArc",
    [X_FillPoly]                = "X_FillPoly",
    [X_PolyFillRectangle]       = "X_PolyFillRectangle",
    [X_PolyFillArc]             = "X_PolyFillArc",
    [X_PutImage]                = "X_PutImage",
    [X_GetImage]                = "X_GetImage",
    [X_PolyText8]               = "X_PolyText8",
    [X_PolyText16]              = "X_PolyText16",
    [X_ImageText8]              = "X_ImageText8",
    [X_ImageText16]             = "X_ImageText16",
    [X_CreateColormap]          = "X_CreateColormap",
    [X_FreeColormap]            = "X_FreeColormap",
    [X_CopyColormapAndFree]     = "X_CopyColormapAndFree",
    [X_InstallColormap]         = "X_InstallColormap",
    [X_UninstallColormap]       = "X_UninstallColormap",
    [X_ListInstalledColormaps]  = "X_ListInstalledColormaps",
    [X_AllocColor]              = "X_AllocColor",
    [X_AllocNamedColor]         = "X_AllocNamedColor",
    [X_AllocColorCells]         = "X_AllocColorCells",
    [X_AllocColorPlanes]        = "X_AllocColorPlanes",
    [X_FreeColors]              = "X_FreeColors",
    [X_StoreColors]             = "X_StoreColors",
    [X_StoreNamedColor]         = "X_StoreNamedColor",
    [X_QueryColors]             = "X_QueryColors",
    [X_LookupColor]             = "X_LookupColor",
    [X_CreateCursor]            = "X_CreateCursor",
    [X_CreateGlyphCursor]       = "X_CreateGlyphCursor",
    [X_FreeCursor]              = "X_FreeCursor",
    [X_RecolorCursor]           = "X_RecolorCursor",
    [X_QueryBestSize]           = "X_QueryBestSize",
    [X_QueryExtension]          = "X_QueryExtension",
    [X_ListExtensions]          = "X_ListExtensions",
    [X_ChangeKeyboardMapping]   = "X_ChangeKeyboardMapping",
    [X_GetKeyboardMapping]      = "X_GetKeyboardMapping",
    [X_ChangeKeyboardControl]   = "X_ChangeKeyboardControl",
    [X_GetKeyboardControl]      = "X_GetKeyboardControl",
    [X_Bell]                    = "X_Bell",
    [X_ChangePointerControl]    = "X_ChangePointerControl",
    [X_GetPointerControl]       = "X_GetPointerControl",
    [X_SetScreenSaver]          = "X_SetScreenSaver",
    [X_GetScreenSaver]          = "X_GetScreenSaver",
    [X_ChangeHosts]             = "X_ChangeHosts",
    [X_ListHosts]               = "X_ListHosts",
    [X_SetAccessControl]        = "X_SetAccessControl",
    [X_SetCloseDownMode]        = "X_SetCloseDownMode",
    [X_KillClient]              = "X_KillClient",
    [X_RotateProperties]        = "X_RotateProperties",
    [X_ForceScreenSaver]        = "X_ForceScreenSaver",
    [X_SetPointerMapping]       = "X_SetPointerMapping",
    [X_GetPointerMapping]       = "X_GetPointerMapping",
    [X_SetModifierMapping]      = "X_SetModifierMapping",
    [X_GetModifierMapping]      = "X_GetModifierMapping",
    [X_NoOperation]             = "X_NoOperation"};

#endif
