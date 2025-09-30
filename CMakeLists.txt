cmake_minimum_required(VERSION 3.31)
project(koala_platform)

set(CMAKE_CXX_STANDARD 20)

# Add subdirectories
add_subdirectory(storage)
add_subdirectory(network)
add_subdirectory(core)
add_subdirectory(app)

# Make "app" the default startup target
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT app)  # For Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY DEFAULT_TARGET app)      # For Ninja/Make (CMake ≥3.26)
