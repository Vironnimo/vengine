cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(vengine)

set(SOURCES
        src/game/game.cpp
        src/vengine/utils/utils.cpp
        src/vengine/core/uuid.cpp
        src/vengine/input/input.cpp
        src/vengine/timer/timer.cpp
        src/vengine/renderer/opengl_manager.cpp
        src/vengine/renderer/renderer.cpp
        src/vengine/renderer/shader_manager.cpp
        src/vengine/ecs/components.cpp
        src/vengine/ecs/ecs.cpp
        src/vengine/ecs/systems.cpp
        src/vengine/ecs/base_system.hpp
)
set(TEST_SOURCES
        tests/ecs_tests.cpp
)

set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/src/vengine/resources/shaders")

# FetchContent setup
include(FetchContent)
FetchContent_Declare(
        vconfig
        GIT_REPOSITORY https://github.com/ViroJB/vconfig.git
        GIT_TAG        main
)
FetchContent_MakeAvailable(vconfig)

# copy resources to the build directory
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${RESOURCES_DIR} ${CMAKE_BINARY_DIR}/Debug/resources/shaders
)

include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(vengine
        src/vengine/core/entry_point.cpp
        ${SOURCES}
)
add_dependencies(vengine copy_resources)

target_link_libraries(vengine PRIVATE fmt::fmt)
target_link_libraries(vengine PRIVATE Microsoft.GSL::GSL)
target_link_libraries(vengine PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm)
target_link_libraries(vengine PRIVATE vconfig)

# target_compile_options(vengine PRIVATE "/W4" "/wd4100" "/RTC1") # disable warning C4100 (unused parameter)
target_compile_options(vengine PRIVATE "/W4")

enable_testing()

# so we have to create a separate executable for each test file to make em show at least a bit separate in vscode testexplorer
add_executable(tests
        tests/main.cpp
        ${TEST_SOURCES}
        ${SOURCES}
)

# target_link_libraries(tests PRIVATE ${GTEST_ROOT}/lib/gtest.lib ${GTEST_ROOT}/lib/gtest_main.lib)
target_link_libraries(tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_link_libraries(tests PRIVATE fmt::fmt)
target_link_libraries(tests PRIVATE Microsoft.GSL::GSL)
target_link_libraries(tests PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm)
add_test(AllTests tests)