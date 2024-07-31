#include "chip8.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    chip8::Chip8Emu *emulator = new chip8::Chip8Emu();
    if (argc == 2) {
        emulator->run_program(argv[1]);
        return 0;
    } else {
        std::cerr << "Usage: dummy.out PROGRAM.ch8\n";
        return -1;
    }
}
