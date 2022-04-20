# Dynamic tiling window manager (unnamed).

#### Requirements:
  - libx11
  - gnu make (optional)

#### Build and Install
```sh
make && sudo make install
```

#### Layout: Tall (master/stack).
| Single window tiled.              |  Multiple windows tiled.        |
|-----------------------------------|---------------------------------|
| ![single](screenshots/single.png) | ![multi](screenshots/multi.png) |

#### Fancy floating.
![fancy](screenshots/fancy.png)
- Statusbar: xdbar &lt;[https://github.com/lycuid/xdbar](https://github.com/lycuid/xdbar)&gt;
- Terminal: Simple Terminal &lt;[https://st.suckless.org/](https://st.suckless.org/)&gt;
- Browser: Brave &lt;[https://brave.com/](https://brave.com/)&gt;

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

less important todos:
---------------
  - [ ] a parent (frame) window for the clients, to possibly support titlebar, close button, window move etc.
  - [ ] workspace related apis (e.g. `ws_getclient`) in constant time.
  - [ ] Other layouts (Fullscreen etc).

bugs:
-----
  - [X] The statusbar's file stream pointer need to be nullified if the statusbar is terminated.
  - [X] negative resize causes crash.

License:
--------
[GPLv3](https://gnu.org/licenses/gpl.html)
