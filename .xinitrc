slstatus &

feh --bg-center ~/src/void-rice/wallpapers/1.jpg

# Runs wpa_gui in system tray
wpa_gui -t &

# Hides the mouse cursor after 3 seconds of inactivity
unclutter -idle 3 &

# Saving a dwm log
exec dwm > ~/.dwm.log
