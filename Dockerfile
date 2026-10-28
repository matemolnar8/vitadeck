# Linux smoke test image used by local golden updates and GitHub Actions.
FROM ubuntu:24.04 AS smoke

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        bash \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        git \
        libcurl4-openssl-dev \
        libegl-dev \
        libgl-dev \
        libssl-dev \
        libxcursor-dev \
        libxext-dev \
        libxi-dev \
        libxinerama-dev \
        libxrandr-dev \
        libxxf86vm-dev \
        ninja-build \
        pkg-config \
        xvfb \
        zlib1g-dev \
    && curl -fsSL https://deb.nodesource.com/setup_24.x | bash - \
    && apt-get install -y --no-install-recommends nodejs \
    && corepack enable \
    && corepack prepare pnpm@12.15.0 --activate \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
RUN git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git /tmp/raylib \
    && cmake -S /tmp/raylib -B /tmp/raylib/build -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF \
    && cmake --build /tmp/raylib/build --parallel "$(nproc)" \
    && cmake --install /tmp/raylib/build \
    && ldconfig \
    && rm -rf /tmp/raylib

WORKDIR /build/git

CMD ["bash"]

# Upstream ships VITASDK; recent prebuilt toolchains need glibc >= 2.36 while the
# image base is still Ubuntu 22.04. Final stage is Ubuntu 24.04 so arm-vita-eabi-gcc runs.
FROM --platform=linux/amd64 gnuton/vitasdk-docker@sha256:93f81ba3721f5fd78ff8ec99793e1ef3668519d8e0bd52ea799e20753b6df07d AS vitasdk-base

FROM --platform=linux/amd64 ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV VITASDK=/usr/local/vitasdk
ENV PATH=${PATH}:${VITASDK}/bin

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        ccache \
        cmake \
        curl \
        file \
        git \
        libarchive-tools \
        make \
        netcat-openbsd \
        pkg-config \
        xz-utils \
    && rm -rf /var/lib/apt/lists/*

COPY --from=vitasdk-base /usr/local/vitasdk /usr/local/vitasdk

# vitaShaRK: vitaGL's GLSL path calls shark_set_shader_association_path, which the
# vdpm copy inside the base image predates. Pinned alongside vitaGL so both move together.
WORKDIR /build
RUN git clone https://github.com/Rinnegatamante/vitaShaRK.git \
    && git -C vitaShaRK checkout df24065e65098b2d1ac533760109ad4367573f28
WORKDIR /build/vitaShaRK
RUN make -j"$(nproc)" install

# vitaGL
WORKDIR /build
RUN git clone https://github.com/Rinnegatamante/vitaGL.git \
    && git -C vitaGL checkout 76f43e6fbe1513cee44118da7ff2c16728c4fed2
WORKDIR /build/vitaGL
RUN HAVE_GLSL_SUPPORT=1 make -j"$(nproc)" install

# SDL
WORKDIR /build
RUN git clone https://github.com/Northfear/SDL.git \
    && git -C SDL checkout f99bb638f7eb256fa812e9da15d4e295a170640c
WORKDIR /build/SDL
RUN cmake -S. -Bbuild -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_TOOLCHAIN_FILE=${VITASDK}/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DVIDEO_VITA_VGL=ON
RUN cmake --build build -- -j$(nproc)
RUN cmake --install build

# raylib
WORKDIR /build
RUN git clone https://github.com/Rinnegatamante/raylib-5.5-vita.git \
    && git -C raylib-5.5-vita checkout 5a5deb4dde2b794f47d85fef5f0ffd4d3dfd44f1
WORKDIR /build/raylib-5.5-vita/src
# TraceLog() on VITA uses sceClib*; vitasdk declares them in psp2/kernel/clib.h (GCC errors without it).
RUN awk '/^#include "utils.h"$$/ { print; print ""; print "#if defined(PLATFORM_VITA)"; print "#include <psp2/kernel/clib.h>"; print "#endif"; next } 1' utils.c > utils.c.tmp && mv utils.c.tmp utils.c
RUN make clean
RUN make -j"$(nproc)"
RUN make install

WORKDIR /build/git

CMD ["bash"]
