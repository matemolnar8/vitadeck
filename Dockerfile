FROM gnuton/vitasdk-docker

RUN apt-get update 
RUN apt-get install -y file libarchive-tools

WORKDIR /build/

# vitaGL
RUN git clone https://github.com/Rinnegatamante/vitaGL.git
WORKDIR /build/vitaGL
RUN HAVE_GLSL_SUPPORT=1 make install

WORKDIR /build/

# SDL
RUN git clone https://github.com/Northfear/SDL.git

WORKDIR /build/SDL

RUN git checkout vitagl
RUN cmake -S. -Bbuild -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_TOOLCHAIN_FILE=${VITASDK}/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DVIDEO_VITA_VGL=ON 
RUN cmake --build build -- -j$(nproc)
RUN cmake --install build

WORKDIR /build/git

CMD ["bash"]