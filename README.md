# cluless

**cluless** is a fantasy motivated, _(mildly opinionated)_ and customizable dynamic tiling [window manager](https://wiki.archlinux.org/title/window_manager). The development was motivated by the author's fantasy of having the feeling of being inside the [grid](https://tron.fandom.com/wiki/Grid), while using his computer. The design and structure was motivated by the author's previously, daily driven, window managers: [xmonad](https://xmonad.org) and [dwm](https://dwm.suckless.org).

![fancy](https://raw.githubusercontent.com/lycuid/assets/master/cluless/floating.png)
- Statusbar: clubar &lt;[https://github.com/lycuid/clubar](https://github.com/lycuid/clubar)&gt;
- Terminal: Simple Terminal &lt;[https://st.suckless.org/](https://st.suckless.org/)&gt;

Layouts
-------
|Tall (master/stack) | Full (fullscreen) |
|--------------------|-------------------|
|![tall](https://raw.githubusercontent.com/lycuid/assets/master/cluless/tall.png) | ![full](https://raw.githubusercontent.com/lycuid/assets/master/cluless/full.png) |

Requirements
------------
  - ***free***, ***libre*** and ***open source*** Operating System (currently doesn't support \*BSD).
  - gnu make
  - libx11
  - pkg-config (optional, changes would be required in Makefile)

Build and Install
-----------------
```sh
make && sudo make install
```

Usage
-----
put the following line in the `~/.xinitrc` file, and use any preferred method (e.g [startx](https://man.archlinux.org/man/startx.1), [xinit](https://man.archlinux.org/man/xinit.1)) to execute it.
```sh
exec cluless
```
Status logs for statusbar ([clubar](https://github.com/lycuid/clubar), in this case) are dumped to `stdout`, which can be redirected in any way suitable.
```sh
exec cluless | clubar
```
using fifo.
```sh
STATUS=/tmp/statusbar${DISPLAY}
[ ! -p ${STATUS} ] && mkfifo ${STATUS}

clubar <${STATUS} &
exec cluless >${STATUS}
```
using a fifo might be slower compared to other methods, but also can be very convenient as the statusbar program will run independent of the window manager (i.e statusbar can be killed, restarted etc. without killing the window manager process).

Features
--------
  - ***scratchpads***: togglable windows.
  - ***window rule***: hooks for newly created windows (make window floating, or move to different workspace etc).
  - ***companions***: window follows the user across workspaces. (best used with floating windows, as it might mess up the tiling).
  - ***magnify***: magnify focused window (useful in case of many tiled windows).

License:
--------
[GPLv3](https://gnu.org/licenses/gpl.html)
