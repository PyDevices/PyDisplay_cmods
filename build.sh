#!/usr/bin/env bash

PORT_PATH=~/gh/micropython/ports/esp32
BOARD=ESP32_GENERIC_S3
C_MOD=../../../../busdrivers/src/micropython.cmake
BUILD_FLAGS="CONFIG_FREERTOS_INTERRUPT_BACKTRACE=n"
MANIFEST=~/gh/busdrivers/manifest.py
IDF_PATH=~/gh/esp-idf

. $IDF_PATH/export.sh
pushd $PORT_PATH
make -j BOARD=$BOARD clean
make -j BOARD=$BOARD submodules
make -j BOARD=$BOARD USER_C_MODULES=$C_MOD $BUILD_FLAGS FROZEN_MANIFEST=$MANIFEST all
esptool --baud 460800 --erase_all write_flash 0 build-$BOARD/firmware.bin
popd