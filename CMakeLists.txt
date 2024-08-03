cmake_minimum_required(VERSION 3.10)
project(Chip8 CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DUMMY_SRC
    src/chip8_disp.cpp
    src/chip8_emu.cpp
    src/chip8_keypad.cpp
    src/chip8_cpu.cpp
    src/config.cpp
    src/dummy.cpp
)

add_executable(dummy.out ${DUMMY_SRC})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(nlohmann_json REQUIRED)

target_link_libraries(dummy.out ${SDL2_LIBRARIES} nlohmann_json::nlohmann_json)

target_compile_options(dummy.out PRIVATE -Wall -Wextra -g -O0)
