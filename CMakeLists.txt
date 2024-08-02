cmake_minimum_required(VERSION 3.10)
project(Chip8 CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DUMMY_SRC
    src/chip8-disp.cpp
    src/chip8-emu.cpp
    src/chip8-keypad.cpp
    src/dummy.cpp
)

add_executable(dummy.out ${DUMMY_SRC})
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(dummy.out ${SDL2_LIBRARIES})

target_compile_options(dummy.out PRIVATE -Wall -Wextra -g -O0)
