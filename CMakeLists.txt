cmake_minimum_required(VERSION 4.0)
project(koala)

set(CMAKE_CXX_STANDARD 20)

add_executable(koala main.cpp)

if (WIN32)
    target_link_libraries(koala PRIVATE winhttp)
endif()
