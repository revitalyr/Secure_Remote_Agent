cmake_minimum_required(VERSION 3.21)
project(UnifiedSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(Qt6 REQUIRED COMPONENTS Core Network Quick)

add_subdirectory(core)
add_subdirectory(service)
add_subdirectory(app)
add_subdirectory(proxy)
add_subdirectory(scraper)

