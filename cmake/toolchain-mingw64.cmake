# CMake Toolchain file for MinGW-w64 cross-compilation to Windows

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross compiler (Ubuntu MinGW-w64 GCC 13)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Where to look for the target environment
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# SFML Windows library path
if(DEFINED ENV{SFML_WINDOWS_ROOT})
    set(SFML_ROOT $ENV{SFML_WINDOWS_ROOT})
    list(APPEND CMAKE_FIND_ROOT_PATH ${SFML_ROOT})
    list(APPEND CMAKE_PREFIX_PATH ${SFML_ROOT})
endif()

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Make sure Qt can be detected by CMake
set(QT_BINARY_DIR /usr/x86_64-w64-mingw32/bin /usr/bin)
set(QT_INCLUDE_DIRS_NO_SYSTEM ON)

# Set the resource compiler
set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")
