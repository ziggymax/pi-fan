# Compiling pi-fan
\- or how to get hold of a runnable executable.

For a quick solution, see if you can use a pre-built binary. Look in the ``binaries`` folder (check the readme file).

_(This is work in progress, more info is the the way ...)_

If your know your way around target and/or cross compiling, you will probably know how to proceed: simply use the CMake build. One caveat is that you have to fetch the sources for the bcm2835 library (2 files), and place bcm2835.h in ``include/`` and bcm2835.c in ``src/`` before building.

For cross-compile setup, take a look the ``raspi4-32.cmake`` file in the root folder of the project. The script file ``build.sh`` in the same spot shows how I build pi-fan myself.