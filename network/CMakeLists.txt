project(network)

file(GLOB NETWORK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(network ${NETWORK_SOURCES})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winhttp)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
