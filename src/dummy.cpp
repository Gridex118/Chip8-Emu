#include "chip8.hpp"

int main(int argc, char *argv[]) {
    chip8::Chip8Emu *emulator = new chip8::Chip8Emu();
    emulator->run_program(argv[1]);
    return 0;
}
