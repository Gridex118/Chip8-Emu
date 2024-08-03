#include "chip8.hpp"
#include "config.hpp"
#include <iostream>
#include <sstream>

int main(int argc, char *argv[]) {
    chip8::Chip8Emu *emulator = new chip8::Chip8Emu();
    if (argc < 2) {
        std::cerr << "Usage: dummy.out PROGRAM.ch8 [Display Scaling Factor] [CPU Frequency (Hz)]\n";
        return -1;
    }
    ch8cfg::Config config;
    if (argc >= 3) {
        std::istringstream ss(argv[2]);
        if ((ss >> config.disp_scale) && (config.disp_scale >= 1)) {
            // OK
        } else {
            std::cerr << "Invalid argument for scaling factor: " << argv[2] << '\n';
            return -1;
        }
    }
    if (argc >= 4) {
        std::istringstream ss(argv[3]);
        if ((ss >> config.cpu_freq) && (config.cpu_freq >= 0)) {
            // OK
        } else {
            std::cerr << "Invalid argument for cpu frequency: " << argv[3] << '\n';
            return -1;
        }
    }
    emulator->run_program(argv[1], config.disp_scale, config.cpu_freq);
    return 0;
}
