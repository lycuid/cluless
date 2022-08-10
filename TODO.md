todo:
-----
  - [X] **find a good name for the project**.
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
  - [X] Other layouts (Fullscreen etc).

less important todos:
---------------------
  - [ ] a parent (frame) window for the clients, to possibly support titlebar,
        close button, window move etc.
  - [ ] workspace related apis (e.g. `ws_getclient`) in constant time.

bugs:
-----
  - [X] The statusbar's file stream pointer need to be nullified if the
        statusbar is terminated.
  - [X] negative resize causes crash.
  - [X] scratchpad dangling pointer, if window gets destroyed while unmapped.
  - [ ] ewmh's `_NET_CLIENT_LIST` doesn't play well with scratchpads.
