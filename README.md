# void-rice

![photo](https://raw.githubusercontent.com/0x0f0f0f/void-rice/master/screenshot.png)

My lightweight Desktop Environment for Void Linux based on suckless tools.
Includes a fully customized dwm fork with lot of patches applied such as  system tray support, sane keybindings and good mouse support out of the box.

## Downloading

`git clone https://github.com/0x0f0f0f/void-rice`
`cd void-rice`

## Installation

1) Fire up a text editor and edit `dependencies.txt` and config files in `software/` as you'd like. 
2) Install dependencies by running `./install-dependencies.sh`. The script will simply read required packages from `dependencies.txt` and run xbps-install.
3) Compile software by running the `./build.sh` script or manually by running `make` in each program's directory under `software/`
4) Install compiled software automatically by running `./install.sh` or with `sudo make install` in programs directories.
5) Set gtk and icon themes with `lxappearance`
6) Copy and tweak `.xinitrc` in your home folder.

## TLDR
```sh
cd void-rice # CD into this repository
sudo ./install-dependencies.sh # Install void packages
./build.sh  # Build
sudo ./install.sh # Install rice

mv ~/.xinitrc ~/.xinitrc.old # Backup old .xinitrc
cp .xinitrc ~/.xinitrc # Apply .xinitrc
```

Now set gtk a theme and icon pack. **slstatus** will likely fail to display correct values if not configuerd first.

If you edit the configuration files you'll need to rebuild and reinstall:
```sh
./build.sh && sudo ./install.sh
```

## Customized Software included in this repo
* [dwm](https://dwm.suckless.org/)
* [st](https://st.suckless.org/)
* [slstatus](https://tools.suckless.org/slstatus/)
* [vpm](https://github.com/bahamas10/vpm)
* [vsv](https://github.com/bahamas10/vsv)
* [dmenu](https://tools.suckless.org/dmenu/)
  

## Excluded packages you may like to install
* `autorandr` to automatically manage multiple monitors
* `icecat` GNU fork of firefox. Best browser out there.