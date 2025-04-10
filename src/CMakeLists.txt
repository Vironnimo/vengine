find_package(spdlog CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

set(SOURCES
        app/app.cpp
        vengine/vengine.cpp
        vengine/core/resource_manager.cpp
        vengine/core/resources.cpp
        vengine/core/action.cpp
        vengine/core/actions.cpp
        vengine/core/timer.cpp
        vengine/core/event_system.cpp
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
        vengine/renderer/window.cpp
)

add_executable(${PROJECT_NAME} 
        ${SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL glfw glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE tl::expected)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)


# copy resources to the build directory
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RESOURCES_DIR} "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/resources"
)
add_dependencies(${PROJECT_NAME} copy_resources)