FROM gnuton/vitasdk-docker

RUN apt-get update \
    && apt-get install -y file libarchive-tools git 

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
RUN make clean
RUN make -j"$(nproc)"
RUN make install

WORKDIR /build/git

CMD ["bash"]