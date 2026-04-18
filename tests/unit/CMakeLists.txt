# tests/unit/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Only build if Catch2 is found
if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found, skipping unit tests")
    return()
endif()

# Unit test executable
add_executable(unit_tests)

# Unit test sources
set(UNIT_TEST_SOURCES
    main.cpp
    # test_network_tls.cpp  # network_tls.h doesn't exist
    test_plugin_interface.cpp
    test_ipc_client.cpp
    test_ui_manager.cpp
    test_service_main.cpp
)

target_sources(unit_tests PRIVATE ${UNIT_TEST_SOURCES})

set_target_properties(unit_tests PROPERTIES
    AUTOMOC ON
)

# Link with Catch2
target_link_libraries(unit_tests
    PRIVATE
    Catch2::Catch2
    Catch2::Catch2WithMain
)

# Link with project libraries
target_link_libraries(unit_tests
    PRIVATE
    core
    plugin_loader
)

# Include directories
target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/plugins/loader
    ${CMAKE_SOURCE_DIR}/src/components/service
)

# Add test to CTest
# Catch2 automatically discovers tests via Catch2::Catch2
include(CTest)
add_test(NAME unit_tests COMMAND unit_tests)
