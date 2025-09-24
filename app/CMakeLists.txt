project(app)

file(GLOB_RECURSE APP_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${APP_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE network core)