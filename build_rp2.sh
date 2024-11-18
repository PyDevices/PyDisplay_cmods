#!/usr/bin/env bash

GIT_DIR=$(pwd)/..
PORT_DIR=$GIT_DIR/micropython/ports/rp2
BOARD=RPI_PICO
VARIANT=
MODULES=$GIT_DIR/busdrivers/modules.cmake
MANIFEST=$GIT_DIR/PyDisplay/manifest.py

BUILD_DIR=$PORT_DIR/build
if [ -n "$BOARD" ]; then
    BUILD_DIR=$BUILD_DIR-$BOARD
fi
if [ -n "$VARIANT" ]; then
    BUILD_DIR=$BUILD_DIR-$VARIANT
fi

set -e

pushd $PORT_DIR
make -j BOARD=$BOARD BOARD_VARIANT=$VARIANT clean
make -j BOARD=$BOARD BOARD_VARIANT=$VARIANT submodules
make -j BOARD=$BOARD BOARD_VARIANT=$VARIANT all USER_C_MODULES=$MODULES FROZEN_MANIFEST=$MANIFEST
popd

echo
echo "The firmware is:  $BUILD_DIR/firmware.uf2"
echo
