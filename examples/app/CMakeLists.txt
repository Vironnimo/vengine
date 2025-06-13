cmake_minimum_required(VERSION 3.20)

project(app)

set(SOURCES
    app.cpp
    test_module.cpp
    test_scene.cpp
    test_scene2.cpp
)

add_executable(app
    ${SOURCES}
)

target_link_libraries(app PRIVATE vengine)