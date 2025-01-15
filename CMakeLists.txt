cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

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

# Only build what we need from SFML
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
set(SFML_BUILD_EXAMPLES OFF)
set(SFML_BUILD_DOC OFF)
set(SFML_BUILD_TEST_SUITE OFF)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)

# Only build what we need from Box2D
set(BOX2D_BUILD_EXAMPLES OFF)
set(BOX2D_BUILD_TESTBED OFF)
set(BOX2D_BUILD_UNIT_TESTS OFF)

FetchContent_Declare(BOX2D
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)

# Only build what we need from ImGui-SFML
FetchContent_Declare(IMGUI
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.3
    GIT_SHALLOW ON)

FetchContent_GetProperties(IMGUI)

if(NOT imgui_POPULATED)
    FetchContent_Populate(IMGUI)
endif()

# Configure ImGui-SFML
set(IMGUI_DIR ${imgui_SOURCE_DIR})
FetchContent_Declare(IMGUI_SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON)

set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_MakeAvailable(SFML BOX2D IMGUI_SFML)

# Add JSON library
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW ON)

FetchContent_MakeAvailable(SFML BOX2D IMGUI_SFML json)

# Create Game Engine library
set(ENGINE_SOURCES
    src/Component.cpp
    src/GameEngine.cpp
)

set(ENGINE_HEADERS
    include/Entity.h
    include/EntityManager.h
    include/GameEngine.h
    include/Vec2.h
    components/Component.h
)

add_library(GameEngine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

target_include_directories(GameEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/components
    ${CMAKE_CURRENT_SOURCE_DIR}/systems
)

target_link_libraries(GameEngine PUBLIC
    sfml-graphics
    box2d
    ImGui-SFML
    nlohmann_json::nlohmann_json
)

# Create main executable
add_executable(main
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_link_libraries(main PRIVATE
    GameEngine
)

# Add tests subdirectory
add_subdirectory(tests)

if(WIN32)
    add_custom_command(
        TARGET main
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:main>
        VERBATIM)
endif()
