cmake_minimum_required(VERSION 3.00)
project(direct-edit)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_SOURCES src/main.c src/buffer.c src/program.c)

add_executable(out ${PROJECT_SOURCES})

find_package(SDL2 REQUIRED)
target_include_directories(out PRIVATE ${SDL2_INCLUDE_DIRS} src)
target_link_libraries(out ${SDL2_LIBRARIES})
