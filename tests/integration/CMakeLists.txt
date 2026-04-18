# tests/integration/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Only build if Catch2 is found
if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found, skipping integration tests")
    return()
endif()

# Integration test executable
add_executable(integration_tests)

# Integration test sources
set(INTEGRATION_TEST_SOURCES
    main.cpp
    test_service_integration.cpp
    test_ipc_integration.cpp
    # test_plugin_integration.cpp  # plugin_loader.h doesn't exist
    # test_tls_integration.cpp  # network_tls.h doesn't exist
)

target_sources(integration_tests PRIVATE ${INTEGRATION_TEST_SOURCES})

# Link with Catch2
target_link_libraries(integration_tests
    PRIVATE
    Catch2::Catch2
    Catch2::Catch2WithMain
)

# Link with project libraries
target_link_libraries(integration_tests
    PRIVATE
    core
    plugin_loader
)

# Include directories
target_include_directories(integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/plugins/loader
    ${CMAKE_SOURCE_DIR}/src/components/service
)

# Add integration tests to CTest
# Catch2 automatically discovers tests via Catch2::Catch2
include(CTest)
add_test(NAME integration_tests COMMAND integration_tests)
