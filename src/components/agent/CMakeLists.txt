# agent/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Agent executable target
add_executable(agent)

# Agent source files
set(AGENT_SOURCES
    agent.cpp
)

target_sources(agent PRIVATE
    ${AGENT_SOURCES}
)

# Link with core library
target_link_libraries(agent
    PRIVATE
    core
)

# Qt dependencies
target_link_libraries(agent
    PRIVATE
    Qt6::Core
)

# Include directories
target_include_directories(agent
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/plugins/loader
)

# Set output name
set_target_properties(agent PROPERTIES
    OUTPUT_NAME "agent"
    AUTOMOC ON
)
