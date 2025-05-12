# find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED) 
# find_package(OpenGL REQUIRED)
# find_package(glad REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
# find_package(glm CONFIG REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(TEST_SOURCES
    main.cpp
    ../src/vengine/core/signal_system.cpp
    signal_system_tests.cpp
    ../src/vengine/core/actions.cpp
    ../src/vengine/core/action.cpp
    actions_tests.cpp
    ecs_tests.cpp
    ../src/vengine/ecs/entities.hpp
    ../src/vengine/ecs/entity.hpp
    ../src/vengine/core/uuid.cpp
    ecs_entities_tests.cpp
)

add_executable(${PROJECT_NAME}_tests
    ${TEST_SOURCES}
)

target_link_libraries(${PROJECT_NAME}_tests PRIVATE 
    spdlog::spdlog
    glfw
)

set_target_properties(${PROJECT_NAME}_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/tests/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/tests/Release"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/$<CONFIG>"
)

# target_link_libraries(${PROJECT_NAME}_tests PRIVATE OpenGL::GL glad::glad glm::glm)
# target_link_options(${PROJECT_NAME}_tests PRIVATE "/SUBSYSTEM:CONSOLE")

add_test(NAME AllTests COMMAND ${PROJECT_NAME}_tests)