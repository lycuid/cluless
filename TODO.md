How it works:
-------------
  - onMapRequest: if client not in `selws`, create client -> restack -> apply layout -> map window.
  - onMapNotify: if client in `selws` and `ClActive` flag set, focus window.
  - onUnmapNotify: if client in `selws`, call `ClientRemove` hook.
  - onConfigureRequest: redirect as it is.
  - onPropertyNotify: if `WM_NAME` or `_NET_WM_NAME`, then log status to `stdout`.
  - onKeyPress: handle keybinding.
  - onButtonPress: grab pointer data -> handle button binding.
  - onMotionNotify: use pointer data to move/resize client (if needed).
  - onButtonRelease: reset pointer data and release client from move/resize state (if needed).
  - onDestroyNotify: if client is managed (part of any workspace), call `ClientRemove` hook.

todo:
-----
  - [ ] **find a good name for the project**.
  - [ ] proper `focus` change for multiple scenarios (on window destroyed, ws
        switched, move client to ws etc).
  - [X] Properly kill client on keypress.
  - [X] proper logging for statusbar.
  - [X] rules for managing windows (docks, dialogs, statusbar etc).
  - [X] Client init hooks (switch ws, floating etc).
  - [X] process starts but cannot write to pipe.
  - [ ] simple left click to focus.
  - [X] windows change size after mapping.
  - [ ] status logging as a seperate module.
  - [ ] Other layouts (Fullscreen etc).

less important todos:
---------------------
  - [ ] a parent (frame) window for the clients, to possibly support titlebar, close button, window move etc.
  - [ ] workspace related apis (e.g. `ws_getclient`) in constant time.

bugs:
-----
  - [X] The statusbar's file stream pointer need to be nullified if the statusbar is terminated.
  - [X] negative resize causes crash.
  - [X] scratchpad dangling pointer, if window gets destroyed while unmapped.
