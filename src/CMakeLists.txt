find_package(spdlog CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(Lua REQUIRED)
find_package(sol2 CONFIG REQUIRED)

set(SOURCES
        app/app.cpp
        app/test_module.cpp
        app/test_scene.cpp
        app/test_scene2.cpp
        vengine/vengine.cpp
        vengine/core/resource_manager.cpp
        vengine/core/action.cpp
        vengine/core/uuid.cpp
        vengine/core/actions.cpp
        vengine/core/timers.cpp
        vengine/core/event_system.cpp
        vengine/core/scenes.cpp
        vengine/ecs/entity.cpp
        vengine/renderer/renderer.cpp
        vengine/renderer/camera.cpp
        vengine/renderer/material.cpp
        vengine/renderer/materials.cpp
        vengine/renderer/mesh.cpp
        vengine/renderer/mesh_loader.cpp
        vengine/renderer/index_buffer.cpp
        vengine/renderer/vertex_array.cpp
        vengine/renderer/vertex_buffer.cpp
        vengine/renderer/shader.cpp
        vengine/renderer/shaders.cpp
        vengine/renderer/font.cpp
        vengine/renderer/fonts.cpp
        vengine/renderer/skybox.cpp
        vengine/renderer/window.cpp
)

add_executable(${PROJECT_NAME} 
        ${SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
        spdlog::spdlog
        OpenGL::GL
        glfw
        glm::glm
        Freetype::Freetype
        tl::expected
        glad::glad
        ${LUA_LIBRARIES}
        sol2
)

find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${MINIAUDIO_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_INCLUDE_DIR})

# copy resources to the build directory
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RESOURCES_DIR} "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/resources"
)
add_dependencies(${PROJECT_NAME} copy_resources)