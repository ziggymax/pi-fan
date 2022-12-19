#!/usr/bin/sh
/usr/bin/cmake --no-warn-unused-cli -DCMAKE_TOOLCHAIN_FILE:STRING=../raspi4.cmake -DCMAKE_VERBOSE_MAKEFILE:STRING=ON -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -S/workspaces/pi-fan -B/workspaces/pi-fan/build -G Ninja
/usr/bin/cmake --build ./build --config Release --target pi-fan-controller --
