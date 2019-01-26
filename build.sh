#!/bin/bash

pushd software/dwm
echo "Compiling dwm"
make clean && make
popd && pushd software/dmenu
echo "Compiling dmenu"
make clean && make
popd && pushd software/slstatus
echo "Compiling slstatus"
make clean && make
popd && pushd software/st
echo "Compiling st"
make clean && make
popd && pushd software/vpm
