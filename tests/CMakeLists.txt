# tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Find Google Test
find_package(GTest REQUIRED)

# Add test subdirectories
add_subdirectory(unit)
add_subdirectory(integration)

# Enable testing
enable_testing()

# Add test targets
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "unit_"
    DEPENDS unit_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run_integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "integration_"
    DEPENDS integration_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS unit_tests integration_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
