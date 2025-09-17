file(GLOB PLATFORM_SOURCES src/windows/*.cpp)

add_library(platform ${PLATFORM_SOURCES})

if (WIN32)
    target_link_libraries(platform PRIVATE winhttp)
endif()

target_include_directories(platform PUBLIC
        ${CMAKE_SOURCE_DIR}/network/include
)
