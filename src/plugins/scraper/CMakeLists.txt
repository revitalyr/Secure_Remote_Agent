# plugins/scraper/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Scraper plugin
add_library(scraper_plugin SHARED)

set(SCRAPER_PLUGIN_SOURCES
    scraper_plugin.cpp
    scraper_plugin.h
)

target_sources(scraper_plugin PRIVATE
    ${SCRAPER_PLUGIN_SOURCES}
)

target_link_libraries(scraper_plugin
    PRIVATE
    core
    Qt6::Core
    Qt6::Network
)

target_include_directories(scraper_plugin
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
)

set_target_properties(scraper_plugin PROPERTIES
    AUTOMOC ON
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
