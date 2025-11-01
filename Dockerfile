FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install essential build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    clang \
    clang-format \
    ninja-build \
    pkg-config \
    wget \
    unzip \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libgl1-mesa-dev \
    mingw-w64 \
    wine \
    wine64 \
    libssl-dev \
    dos2unix \
    && rm -rf /var/lib/apt/lists/*

# Install CMake 3.28+ from official source
RUN wget https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0-linux-x86_64.sh && \
    chmod +x cmake-3.28.0-linux-x86_64.sh && \
    ./cmake-3.28.0-linux-x86_64.sh --skip-license --prefix=/usr/local && \
    rm cmake-3.28.0-linux-x86_64.sh

# Set up working directory
WORKDIR /app

# Download and setup SFML for Windows (MinGW)
RUN mkdir -p /opt/sfml-windows && \
    cd /opt/sfml-windows && \
    wget https://www.sfml-dev.org/files/SFML-2.6.1-windows-gcc-13.1.0-mingw-64-bit.zip && \
    unzip SFML-2.6.1-windows-gcc-13.1.0-mingw-64-bit.zip && \
    mv SFML-2.6.1 sfml && \
    rm SFML-2.6.1-windows-gcc-13.1.0-mingw-64-bit.zip

# Set environment variables for Windows builds
ENV SFML_WINDOWS_ROOT=/opt/sfml-windows/sfml
ENV MINGW_PREFIX=x86_64-w64-mingw32

# Copy project files
COPY . .

# Create build directory
RUN mkdir -p build

# Create script for building and testing
RUN echo '#!/bin/bash\n\
    cd build\n\
    cmake .. -GNinja\n\
    ninja\n\
    ./bin/unit_tests\n\
    ' > /usr/local/bin/build-and-test && \
    chmod +x /usr/local/bin/build-and-test

# Create script for formatting
RUN echo '#!/bin/bash\n\
    find . -type f \( -name "*.cpp" -o -name "*.h" \) \
    -not -path "./build/*" \
    -not -path "./deps_cache/*" \
    -exec clang-format -i -style=file {} +\n\
    ' > /usr/local/bin/format-code && \
    chmod +x /usr/local/bin/format-code

# Create script for Windows cross-compilation
RUN echo '#!/bin/bash\n\
    mkdir -p build-windows\n\
    cd build-windows\n\
    cmake .. -GNinja \\\n\
        -DCMAKE_TOOLCHAIN_FILE=/app/cmake/toolchain-mingw64.cmake \\\n\
        -DCMAKE_BUILD_TYPE=Release\n\
    ninja\n\
    ' > /usr/local/bin/build-windows && \
    chmod +x /usr/local/bin/build-windows

# Default command
CMD ["/bin/bash"] 