cmake_minimum_required(VERSION 3.28)

# Check if GameEngine package is in this folder.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/GameEngine")
    message(FATAL_ERROR "GameEngine package not found in this folder. Please run build.sh in the root directory to create the package.")
endif()

# Set C++17 as the global standard before the project declaration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(GameEngineExample
    VERSION 1.0
    DESCRIPTION "Example project using GameEngine"
    LANGUAGES CXX)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Enable parallel compilation for MSVC
if(MSVC)
    add_compile_options(/MP)
endif()

# Cache directory for dependencies
set(DEPS_CACHE_DIR "${CMAKE_SOURCE_DIR}/deps_cache" CACHE PATH "Directory for cached dependencies")

include(FetchContent)

# Configure dependency caching
set(FETCHCONTENT_BASE_DIR ${DEPS_CACHE_DIR})

# Configure SFML with C++17
option(SFML_BUILD_AUDIO "Build audio" OFF)
option(SFML_BUILD_NETWORK "Build network" OFF)
option(SFML_BUILD_EXAMPLES "Build examples" OFF)
option(SFML_BUILD_DOC "Build documentation" OFF)
option(SFML_BUILD_TEST_SUITE "Build test suite" OFF)
option(SFML_INSTALL_PKGCONFIG_FILES "Install pkg-config files" OFF)
set(BUILD_SHARED_LIBS ON CACHE BOOL "") # Build dependencies as shared libraries
set(SFML_CXX_STANDARD 17 CACHE STRING "C++ standard for SFML")

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON)

# Dear ImGui
FetchContent_Declare(IMGUI
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.3
    GIT_SHALLOW ON)

# ImGui-SFML
FetchContent_Declare(IMGUI_SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON)

# Make SFML available
FetchContent_MakeAvailable(SFML)

if(TARGET sfml-system)
    set_target_properties(sfml-system sfml-window sfml-graphics
        PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
endif()

# Make ImGui available and set its C++ standard
FetchContent_MakeAvailable(IMGUI)

if(TARGET imgui)
    set_target_properties(imgui PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
endif()

# Configure ImGui-SFML
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO OFF)
set(IMGUI_SFML_CXX_STANDARD 17)

FetchContent_MakeAvailable(IMGUI_SFML)

if(TARGET ImGui-SFML)
    set_target_properties(ImGui-SFML PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
endif()

# Create imported target for GameEngine
add_library(GameEngine SHARED IMPORTED)

# Set GameEngine properties
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(GameEngine PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/bin/GameEngine-d.dll"
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/lib/GameEngine-d.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/include")
else()
    set_target_properties(GameEngine PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/bin/GameEngine.dll"
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/lib/GameEngine.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/include")
endif()

# Create example executable
add_executable(example src/main.cpp)

# Link against GameEngine and ImGui-SFML
target_link_libraries(example
    PRIVATE
    GameEngine
    ImGui-SFML::ImGui-SFML)

# Set C++17 for the example executable
set_target_properties(example PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

# Copy GameEngine DLL to output directory
add_custom_command(TARGET example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/bin/GameEngine-d.dll>$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/bin/GameEngine.dll>"
    "$<TARGET_FILE_DIR:example>"
    COMMENT "Copying GameEngine DLL to output directory")