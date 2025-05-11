cmake_minimum_required(VERSION 3.20)
project(app)

add_executable(app
    app.cpp
    test_module.cpp
    test_scene.cpp
    test_scene2.cpp
)

target_link_libraries(app PRIVATE vengine)