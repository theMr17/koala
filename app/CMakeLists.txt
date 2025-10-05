project(app)

file(GLOB_RECURSE APP_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${APP_SRC})

# In Windows Release builds, use the GUI subsystem to avoid spawning a console window.
# Keep Debug as a Console app so logs/printf are visible during development.
if (WIN32)
    # WIN32_EXECUTABLE toggles /SUBSYSTEM:WINDOWS on MSVC; other toolchains typically respect it as well.
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE $<IF:$<CONFIG:Release>,TRUE,FALSE>
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE storage network core)