cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")


project(vengine)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/vendor)

add_subdirectory(src)
add_subdirectory(examples/app)

enable_testing()
add_subdirectory(tests)

# target_compile_options(vengine PRIVATE "/W4" "/wd4100" "/RTC1") # disable warning C4100 (unused parameter)
# target_compile_options(vengine PRIVATE "/W4")

# when not lowercasing the drive in the path, clangd will have problems.
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  add_custom_target(post_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/lowercase_paths.cmake
    COMMENT "Converting drive letters to lowercase in compile_commands.json"
    VERBATIM
  )
endif()