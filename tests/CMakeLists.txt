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

# Add test executable
set(TEST_FILES
    component_test.cpp
    entity_test.cpp
    entity_manager_test.cpp
)

add_executable(unit_tests ${TEST_FILES})

# Link with the main library and Google Test
target_link_libraries(unit_tests PRIVATE
    GameEngine
    GTest::gtest
    GTest::gtest_main
)

# Include directories for tests
target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/components
    ${json_SOURCE_DIR}/include
)

# Add compile definition for source directory
target_compile_definitions(unit_tests PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Copy test data to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Add the test
add_test(NAME unit_tests COMMAND unit_tests)