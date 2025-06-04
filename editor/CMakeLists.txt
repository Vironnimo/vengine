cmake_minimum_required(VERSION 3.20)

find_package(imgui CONFIG REQUIRED)

project(editor)

add_executable(${PROJECT_NAME}
    app.cpp
    editor_scene.cpp
    editor_module.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    vengine
    imgui::imgui
)