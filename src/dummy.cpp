#include "chip8.hpp"

int main(void) {
    chip8::Chip8Emu *emulator = new chip8::Chip8Emu();
    emulator->run_program("/home/alex/.projects/ch8roms/roms/IBM Logo.ch8");
    return 0;
}
