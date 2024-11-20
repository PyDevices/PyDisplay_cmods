#!/usr/bin/env bash

GIT_DIR=$(pwd)/..
PORT_DIR=$GIT_DIR/micropython/ports/unix
MODULES=$GIT_DIR/lvmp
MANIFEST=$GIT_DIR/pydisplay/manifest.py

set -e

pushd $PORT_DIR
make -j BOARD=$BOARD clean
make -j BOARD=$BOARD submodules
make -j BOARD=$BOARD all USER_C_MODULES=$MODULES # FROZEN_MANIFEST=$MANIFEST
popd

echo
echo "The firmware is:  $PORT_DIR/build-standard/micropython"
echo
