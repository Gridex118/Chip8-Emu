#include "chip8.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <memory>

#define arrlen(arr) (sizeof arr / sizeof arr[0])

uint8_t FONT_DATA[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0
    0x20, 0x60, 0x20, 0x20, 0x70,   // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,   // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,   // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
    0xF0, 0x80, 0xF0, 0x80, 0x80    // F
};

inline std::ifstream::pos_type filesize(std::string file_name) {
    std::ifstream in(file_name, std::ios::ate | std::iostream::binary);
    return in.tellg();
}

namespace chip8 {

Chip8Emu::Chip8Emu() {
    display = std::make_shared<Chip8Display>();
    keypad = std::make_shared<Chip8Keypad>();
    memory = std::make_shared<Memory>();
    cpu = std::make_unique<Chip8Cpu>(memory, display, keypad);
    for (size_t i = 0; i < arrlen(FONT_DATA); i++) {
        memory->ram[i] = FONT_DATA[i];
    }
}

int Chip8Emu::load_program() {
    std::ifstream source(runnig_program, std::ios::in | std::ios::binary);
    if (source.is_open()) {
        source.read(reinterpret_cast<char*>(memory->ram.data() + 0x200), filesize(runnig_program));
    } else {
        std::cerr << "Could not open file\n";
        return -1;
    }
    return 0;
}

int Chip8Emu::run_program(std::string program, const short display_scaling_factor, const short cpu_freq) {
    const double FRAMEDELAY = 1000 / cpu_freq;
    runnig_program = program;
    display->init(runnig_program, display_scaling_factor);
    if (load_program() != 0) {
        std::cerr << "Error while loading program to memory\n";
        return -1;
    }
    const Uint8 *kbstate = SDL_GetKeyboardState(NULL);
    SDL_Event event;
    bool running = true;
    int frame_time;
    while (running) {
        int frame_start = SDL_GetTicks();
        if (cpu->exec_next() != 0) {
            std::cerr << "Error in execution stage\n";
            return -1;
        }
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        keypad->handle_input(&event, kbstate, &cpu->PC);
        cpu->decrement_timers();
        frame_time = SDL_GetTicks() - frame_start;
        if (FRAMEDELAY > frame_time) {
            SDL_Delay(FRAMEDELAY - frame_time);
        }
    }
    display->quit_sdl();
    return 0;
}

}
