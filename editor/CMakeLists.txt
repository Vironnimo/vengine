cmake_minimum_required(VERSION 3.20)

find_package(imgui CONFIG REQUIRED)

project(editor)

set(SOURCES
    app.cpp
    editor_scene.cpp
    editor_module.cpp
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    vengine
    imgui::imgui
)