cmake_minimum_required(VERSION 3.00)
project(direct-edit)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# TODO : move sources to ./src
set(PROJECT_SOURCES main.c buffer.c)

add_executable(out ${PROJECT_SOURCES})

find_package(SDL2 REQUIRED)
target_include_directories(out PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(out ${SDL2_LIBRARIES})