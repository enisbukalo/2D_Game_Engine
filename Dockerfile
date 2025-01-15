FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install essential build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    clang \
    clang-format \
    ninja-build \
    pkg-config \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /app

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

# Default command
CMD ["/bin/bash"] 