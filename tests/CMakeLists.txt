# find_package(GTest CONFIG REQUIRED)
# find_package(spdlog CONFIG REQUIRED) 
# find_package(OpenGL REQUIRED)
# find_package(glad REQUIRED)
# find_package(glfw3 CONFIG REQUIRED)
# find_package(glm CONFIG REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(TEST_SOURCES
    main.cpp
    test_tests.cpp
)

add_executable(${PROJECT_NAME}_tests
    ${TEST_SOURCES}
)

# target_link_libraries(${PROJECT_NAME}_tests PRIVATE 
#     GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
# )
# target_link_libraries(${PROJECT_NAME}_tests PRIVATE spdlog::spdlog)
# target_link_libraries(${PROJECT_NAME}_tests PRIVATE OpenGL::GL glad::glad glfw glm::glm)
# target_link_options(${PROJECT_NAME}_tests PRIVATE "/SUBSYSTEM:CONSOLE")

add_test(NAME AllTests COMMAND ${PROJECT_NAME}_tests)