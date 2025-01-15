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

# Include GoogleTest for test discovery
include(GoogleTest)

# Add test executable
set(TEST_FILES
    component_test.cpp
    entity_test.cpp
    entity_manager_test.cpp
    component_factory_test.cpp
)

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
    sfml-graphics-s-d
    sfml-window-s-d
    sfml-system-s-d
    imgui_sfml_imported
    opengl32
    winmm
    gdi32
)

# Add SFML_STATIC definition when linking statically
target_compile_definitions(unit_tests PRIVATE
    SFML_STATIC
    IMGUI_SFML_STATIC
    $<$<CONFIG:Debug>:_DEBUG>
    SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# Include directories for tests
target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/components
    ${json_SOURCE_DIR}/include
    ${sfml_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${IMGUI_SFML_SOURCE_DIR}
)

# Copy test data to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Register tests with CTest
gtest_discover_tests(unit_tests
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
    DISCOVERY_MODE PRE_TEST
    PROPERTIES
    LABELS "unit"
    TIMEOUT 10
    TEST_PREFIX "GameEngine::"
    TEST_LIST GAMEENGINE_TESTS
)

# Add the test list to CTest
set_property(DIRECTORY PROPERTY TEST_INCLUDE_FILES "${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake")