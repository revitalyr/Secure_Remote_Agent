# app/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# UI executable target
add_executable(main)

# UI source files
set(APP_SOURCES
    main.cpp
    ui_manager.cpp
    main.qml
)

target_sources(main PRIVATE
    ${APP_SOURCES}
)

# Link with core library
target_link_libraries(main
    PRIVATE
    core
)

# Qt dependencies
target_link_libraries(main
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Quick
    Qt6::RemoteObjects
)

# Include directories
target_include_directories(main
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
)

# Qt MOC and QML setup
set_target_properties(main PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Add QML resources to sources
target_sources(main PRIVATE resources.qrc)

# Copy QML file to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/main.qml ${CMAKE_CURRENT_BINARY_DIR}/main.qml COPYONLY)

# Set output name
set_target_properties(main PROPERTIES
    OUTPUT_NAME "main"
)
