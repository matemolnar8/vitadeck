# Upstream ships VITASDK; recent prebuilt toolchains need glibc >= 2.36 while the
# image base is still Ubuntu 22.04. Final stage is Ubuntu 24.04 so arm-vita-eabi-gcc runs.
FROM --platform=linux/amd64 gnuton/vitasdk-docker AS vitasdk-base

FROM --platform=linux/amd64 ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV VITASDK=/usr/local/vitasdk
ENV PATH=${PATH}:${VITASDK}/bin

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
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

# vitaGL
WORKDIR /build
RUN git clone --depth=1 https://github.com/Rinnegatamante/vitaGL.git
WORKDIR /build/vitaGL
RUN HAVE_GLSL_SUPPORT=1 make install

# SDL
WORKDIR /build
RUN git clone --depth=1 https://github.com/Northfear/SDL.git
WORKDIR /build/SDL
RUN git checkout vitagl
RUN cmake -S. -Bbuild -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_TOOLCHAIN_FILE=${VITASDK}/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DVIDEO_VITA_VGL=ON
RUN cmake --build build -- -j$(nproc)
RUN cmake --install build

# raylib
WORKDIR /build
RUN git clone --depth=1 https://github.com/Rinnegatamante/raylib-5.5-vita.git
WORKDIR /build/raylib-5.5-vita/src
# TraceLog() on VITA uses sceClib*; vitasdk declares them in psp2/kernel/clib.h (GCC errors without it).
RUN awk '/^#include "utils.h"$$/ { print; print ""; print "#if defined(PLATFORM_VITA)"; print "#include <psp2/kernel/clib.h>"; print "#endif"; next } 1' utils.c > utils.c.tmp && mv utils.c.tmp utils.c
RUN make clean
RUN make -j"$(nproc)"
RUN make install

WORKDIR /build/git

CMD ["bash"]
