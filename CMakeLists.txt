cmake_minimum_required(VERSION 3.16)
project(koala_platform)

set(CMAKE_CXX_STANDARD 20)

# Add subdirectories
add_subdirectory(network)
add_subdirectory(platform)

# Main executable
add_executable(koala main.cpp)

# Link the libraries
target_link_libraries(koala PRIVATE network platform)

# Add includes for all modules
target_include_directories(koala PRIVATE
        ${CMAKE_SOURCE_DIR}/network/include
)
