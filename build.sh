#!/usr/bin/env bash
. ~/gh/esp-idf/export.sh 
pushd ~/gh/micropython/ports/esp32
make BOARD=ESP32_GENERIC_S3 USER_C_MODULES=../../../../esp_lcd/src/micropython.cmake clean submodules all
popd