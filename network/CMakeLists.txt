project(network)

add_library(network)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winhttp)
    target_sources(${PROJECT_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/windows_client.cpp
    )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
