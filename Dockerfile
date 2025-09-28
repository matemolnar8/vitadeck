FROM gnuton/vitasdk-docker:latest

SHELL ["/bin/bash", "-lc"]

# Install build prerequisites inside the container
RUN apt-get update && apt-get install -y --no-install-recommends \
    git make gcc g++ pkg-config cmake wget ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Ensure VITASDK env is set (in base image it usually is)
ENV PATH="/usr/local/vitasdk/bin:${PATH}" \
    VITASDK="/usr/local/vitasdk"

# Build and install vitaGL (dependency for Raylib Vita backend) if not present
RUN if [ ! -d "$VITASDK/arm-vita-eabi/lib" ] || ! arm-vita-eabi-gcc -v >/dev/null 2>&1; then \
      echo "VitaSDK toolchain missing" && exit 1; \
    fi

# Build and install Raylib for Vita (vitaGL backend)
# We use a maintained fork that provides Vita port; adjust repository if needed.
RUN mkdir -p /tmp/build && cd /tmp/build \
    && git clone --depth 1 https://github.com/raylib4PlayStation/raylib4PlayStation.git \
    && cd raylib4PlayStation/src \
    && make clean && make -j"$(nproc)" PLATFORM=PLATFORM_VITA \
    && make install PLATFORM=PLATFORM_VITA \
    && cd / && rm -rf /tmp/build

WORKDIR /build/git

CMD ["bash"]

