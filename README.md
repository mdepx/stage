The Stage window manager for Wayland.

This is a minimalistic tiling window manager Stage, which is identical (in terms of features) rewrite of my previous X11 window manager that I had been using for about 10 years.

I spend just a week to write it, but already using it on daily basis. So be careful, it has lots of bugs.

All the features supported you can find reading the code. If you don't want to read code, this WM is not for you.

This supports xwayland, but I'm about to drop it.

The main reason for this WM is to define memory regions (slots) where a new terminal window appears once opened.

The width limit of a terminal window is enforced: it is 80 characters per Linux/FreeBSD code style guides. Configure this in the stage.c for your font size.

The default terminal is foot(1). I have this configuration in my ~/.config/foot/foot.ini:
```
font=xos4 Terminus:size=13
```

Keyboard bindings:
- Mod + Enter: open a terminal
- Mod + m: maximize/unmaximize a terminal window vertically
- Mod + 0..9: change workspace

Mouse buttons:
- Mod + Left Button: move window
- Mod + Right Button: resize window

![alt text](https://raw.githubusercontent.com/mdepx/stage/main/screenshots/stage.png)
