# plugins/proxy/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Proxy plugin
add_library(proxy_plugin SHARED)

set(PROXY_PLUGIN_SOURCES
    proxy_plugin.cpp
    proxy_plugin.h
)

target_sources(proxy_plugin PRIVATE
    ${PROXY_PLUGIN_SOURCES}
)

target_link_libraries(proxy_plugin
    PRIVATE
    core
    Qt6::Core
    Qt6::Network
)

target_include_directories(proxy_plugin
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
)

set_target_properties(proxy_plugin PROPERTIES
    AUTOMOC ON
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
