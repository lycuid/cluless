# cluless window manager

**cluwm** is a fantasy motivated, _(mildly opinionated)_ and customizable dynamic tiling [window manager](https://wiki.archlinux.org/title/window_manager). The design and structure was motivated by the author's previously, daily driven, window managers: [xmonad](https://xmonad.org) and [dwm](https://dwm.suckless.org).

![fancy](https://lycuid.github.io/assets/cluwm/floating.png)
- Statusbar: clubar &lt;[https://github.com/lycuid/clubar](https://github.com/lycuid/clubar)&gt;
- Terminal: cluterm &lt;[https://github.com/lycuid/cluterm](https://github.com/lycuid/cluterm)&gt;

Layouts
-------
|Tall (master/stack) | Full (fullscreen) |
|--------------------|-------------------|
|![tall](https://lycuid.github.io/assets/cluwm/tall.png) | ![full](https://lycuid.github.io/assets/cluwm/full.png) |

Requirements
------------
  - libx11

Build and Install
-----------------
```sh
make && sudo make install
```

Usage
-----
put the following line in the `~/.xinitrc` file, and use any preferred method (e.g [startx](https://man.archlinux.org/man/startx.1), [xinit](https://man.archlinux.org/man/xinit.1)) to execute it.
```sh
exec cluwm
```
Status logs for statusbar ([clubar](https://github.com/lycuid/clubar), in this case) are dumped to `stdout`, which can be redirected in any way suitable. (see `LogFormat` in [src/config.h](src/config.h#L45)
```sh
exec cluwm | clubar
```
using fifo.
```sh
STATUS=/tmp/status${DISPLAY}
[ ! -p ${STATUS} ] && mkfifo ${STATUS}

clubar <${STATUS} &
exec cluwm >${STATUS}
```
Using fifo, the statusbar program can run independent from the window manager (i.e statusbar can be killed, restarted etc. without killing the window manager process).

Features
--------
  - ***scratchpads***: togglable windows.
  - ***window rule***: hooks for newly created windows (make window floating, or move to different workspace etc).
  - ***companions***: window follows the user across workspaces. (best used with floating windows, as it might mess up the tiling).
  - ***magnify***: magnify focused window (useful in case of many tiled windows).

License:
--------
[GPLv3](https://gnu.org/licenses/gpl.html)
