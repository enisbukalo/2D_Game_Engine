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

# Find threading library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Enable parallel compilation for MSVC
if(MSVC)
    add_compile_options(/MP)
endif()

# Cache directory for dependencies
set(DEPS_CACHE_DIR "${CMAKE_SOURCE_DIR}/deps_cache" CACHE PATH "Directory for cached dependencies")

include(FetchContent)

# Configure dependency caching
set(FETCHCONTENT_BASE_DIR ${DEPS_CACHE_DIR})

# Find SFML (pre-built for Windows cross-compilation)
set(SFML_STATIC_LIBRARIES FALSE)
find_package(SFML 2.6 REQUIRED COMPONENTS graphics window system)

# Find OpenGL (required for ImGui-SFML)
find_package(OpenGL REQUIRED)

# Fetch ImGui (v1.88 is compatible with ImGui-SFML v2.6)
FetchContent_Declare(IMGUI
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.88
    GIT_SHALLOW ON)

FetchContent_MakeAvailable(IMGUI)

# Fetch ImGui-SFML (v2.6 for SFML 2.6.x compatibility)
FetchContent_Declare(IMGUI_SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v2.6
    GIT_SHALLOW ON)

FetchContent_GetProperties(IMGUI_SFML)
if(NOT imgui_sfml_POPULATED)
    FetchContent_Populate(IMGUI_SFML)

    # Manually build ImGui-SFML without using its CMakeLists.txt find_package
    add_library(ImGui-SFML STATIC
        ${imgui_sfml_SOURCE_DIR}/imgui-SFML.cpp
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    )

    target_include_directories(ImGui-SFML PUBLIC
        $<BUILD_INTERFACE:${imgui_sfml_SOURCE_DIR}>
        $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/ImGui-SFML>
    )

    target_link_libraries(ImGui-SFML PUBLIC
        sfml-graphics
        sfml-window
        sfml-system
        OpenGL::GL
    )

    target_compile_features(ImGui-SFML PUBLIC cxx_std_17)
    target_compile_definitions(ImGui-SFML PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

    # Create an alias to match the original target name
    add_library(ImGui-SFML::ImGui-SFML ALIAS ImGui-SFML)
endif()

# Common Directories
set(ENGINE_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ENGINE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(ENGINE_COMPONENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/components")
set(ENGINE_SYSTEM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/systems")
set(ENGINE_UTILITY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/utility")
set(ENGINE_PHYSICS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/physics")

# Create Game Engine library
file(GLOB_RECURSE ENGINE_SOURCES "${ENGINE_SOURCES_DIR}/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS "${ENGINE_INCLUDE_DIR}/*.h")

set(ENGINE_HEADERS ${ENGINE_HEADERS})

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
    $<BUILD_INTERFACE:${ENGINE_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${ENGINE_COMPONENT_DIR}>
    $<BUILD_INTERFACE:${ENGINE_SYSTEM_DIR}>
    $<BUILD_INTERFACE:${ENGINE_UTILITY_DIR}>
    $<BUILD_INTERFACE:${ENGINE_PHYSICS_DIR}>
    $<INSTALL_INTERFACE:include/GameEngine>
)

# Link against dependencies
target_link_libraries(GameEngine
    PUBLIC
    sfml-graphics
    sfml-window
    sfml-system
    ImGui-SFML
)

# Link Windows-specific libraries only when building for Windows
if(WIN32 OR MINGW)
    target_link_libraries(GameEngine
        PRIVATE
        opengl32
        winmm
        gdi32
    )
endif()

# Installation configuration
if(GAMEENGINE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/GameEngine)

    # Install library and dependencies
    install(TARGETS GameEngine ImGui-SFML
        EXPORT GameEngineTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GameEngine
    )

    # Install headers
    install(DIRECTORY
        ${ENGINE_INCLUDE_DIR}
        ${ENGINE_COMPONENT_DIR}
        ${ENGINE_SYSTEM_DIR}
        ${ENGINE_UTILITY_DIR}
        ${ENGINE_PHYSICS_DIR}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GameEngine
        FILES_MATCHING PATTERN "*.h"
    )

    # Install ImGui and ImGui-SFML headers
    install(DIRECTORY
        ${imgui_SOURCE_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ImGui-SFML
        FILES_MATCHING PATTERN "*.h"
    )
    install(FILES
        ${imgui_sfml_SOURCE_DIR}/imgui-SFML.h
        ${imgui_sfml_SOURCE_DIR}/imgui-SFML_export.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ImGui-SFML
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
    # Configure GTest - build as static libraries to avoid DLL issues
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Temporarily disable shared library building for Google Test
    set(BUILD_SHARED_LIBS_BACKUP ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF)

    add_subdirectory(tests)

    # Restore BUILD_SHARED_LIBS setting
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_BACKUP})
endif()
