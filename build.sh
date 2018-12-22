#!/bin/bash

pushd software/dwm-6.1
echo "Compiling dwm"
make clean && make
popd && pushd software/dmenu-4.8
echo "Compiling dmenu"
make clean && make
popd && pushd software/slstatus
echo "Compiling slstatus"
make clean && make
popd && pushd software/st
echo "Compiling st"
make clean && make
popd && pushd software/vpm