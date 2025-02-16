enable_testing()

# Set up Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Common Directories
set(TEST_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(COMPONENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../components")
set(SYSTEM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../systems")
set(UTILITY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../utility")

# Copy test data to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data DESTINATION ${CMAKE_BINARY_DIR}/bin/Release)

# Add test executable
file(GLOB_RECURSE TEST_FILES "${TEST_SOURCES_DIR}/*.cpp")

add_executable(unit_tests ${TEST_FILES})

# Add library directories
target_link_directories(unit_tests PRIVATE
    "${CMAKE_BINARY_DIR}/lib/Debug"
)

# Link with the main library and Google Test
target_link_libraries(unit_tests PRIVATE
    GameEngine
    GTest::gtest
    GTest::gtest_main
    sfml-graphics
    sfml-window
    sfml-system
    ImGui-SFML
    opengl32
    winmm
    gdi32
)

# Include directories for tests
target_include_directories(unit_tests PRIVATE
    ${INCLUDE_DIR}
    ${COMPONENT_DIR}
    ${SYSTEM_DIR}
    ${UTILITY_DIR}
)