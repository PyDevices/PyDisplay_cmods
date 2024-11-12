#!/usr/bin/env bash

REPOS_DIR=~/gh
PORT_DIR=$REPOS_DIR/micropython/ports/esp32
BOARD=ESP32_GENERIC_S3
# VARIANT=SPIRAM_OCT
VARIANT=
MODULE=../../../../busdrivers/src/micropython.cmake
MANIFEST=$REPOS_DIR/mpdisplay/manifest.py
IDF_DIR=$REPOS_DIR/esp-idf
BUILD_DIR=build
if [ -n "$BOARD" ]; then
    BUILD_DIR=$BUILD_DIR-$BOARD
fi
if [ -n "$VARIANT" ]; then
    BUILD_DIR=$BUILD_DIR-$VARIANT
fi

set -e

. $IDF_DIR/export.sh
pushd $PORT_DIR
make -j BOARD=$BOARD BOARD_VARIANT=$VARIANT clean
make -j BOARD=$BOARD BOARD_VARIANT=$VARIANT submodules
make -j BOARD=$BOARD BOARD_VARIANT=$VARIANT all USER_C_MODULES=$MODULE # FROZEN_MANIFEST=$MANIFEST
esptool --baud 460800 write_flash 0 $BUILD_DIR/firmware.bin
popd
