# core/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Core library target
add_library(core STATIC)

# Core source files
set(CORE_SOURCES
    ipc_client.cpp
    ipc_server.cpp
    network_client.cpp
    remote_executor.cpp
)

target_sources(core PRIVATE
    ${CORE_SOURCES}
)

# Qt dependencies
target_link_libraries(core
    Qt6::Core
    Qt6::Network
)

# Include directories
target_include_directories(core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Qt MOC
set_target_properties(core PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)
