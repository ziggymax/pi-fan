FROM alpine:latest AS base

RUN apk update && apk add cmake git ninja-build python3 wget xz
RUN sh -c "cd /tmp; \
          wget --quiet 'https://github.com/tttapa/docker-arm-cross-toolchain/releases/latest/download/x-tools-aarch64-rpi3-linux-gnu.tar.xz'; \
          tar xf x-tools-aarch64-rpi3-linux-gnu.tar.xz; \
          chmod -R +w x-tools; \
          mkdir -p /opt/pi-cross-compile; \
          mv x-tools/aarch64-rpi3-linux-gnu /opt/pi-cross-compile; \
          rm -rf x-tools x-tools-aarch64-rpi3-linux-gnu.tar.xz;"
