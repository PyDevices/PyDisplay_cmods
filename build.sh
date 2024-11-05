#!/usr/bin/env bash
. ~/gh/esp-idf/export.sh 
pushd ~/gh/micropython/ports/esp32
make BOARD=ESP32_GENERIC_S3 USER_C_MODULES=../../../../busdrivers/src/micropython.cmake submodules all
popd