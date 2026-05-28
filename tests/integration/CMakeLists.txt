# tests/integration/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Only build if Catch2 is found
if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found, skipping integration tests")
    return()
endif()

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Integration test executable
add_executable(integration_tests)

# Integration test sources
set(INTEGRATION_TEST_SOURCES
    main.cpp
    test_service_integration.cpp
    test_ipc_integration.cpp
    test_network_tls_integration.cpp
)

target_sources(integration_tests PRIVATE ${INTEGRATION_TEST_SOURCES})

# Link with Catch2
target_link_libraries(integration_tests PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)

# Link with Qt6
    target_link_libraries(integration_tests PRIVATE Qt6::Core Qt6::Network)

# Link with project libraries
target_link_libraries(integration_tests PRIVATE core plugin_loader)

# Include directories
target_include_directories(integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/plugins/loader
    ${CMAKE_SOURCE_DIR}/src/components/service
)

# Qt MOC
set_target_properties(integration_tests PROPERTIES AUTOMOC ON)

# Add integration tests to CTest
# Catch2 automatically discovers tests via Catch2::Catch2
include(CTest)
add_test(NAME integration_tests COMMAND integration_tests)
