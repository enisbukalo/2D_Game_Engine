cmake_minimum_required(VERSION 3.28)

# Set version information
set(GAMEENGINE_VERSION_MAJOR 0)
set(GAMEENGINE_VERSION_MINOR 1)
set(GAMEENGINE_VERSION_PATCH 0)
set(GAMEENGINE_VERSION ${GAMEENGINE_VERSION_MAJOR}.${GAMEENGINE_VERSION_MINOR}.${GAMEENGINE_VERSION_PATCH})

project(GameEngine
    VERSION ${GAMEENGINE_VERSION}
    DESCRIPTION "A 2D Game Engine with Entity Component System"
    LANGUAGES CXX)

# Build options
option(GAMEENGINE_BUILD_SHARED "Build GameEngine as shared library" OFF)
option(GAMEENGINE_BUILD_TESTS "Build test programs" ON)
option(GAMEENGINE_INSTALL "Generate installation target" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable parallel compilation for MSVC
if(MSVC)
    add_compile_options(/MP)
endif()

# Cache directory for dependencies
set(DEPS_CACHE_DIR "${CMAKE_SOURCE_DIR}/deps_cache" CACHE PATH "Directory for cached dependencies")

include(FetchContent)

# Configure dependency caching
set(FETCHCONTENT_BASE_DIR ${DEPS_CACHE_DIR})

# Configure SFML
set(SFML_BUILD_AUDIO OFF CACHE BOOL "")
set(SFML_BUILD_NETWORK OFF CACHE BOOL "")
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "")
set(SFML_BUILD_DOC OFF CACHE BOOL "")
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "")
set(SFML_INSTALL_PKGCONFIG_FILES OFF CACHE BOOL "")
set(BUILD_SHARED_LIBS ON CACHE BOOL "") # Build dependencies as shared libraries
set(SFML_BUILD_MAIN OFF CACHE BOOL "")
set(SFML_BUILD_WINDOW ON CACHE BOOL "")
set(SFML_BUILD_GRAPHICS ON CACHE BOOL "")

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON)

FetchContent_MakeAvailable(SFML)

# Only build what we need from ImGui-SFML
FetchContent_Declare(IMGUI
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.3
    GIT_SHALLOW ON)

FetchContent_MakeAvailable(IMGUI)

# Configure ImGui-SFML
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_INSTALL ON)

FetchContent_Declare(IMGUI_SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON)

FetchContent_MakeAvailable(SFML IMGUI_SFML)

# Create Game Engine library
file(GLOB ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB INCLUDE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB COMPONENT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/components/*.h")
file(GLOB SYSTEM_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/systems/*.h")

set(ENGINE_HEADERS
    ${INCLUDE_HEADERS}
    ${COMPONENT_HEADERS}
    ${SYSTEM_HEADERS}
)

# Create library target
if(GAMEENGINE_BUILD_SHARED)
    add_library(GameEngine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    set_target_properties(GameEngine PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
    add_library(GameEngine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
endif()

add_library(GameEngine::GameEngine ALIAS GameEngine)

# Set library properties
set_target_properties(GameEngine PROPERTIES
    VERSION ${GAMEENGINE_VERSION}
    SOVERSION ${GAMEENGINE_VERSION_MAJOR}
    DEBUG_POSTFIX "-d"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON)

target_include_directories(GameEngine
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/components>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/systems>
    $<INSTALL_INTERFACE:include/GameEngine>
)

# Link against dependencies
target_link_libraries(GameEngine
    PUBLIC
    sfml-graphics
    sfml-window
    sfml-system
    ImGui-SFML
    PRIVATE
    opengl32
    winmm
    gdi32
)

# Installation configuration
if(GAMEENGINE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/GameEngine)

    # Install library
    install(TARGETS GameEngine
        EXPORT GameEngineTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GameEngine
    )

    # Install headers
    install(DIRECTORY
        include/
        include/components/
        include/systems/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GameEngine
        FILES_MATCHING PATTERN "*.h"
    )

    # Export targets
    install(EXPORT GameEngineTargets
        FILE GameEngineTargets.cmake
        NAMESPACE GameEngine::
        DESTINATION ${INSTALL_CONFIGDIR}
    )

    # Create version file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/GameEngineConfigVersion.cmake"
        VERSION ${GAMEENGINE_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    # Generate config file
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/GameEngineConfig.cmake"
        "@PACKAGE_INIT@\n\n"
        "include(CMakeFindDependencyMacro)\n\n"
        "# Find dependencies\n"
        "find_dependency(SFML COMPONENTS graphics)\n"
        "find_dependency(ImGui-SFML)\n"
        "include(\"\${CMAKE_CURRENT_LIST_DIR}/GameEngineTargets.cmake\")\n"
    )

    # Install config files
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/GameEngineConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/GameEngineConfigVersion.cmake"
        DESTINATION ${INSTALL_CONFIGDIR}
    )
endif()

# Add tests if enabled
if(GAMEENGINE_BUILD_TESTS)
    # Configure GTest
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    add_subdirectory(tests)
endif()
