# cluless

**cluless** is a fantasy motivated, _(mildly opinionated)_ and customizable dynamic tiling [window manager](https://wiki.archlinux.org/title/window_manager). The development was motivated by the author's fantasy of having the feeling of being inside the [grid](https://tron.fandom.com/wiki/Grid), while using his computer. The design and structure was motivated by the author's previously, daily driven, window managers: [xmonad](https://xmonad.org) and [dwm](https://dwm.suckless.org).

![fancy](https://raw.githubusercontent.com/lycuid/cluless/master/screenshots/floating.png)
- Statusbar: xdbar &lt;[https://github.com/lycuid/xdbar](https://github.com/lycuid/xdbar)&gt;
- Terminal: Simple Terminal &lt;[https://st.suckless.org/](https://st.suckless.org/)&gt;

Layouts
-------
#### Tall (master/stack)
![tall](https://raw.githubusercontent.com/lycuid/cluless/master/screenshots/tall.png)

#### Full (fullscreen)
![full](https://raw.githubusercontent.com/lycuid/cluless/master/screenshots/full.png)

Requirements
------------
  - libx11
  - gnu make (optional)
  - pkg-config (optional)

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
All logs for statusbar are dumped to `stdout`, which can be redirected in any way suitable.
```sh
exec cluless | statusbar
```
using fifo.
```sh
STATUS=/tmp/statusbar${DISPLAY}
[ ! -p ${STATUS} ] && mkfifo ${STATUS}

statusbar <${STATUS} &
exec cluless >${STATUS}
```

License:
--------
[GPLv3](https://gnu.org/licenses/gpl.html)
