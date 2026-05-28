cmake_minimum_required(VERSION 3.21)

add_library(core STATIC)

# 1. Explicitly track both CPP and Header files.
# If a header has a Q_OBJECT macro, CMake needs to see it here for AUTOMOC to catch it!
set(CORE_SOURCES
    ipc_client.cpp
    ipc_server.cpp
    network_client.cpp
    remote_executor.cpp
)

set(CORE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/core/ipc_client.h
    ${CMAKE_SOURCE_DIR}/include/core/ipc_server.h
    ${CMAKE_SOURCE_DIR}/include/core/network_client.h
    ${CMAKE_SOURCE_DIR}/include/core/remote_executor.h
)

target_sources(core PRIVATE
    ${CORE_SOURCES}
    ${CORE_HEADERS}  # <-- CRITICAL: Tells AUTOMOC to process these headers
)

# 2. Handles Qt Remote Objects compilation
qt6_add_repc_merged(core
    ${CMAKE_CURRENT_SOURCE_DIR}/ServiceInterface.rep
)

target_link_libraries(core
    PUBLIC
    Qt6::RemoteObjects
    PRIVATE
    Qt6::Core
    Qt6::Network
)

target_include_directories(core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# 3. Enable Qt build tools
set_target_properties(core PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)