cmake_minimum_required(VERSION 3.21)
project(UnifiedSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Quick RemoteObjects)
find_package(OpenSSL)
find_package(Catch2 3)

# Tests are disabled by default
# Enable with -DBUILD_TESTS=ON if Catch2 is properly installed
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS AND NOT Catch2_FOUND)
    message(WARNING "BUILD_TESTS is ON but Catch2 not found, tests will not be built")
    set(BUILD_TESTS OFF)
endif()

if(BUILD_TESTS)
    message(STATUS "Tests will be built")
else()
    message(STATUS "Tests will be skipped")
endif()

add_subdirectory(src/core)
add_subdirectory(src/components/service)
add_subdirectory(src/components/agent)
add_subdirectory(src/components/app)
add_subdirectory(src/plugins/loader)
add_subdirectory(src/plugins/proxy)
add_subdirectory(src/plugins/scraper)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
