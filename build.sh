#!/usr/bin/env bash

GIT_PATH=~/gh
PORT_PATH=$GIT_PATH/micropython/ports/esp32
BOARD=ESP32_GENERIC_S3
VARIANT="BOARD_VARIANT=SPIRAM_OCT"
CMOD=../../../../busdrivers/src/micropython.cmake
MANIFEST=$GIT_PATH/mpdisplay/manifest.py
IDF_PATH=$GIT_PATH/esp-idf

set -e

. $IDF_PATH/export.sh
pushd $PORT_PATH
make -j BOARD=$BOARD $VARIANT clean
make -j BOARD=$BOARD $VARIANT submodules
make -j BOARD=$BOARD $VARIANT all FROZEN_MANIFEST=$MANIFEST USER_C_MODULES=$CMOD
# esptool --baud 460800 write_flash 0 build-$BOARD/firmware.bin
popd
