#!/bin/bash

rm -rf cmake-build
mkdir cmake-build
cd cmake-build
cmake -D CMAKE_TOOLCHAIN_FILE=../raspi4-32.cmake ..
make -j4
