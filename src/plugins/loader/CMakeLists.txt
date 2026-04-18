# plugins/loader/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Plugin loader library
add_library(plugin_loader STATIC)

# Plugin loader source files
set(PLUGIN_LOADER_SOURCES
    plugin_loader.cpp
)

target_sources(plugin_loader PRIVATE
    ${PLUGIN_LOADER_SOURCES}
)

# Link with core library
target_link_libraries(plugin_loader
    PRIVATE
    core
)

# Qt dependencies
target_link_libraries(plugin_loader
    PRIVATE
    Qt6::Core
    Qt6::Network
)

# Include directories
target_include_directories(plugin_loader
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
)

# Qt MOC
set_target_properties(plugin_loader PROPERTIES
    AUTOMOC ON
)
