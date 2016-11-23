#!/usr/bin/env bash

# Build OMI
pushd omi/Unix
./configure --dev
make -j
popd

# Build the OMI Provider
pushd src
cmake -DCMAKE_BUILD_TYPE=Debug .
make -j
make reg
popd
