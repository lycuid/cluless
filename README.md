# window manager (unnamed).

#### Single window tiled.
![single](screenshots/single.png)
#### Multiple windows tiled.
![multi](screenshots/multi.png)
#### Fancy floating.
![fancy](screenshots/fancy.png)

Currently supports:
-------------------
  - `WM_DELETE_WINDOW`: sends event, if user signals to kill client.
  - `WM_TRANSIENT_FOR`: window doesn't tile.
  - `WM_SIZE_HINTS`: respects `min_{width,height}`, while window resize.
  - `_NET_WM_NAME`: property change will log to statusbar (fallback to `WM_NAME`, if not present).
  - `_NET_WM_WINDOW_TYPE`: support for `_NET_WM_WINDOW_TYPE_DOCK` with `_NET_WM_STRUT` property (`_NET_WM_STRUT_PARTIAL` is in progress).
  - `_NET_ACTIVE_WINDOW`: sets property for root window on child focus change.
  - `_NET_CLIENT_LIST`: updates property on root window on child window created/destoyed.

todo:
-----
  - [ ] proper `focus` change for multiple scenarios (on window destroyed, ws
        switched, move client to ws etc).
  - [X] Properly kill client on keypress.
  - [X] proper logging for statusbar.
  - [ ] rules for managing windows (docks, dialogs, statusbar etc).
  - [ ] Client init hooks (switch ws, floating etc).
  - [X] process starts but cannot write to pipe.
  - [ ] simple click to focus.
  - [ ] windows change size after mapping.

less important todos:
---------------
  - [ ] `ws_getclient` in constant time.

bugs:
-----
  - [ ] The statusbar's file stream pointer need to be nullified if the statusbar
        is terminated.
  - [X] negative resize causes crash.
