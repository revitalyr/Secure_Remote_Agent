# service/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Service executable target
add_executable(service_main)

# Service source files
set(SERVICE_SOURCES
    service.cpp
    service_controller.cpp
)

target_sources(service_main PRIVATE
    ${SERVICE_SOURCES}
)

# Demo simulator executable (for testing without Windows Service)
add_executable(demo_simulator
    simulator.cpp
    service_controller.cpp
)

target_link_libraries(demo_simulator
    PRIVATE
    core
    Qt6::Core
    Qt6::Network
)

target_include_directories(demo_simulator
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/plugins/loader
)

set_target_properties(demo_simulator PROPERTIES
    AUTOMOC ON
)

# Link with core library
target_link_libraries(service_main
    PRIVATE
    core
)

# Qt dependencies
target_link_libraries(service_main
    PRIVATE
    Qt6::Core
    Qt6::Network
)

# Windows Service dependencies
if(WIN32)
    target_link_libraries(service_main
        PRIVATE
        advapi32
        user32
        shell32
    )
endif()

# Include directories
target_include_directories(service_main
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/plugins/loader
)

target_include_directories(demo_simulator
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/plugins/loader
)

# Set output name
set_target_properties(service_main PROPERTIES
    OUTPUT_NAME "service_main"
    AUTOMOC ON
)
