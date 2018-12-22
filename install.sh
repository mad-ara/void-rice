#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "The script has to be run as root."
  exit
fi

echo "Checking directories"
mkdir -p /usr/local/bin/
mkdir -p /usr/share/themes/

pushd software/dwm-6.1
echo "Installing dwm"
make install
popd && pushd software/dmenu-4.8
echo "Installing dmenu"
make install 
popd && pushd software/slstatus
echo "Installing slstatus"
make install
popd && pushd software/st
echo "Installing st"
make install
popd && pushd software/vpm
echo "Installing vpm"
cp vpm /usr/local/bin/vpm
popd && pushd software/vsv
echo "Installing vsv"
cp vsv /usr/local/bin/vsv
popd
echo "Installing themes"
cp -r .themes/* /usr/share/themes/