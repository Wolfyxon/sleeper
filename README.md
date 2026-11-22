# sleeper
Advanced activity timeout manager for X11, to dim your screen, lock your session or put your system to sleep.

## Features
- Infinite custom sleep targets instead of just dim and lock
- Custom commands
- Detecting video players
- Detecting lockscreens
- Detecting fullscreen windows (off by default)
- Ability to switch all features on and off
- Default out of the box configuration

## Example configuration
```
sleeper \
--stage 30 --command "brightnessctl s --save 10%" \
--stage 60 --command "brightnessctl s 1%" \
--stage 120 --command "your-lockscreen-command" \
--wakeup --command "brightnessctl --restore"
```
