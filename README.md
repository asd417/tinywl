# Personal Wayland Playground

A wayland compositor based on wlroots and tinywl, written in c. This is really just for me to hack on wayland. Code copied from the official wlroots

## Building wlroots-tinywl

install wlroots

update wlroots package version in the meson.build

meson build


## Debugging Wayland Compositors
You can run a wayland compositor by switching to a tty and launching it directly. But for debugging 
it is much nicer to run it embedded in your current window manager. We can set up a nice environment 
quite easily that will work regardless of window manager, display server, or video card. This is a way
get wlroots based compositors to play nice with NVIDIA graphics cards.
```
kwin_wayland --xwayland --socket wayland-1 --width 1920 --height 1080
WAYLAND_DISPLAY=wayland-1 ./result/bin/tinywl -s alacritty
```
Now tinywl will be running on `wayland-0`, inside the kwin window. Note that it doesn't matter if you
use KDE Plasma (or litterally anything else), it's just that kwin is a mature compositor with multiple
backends that will let us run tinywl (or any other compositor/window manager) embdedded. The kwin display
will be called `WL-1`, as opposed to something like `HDMI-A-1` if you need to access it (like from a sway
config).

# [TinyWL's readme](https://gitlab.freedesktop.org/wlroots/wlroots/-/tree/master/tinywl)
