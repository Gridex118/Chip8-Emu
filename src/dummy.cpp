#include "chip8.hpp"
#include <iostream>
#include <sstream>

int main(int argc, char *argv[]) {
    chip8::Chip8Emu *emulator = new chip8::Chip8Emu();
    if (argc == 2) {
        emulator->run_program(argv[1]);
        return 0;
    } else if (argc == 3) {
        std::istringstream ss(argv[2]);
        short scale;
        if ((ss >> scale) && (scale >= 1)) {
            emulator->run_program(argv[1], scale);
            return 0;
        } else {
            std::cerr << "Invalid argument for scaling factor: " << argv[2] << '\n';
            return -1;
        }
    } else {
        std::cerr << "Usage: dummy.out PROGRAM.ch8 [Display Scaling Factor]\n";
        return -1;
    }
}
