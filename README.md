# sleeper
Advanced activity timeout manager for X11, to dim your screen and activate your lockscreen.

Features:
- Multiple sleep targets
- Custom commands
- Detecting video players
- Detecting lockscreens
- Default out of the box configuration

## Example configuration
```
sleeper \
--stage 30 --command "brightnessctl s --save 10%" \
--stage 60 --command "brightnessctl s 1%" \
--stage 120 --command "your-lockscreen-command" \
--wakeup --command "brightnessctl --restore"
```
